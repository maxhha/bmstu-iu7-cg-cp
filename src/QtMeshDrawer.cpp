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

QtMeshDrawer::QtMeshDrawer(QGraphicsView *view)
    : QWidget(view),
      view_(view),
      scene_(new QGraphicsScene(view_)),
      urd_normal_(-0.08, 0.08)
{
    light_direction.normalize();

    view_->setScene(scene_);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    updateBuffers();
};

void QtMeshDrawer::updateBuffers()
{
    int w = view_->geometry().width();
    int h = view_->geometry().height();

    color_buffer_ = QImage(w, h, QImage::Format::Format_ARGB32_Premultiplied);
    z_buffer_ = std::make_unique<double[]>(w * h);
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

    double ratio = (double)view_->geometry().width() / view_->geometry().height();

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
    scene_->setSceneRect(0, 0, view_->geometry().width(), view_->geometry().height());

    drawMesh();
    // drawMeshWireframe();

    view_->show();
}

void QtMeshDrawer::drawMesh()
{
    if (!mesh_)
        return;

    color_rg_.seed(0);
    int w = view_->geometry().width();
    int h = view_->geometry().height();

    color_buffer_.fill(Qt::white);
    for (int i = 0; i < w * h; i++)
        z_buffer_[i] = -INFINITY;

    double ratio = (double)w / h;

    projection_.setToIdentity();

    projection_.perspective(
        PERSPECTIVE_VERTICAL_ANGLE,
        ratio,
        PERSPECTIVE_NEAR_PLANE,
        PERSPECTIVE_FAR_PLANE);

    for (const auto &it : mesh_->triangles())
    {
        drawTriangle(color_buffer_, z_buffer_.get(), it, this->color(it));
    }

    scene_->addPixmap(QPixmap::fromImage(color_buffer_));
}

void QtMeshDrawer::drawTriangle(QImage &c_buf, double *z_buf, const CGCP::Triangle3Df &it, const QColor &color)
{
    auto p1 = transform(it.p1());
    auto p2 = transform(it.p2());
    auto p3 = transform(it.p3());

    if (p1.y() == p2.y() && p2.y() == p3.y())
        return; // ignore degenerate triangles

    // sort p1, p2, p3 by y
    if (p2.y() < p1.y())
        std::swap(p1, p2);

    if (p3.y() < p2.y())
    {
        std::swap(p2, p3);
        if (p2.y() < p1.y())
            std::swap(p2, p1);
    }

    int h = c_buf.height();
    int w = c_buf.width();

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

        if (A.x() > B.x())
            std::swap(A, B);

        int x1 = floor(A.x()), x2 = floor(B.x());
        double z = A.z(), dz = (B.z() - A.z()) / (x2 - x1 + 1);

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
            if (z > z_buf[y * w + x] && z < 0)
            {
                z_buf[y * w + x] = z;
                c_buf.setPixelColor(x, y, color);
            }
            z += dz;
        }
    }
}

QVector3D QtMeshDrawer::transform(const CGCP::Vec3Df &p)
{
    double w = view_->geometry().width(), h = view_->geometry().height();

    QVector3D window_scale(w, h, 1);
    QVector3D center(w / 2, h / 2, 0);

    // auto v = transformMesh(p);

    // v = projection_.map(v);
    // v *= window_scale;
    // v += center;

    auto v = QVector4D(transformMesh(p), 1);

    v = projection_.map(v);

    v.setX(v.x() / v.w());
    v.setY(v.y() / v.w());
    v.setZ(v.w());

    v *= window_scale;
    v += center;

    return QVector3D(v.x(), v.y(), v.z());
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

QColor QtMeshDrawer::color(const CGCP::Triangle3Df &t)
{
    static QColor dark_color("#100018");
    static QColor light_color("#888088");

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

    double c = QVector3D::dotProduct(normal, light_direction);

    if (c < 0)
    {
        c = -c;
    }

    return QColor(
        dark_color.red() * (1 - c) + light_color.red() * c,
        dark_color.green() * (1 - c) + light_color.green() * c,
        dark_color.blue() * (1 - c) + light_color.blue() * c);
}

void QtMeshDrawer::resetTransformation()
{
    scale_ = QVector3D(1, 1, 1);
    translate_ = QVector3D(0, 0, 0);
    base_ = QVector3D(0, 0, 0);
    // rotate_.setToIdentity();

    if (mesh_)
    {
        QVector3D origin = Vec3Df2QVector3D(mesh_->origin());

        int w = view_->geometry().width();
        int h = view_->geometry().height();

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
