#include "QtMeshDrawer.h"
#include <QDebug>
#include <QEvent>
#include <QPixmap>
#include <algorithm>
#include <cstdlib>
#include <math.h>

#define PERSPECTIVE_VERTICAL_ANGLE 90
#define PERSPECTIVE_NEAR_PLANE 0.1
#define PERSPECTIVE_FAR_PLANE 20

static inline QVector3D Vec3Df2QVector3D(const CGCP::Vec3Df &v)
{
    return QVector3D(v.x(), v.y(), v.z());
}

template <typename T>
static inline T clamp(T x, T min_x, T max_x)
{
    if (x < min_x)
        return min_x;
    if (x > max_x)
        return max_x;
    return x;
}

void QtMeshDrawer::config(const Config &config)
{
    if (config.count("shadow"))
    {
        shadow_enabled_ = std::stoi(config.at("shadow"));
    }

    if (config.count("border"))
    {
        border_enabled_ = std::stoi(config.at("border"));
    }

    if (config.count("light_x"))
    {
        light_direction_.setX(std::stod(config.at("light_x")));
    }

    if (config.count("light_y"))
    {
        light_direction_.setY(std::stod(config.at("light_y")));
    }

    if (config.count("light_z"))
    {
        light_direction_.setZ(std::stod(config.at("light_z")));
    }

    if (config.count("color1"))
    {
        light_color_[0] = QColor(QString::fromStdString(config.at("color1")));
    }

    if (config.count("color2"))
    {
        dark_color_[0] = QColor(QString::fromStdString(config.at("color2")));
    }

    if (config.count("color3"))
    {
        light_color_[1] = QColor(QString::fromStdString(config.at("color3")));
    }

    if (config.count("color4"))
    {
        dark_color_[1] = QColor(QString::fromStdString(config.at("color4")));
    }

    if (config.count("border_color"))
    {
        border_color_ = QColor(QString::fromStdString(config.at("border_color")));
    }

    light_direction_.normalize();

    draw();
}

QtMeshDrawer::QtMeshDrawer(QGraphicsView *view)
    : QWidget(view),
      view_(view),
      scene_(new QGraphicsScene(view_)),
      urd_normal_(-0.08, 0.08)
{
    light_direction_.normalize();

    light_buffer_w_ = 2048;
    light_buffer_h_ = 2048;
    light_buffer_ = std::make_unique<double[]>(light_buffer_w_ * light_buffer_h_);

    view_->setScene(scene_);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    updateBuffers();
};

void QtMeshDrawer::updateBuffers()
{
    int w = view_->contentsRect().width();
    int h = view_->contentsRect().height();

    if (z_buffer_h_ == h && z_buffer_w_ == w) {
        return;
    }

    z_buffer_h_ = h;
    z_buffer_w_ = w;

    color_buffer_ = QImage(w, h, QImage::Format::Format_ARGB32_Premultiplied);
    z_buffer_ = std::make_unique<double[]>(w * h);
}

void QtMeshDrawer::updateLightMatrix()
{
    if (!mesh_)
    {
        return;
    }

    light_projection_.setToIdentity();
    light_translate_ = QVector3D(0, 0, 0);
    light_scale_ = QVector3D(1, 1, 1);
    light_projection_.lookAt(
        QVector3D(0, 0, 0),
        -light_direction_,
        QVector3D(
            -light_direction_.y(),
            light_direction_.x(),
            light_direction_.z()));

    auto minimum = mesh_->domain().start();
    auto maximum = mesh_->domain().end();

    std::vector<CGCP::Vec3Df> points = {
        CGCP::Vec3Df(minimum.x(), minimum.y(), minimum.z()),
        CGCP::Vec3Df(maximum.x(), minimum.y(), minimum.z()),
        CGCP::Vec3Df(minimum.x(), maximum.y(), minimum.z()),
        CGCP::Vec3Df(maximum.x(), maximum.y(), minimum.z()),
        CGCP::Vec3Df(minimum.x(), minimum.y(), maximum.z()),
        CGCP::Vec3Df(maximum.x(), minimum.y(), maximum.z()),
        CGCP::Vec3Df(minimum.x(), maximum.y(), maximum.z()),
        CGCP::Vec3Df(maximum.x(), maximum.y(), maximum.z()),
    };

    auto min_v = transformLight(points[0]);
    auto max_v = min_v;

    for (int i = 1; i < 8; i++)
    {
        auto v = transformLight(points[i]);
        if (min_v.x() > v.x())
        {
            min_v.setX(v.x());
        }
        if (min_v.y() > v.y())
        {
            min_v.setY(v.y());
        }

        if (max_v.x() < v.x())
        {
            max_v.setX(v.x());
        }
        if (max_v.y() < v.y())
        {
            max_v.setY(v.y());
        }
    }

    light_translate_.setX(-min_v.x());
    light_translate_.setY(-min_v.y());

    light_scale_.setX(light_buffer_w_ / (max_v.x() - min_v.x()));
    light_scale_.setY(light_buffer_h_ / (max_v.y() - min_v.y()));
}

void QtMeshDrawer::setMesh(const std::shared_ptr<CGCP::Mesh> mesh)
{
    MeshDrawer::setMesh(mesh);
    resetTransformation();
    draw();
};

void QtMeshDrawer::drawMeshWireframe()
{
    if (!mesh_)
        return;

    double ratio = (double)view_->contentsRect().width() / view_->contentsRect().height();

    projection_.setToIdentity();

    projection_.perspective(
        PERSPECTIVE_VERTICAL_ANGLE,
        ratio,
        PERSPECTIVE_NEAR_PLANE,
        PERSPECTIVE_FAR_PLANE);

    QPen pen(QColor("#ff0000"), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    for (const auto &it : mesh_->triangles())
    {
        QPolygonF polygon;

        polygon << transform(it.p1()).toPointF();
        polygon << transform(it.p2()).toPointF();
        polygon << transform(it.p3()).toPointF();

        scene_->addPolygon(polygon, pen);
    }
}

void QtMeshDrawer::draw()
{
    scene_->clear();
    scene_->setSceneRect(0, 0, view_->contentsRect().width(), view_->contentsRect().height());

    drawMesh();
    // drawMeshWireframe();
    drawAxis();

    view_->show();
}

void QtMeshDrawer::drawMesh()
{
    if (!mesh_)
        return;

    updateBuffers();

    color_rg_.seed(0);
    int w = z_buffer_w_;
    int h = z_buffer_h_;

    color_buffer_.fill(Qt::white);
    for (int i = 0; i < w * h; i++)
        z_buffer_[i] = -INFINITY;

    if (shadow_enabled_)
    {
        for (int i = 0; i < light_buffer_w_ * light_buffer_h_; i++)
            light_buffer_[i] = -INFINITY;
    }

    double ratio = (double)w / h;

    projection_.setToIdentity();

    projection_.perspective(
        PERSPECTIVE_VERTICAL_ANGLE,
        ratio,
        PERSPECTIVE_NEAR_PLANE,
        PERSPECTIVE_FAR_PLANE);

    if (shadow_enabled_)
    {
        updateLightMatrix();

        for (const auto &it : mesh_->triangles())
        {
            drawTriangle(
                light_buffer_.get(),
                light_buffer_w_,
                light_buffer_h_,
                it,
                [&](const CGCP::Vec3Df &v) -> QVector3D
                {
                    return transformLight(v);
                },
                [&](const CGCP::Vec3Df &, int, int, bool) -> void {},
                INFINITY);
        }
    }

    for (const auto &it : mesh_->triangles())
    {
        auto color = this->color(it, false);
        auto shadow = this->color(it, true);

        drawTriangle(
            z_buffer_.get(),
            w,
            h,
            it,
            [&](const CGCP::Vec3Df &v) -> QVector3D
            {
                return transform(v);
            },
            [&](const CGCP::Vec3Df &p, int x, int y, bool b) -> void
            {
                color_buffer_.setPixelColor(
                    x,
                    y,
                    border_enabled_ && b
                        ? border_color_
                    : !shadow_enabled_ || isInLight(p)
                        ? color
                        : shadow);
            },
            0);
    }

    scene_->addPixmap(QPixmap::fromImage(color_buffer_));
}

void QtMeshDrawer::drawTriangle(
    double *z_buf,
    int w, int h,
    const CGCP::Triangle3Df &it,
    std::function<QVector3D(const CGCP::Vec3Df &)> transform,
    std::function<void(const CGCP::Vec3Df &p, int x, int y, bool b)> set_color,
    double limit_z)
{
    auto tp1 = it.p1();
    auto tp2 = it.p2();
    auto tp3 = it.p3();
    auto p1 = transform(tp1);
    auto p2 = transform(tp2);
    auto p3 = transform(tp3);

    if (p1.y() == p2.y() && p2.y() == p3.y())
        return; // ignore degenerate triangles

    // sort p1, p2, p3 by y
    if (p2.y() < p1.y())
    {
        std::swap(tp1, tp2);
        std::swap(p1, p2);
    }

    if (p3.y() < p2.y())
    {
        std::swap(tp2, tp3);
        std::swap(p2, p3);
        if (p2.y() < p1.y())
        {
            std::swap(tp2, tp1);
            std::swap(p2, p1);
        }
    }

    int y1 = clamp<int>(floor(p1.y()), 0, h - 1);
    int y2 = clamp<int>(floor(p3.y()), 0, h - 1);

    double total_height = p3.y() - p1.y();
    double h1 = p2.y() - p1.y();
    double h2 = p3.y() - p2.y();

    for (int y = y1; y <= y2; y++)
    {
        bool is_second_half = y > floor(p2.y()) || p1.y() == p2.y();
        double segment_height = is_second_half ? h2 : h1;

        double alpha = (y - floor(p1.y())) / total_height;
        double beta = (y - floor(is_second_half ? p2.y() : p1.y())) / segment_height;

        alpha = clamp<double>(alpha, 0, 1);
        beta = clamp<double>(beta, 0, 1);

        QVector3D A = p1 + (p3 - p1) * alpha;
        QVector3D B = is_second_half ? p2 + (p3 - p2) * beta : p1 + (p2 - p1) * beta;

        auto tA = tp1.mix(tp3, alpha);
        auto tB = is_second_half ? tp2.mix(tp3, beta) : tp1.mix(tp2, beta);

        if (A.x() > B.x())
        {
            std::swap(tA, tB);
            std::swap(A, B);
        }

        int x1 = floor(A.x()), x2 = floor(B.x());
        int sx1 = x1, sx2 = x2 - 1;
        double delta_z = (B.z() - A.z());
        double z = A.z(), dz = delta_z / (x2 - x1 + 1);

        if (x1 < 0)
        {
            z += dz * (-x1);
            x1 = 0;
        }

        if (x2 >= w)
        {
            x2 = w - 1;
        }

        for (int x = x1; x < x2; x++)
        {
            if (z > z_buf[y * w + x] && z < limit_z)
            {
                z_buf[y * w + x] = z;

                auto p = tA.mix(tB, (z - A.z()) / delta_z);
                set_color(p, x, y, sx1 == x || sx2 == x);
            }
            z += dz;
        }
    }
}

QVector3D QtMeshDrawer::transform(const CGCP::Vec3Df &p)
{
    double w = view_->contentsRect().width(), h = view_->contentsRect().height();

    auto v = QVector4D(transformMesh(p), 1);

    v = projection_.map(v);

    double v_w = v.w();
    v.setX((v.x() / v_w + 0.5) * w);
    v.setY((v.y() / v_w + 0.5) * h);
    v.setW(v.z());
    v.setZ(v_w);

    return QVector3D(v.x(), v.y(), v.z());
}

void QtMeshDrawer::drawAxis()
{
    if (!mesh_)
    {
        return;
    }
    QPen pen(QColor("#ff0000"), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QLineF line;
    auto p = transform(CGCP::Vec3Df(0));
    line.setP1(p.toPointF());

    CGCP::Vec3Df end(64, 64, 64);
    if (mesh_)
    {
        end = mesh_->domain().mix(CGCP::Vec3Df(0.5));
    }

    p = transform(CGCP::Vec3Df(end.x(), 0, 0));
    line.setP2(p.toPointF());
    scene_->addLine(line, pen);

    pen.setColor(QColor("#00ff00"));
    p = transform(CGCP::Vec3Df(0, end.y(), 0));
    line.setP2(p.toPointF());
    scene_->addLine(line, pen);

    pen.setColor(QColor("#0000ff"));
    p = transform(CGCP::Vec3Df(0, 0, end.z()));
    line.setP2(p.toPointF());
    scene_->addLine(line, pen);
}

QVector3D QtMeshDrawer::transformMesh(const CGCP::Vec3Df &p)
{
    QVector3D v = Vec3Df2QVector3D(p);
    QVector3D origin = Vec3Df2QVector3D(mesh_->origin());

    v -= origin;
    v *= scale_;
    v = rotate_.map(v);
    v += base_ + translate_;

    return v;
}

QVector3D QtMeshDrawer::transformLight(const CGCP::Vec3Df &p)
{
    auto v = transformMesh(p);
    v = light_projection_.map(v);
    v += light_translate_;
    v *= light_scale_;
    return v;
}

#define LIGHT_EPS 0.1

bool QtMeshDrawer::isInLight(const CGCP::Vec3Df &p)
{
    auto v = transformLight(p);

    int x = v.x();
    int y = v.y();

    if (x < 0 || y < 0 || x >= light_buffer_w_ || y >= light_buffer_h_)
    {
        return true;
    }

    int x1 = std::max({x - 1, 0});
    int y1 = std::max({y - 1, 0});
    int x2 = std::min({x + 1, light_buffer_w_ - 1});
    int y2 = std::min({y + 1, light_buffer_h_ - 1});

    double buf_z = INFINITY;

    for (int i = x1; i <= x2; i++)
    {
        for (int j = y1; j <= y2; j++)
        {
            double z = light_buffer_[i + j * light_buffer_w_];

            if (z != -INFINITY && buf_z > z)
            {
                buf_z = z;
            }
        }
    }

    return v.z() + LIGHT_EPS >= buf_z;
}

QColor QtMeshDrawer::color(const CGCP::Triangle3Df &t, bool shadow)
{
    auto p1 = transformMesh(t.p1());
    auto p2 = transformMesh(t.p2());
    auto p3 = transformMesh(t.p3());

    QVector3D r(urd_normal_(color_rg_), urd_normal_(color_rg_), urd_normal_(color_rg_));

    auto normal = QVector3D::crossProduct(p2 - p1, p3 - p1);
    normal.normalize();
    normal += r;
    normal.normalize();

    // if (QVector3D::dotProduct(normal, transformMesh(mesh_->origin()) - p1) > 0)
    // {
    //     normal = -normal;
    // }

    double c = QVector3D::dotProduct(normal, light_direction_);

    if (c < 0)
    {
        c = -c;
    }

    shadow = shadow > 0;

    return QColor(
        dark_color_[shadow].red() * (1 - c) + light_color_[shadow].red() * c,
        dark_color_[shadow].green() * (1 - c) + light_color_[shadow].green() * c,
        dark_color_[shadow].blue() * (1 - c) + light_color_[shadow].blue() * c);
}

void QtMeshDrawer::resetTransformation()
{
    scale_ = QVector3D(1, 1, 1);
    translate_ = QVector3D(0, 0, 0);
    base_ = QVector3D(0, 0, 0);
    // rotate_.setToIdentity();

    if (mesh_)
    {
        int w = view_->contentsRect().width();
        int h = view_->contentsRect().height();

        double ratio = (double)w / h;

        projection_.setToIdentity();

        projection_.perspective(
            PERSPECTIVE_VERTICAL_ANGLE,
            ratio,
            PERSPECTIVE_NEAR_PLANE,
            PERSPECTIVE_FAR_PLANE);

        auto minimum = mesh_->domain().start();
        auto maximum = mesh_->domain().end();

        std::vector<CGCP::Vec3Df> points = {
            CGCP::Vec3Df(minimum.x(), minimum.y(), minimum.z()),
            CGCP::Vec3Df(maximum.x(), minimum.y(), minimum.z()),
            CGCP::Vec3Df(minimum.x(), maximum.y(), minimum.z()),
            CGCP::Vec3Df(maximum.x(), maximum.y(), minimum.z()),
            CGCP::Vec3Df(minimum.x(), minimum.y(), maximum.z()),
            CGCP::Vec3Df(maximum.x(), minimum.y(), maximum.z()),
            CGCP::Vec3Df(minimum.x(), maximum.y(), maximum.z()),
            CGCP::Vec3Df(maximum.x(), maximum.y(), maximum.z()),
        };

#define B 10

        for (bool inside = false; !inside; base_.setZ(base_.z() + 1))
        {
            inside = true;

            for (auto &it : points)
            {
                auto p = transform(it);
                inside &= p.x() >= B && p.y() >= B && p.x() < w - B && p.y() < h - B;

                if (!inside)
                {
                    break;
                }
            }
        }
    }

    draw();
}

void QtMeshDrawer::resizeEvent(QResizeEvent *event)
{
    updateBuffers();
    draw();
    QWidget::resizeEvent(event);
}

void QtMeshDrawer::rotate(const CGCP::Vec3Df &axis, double phi)
{
    QMatrix4x4 m;
    m.setToIdentity();
    m.rotate(phi, axis.x(), axis.y(), axis.z());
    rotate_ = m * rotate_;
    draw();
}

void QtMeshDrawer::translate(const CGCP::Vec3Df &offset)
{
    translate_ = Vec3Df2QVector3D(offset);
    draw();
}

void QtMeshDrawer::scale(const CGCP::Vec3Df &scale)
{
    scale_ = Vec3Df2QVector3D(scale);
    draw();
}

QtMeshDrawer::~QtMeshDrawer()
{
}
