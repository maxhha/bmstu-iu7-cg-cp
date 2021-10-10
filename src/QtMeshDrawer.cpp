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

QtMeshDrawer::QtMeshDrawer(QGraphicsView *view) : QWidget(view), view_(view), scene_(new QGraphicsScene(view_)), urd_normal_(-0.05, 0.05)
{
    light_direction.normalize();

    view_->setScene(scene_);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
};

void QtMeshDrawer::setMesh(const std::shared_ptr<CGCP::Mesh> mesh)
{
    MeshDrawer::setMesh(mesh);
    drawMesh();
};

void QtMeshDrawer::drawWireframeMesh()
{
    scene_->clear();
    scene_->setSceneRect(0, 0, view_->geometry().width(), view_->geometry().height());

    double ratio = (double)view_->geometry().width() / view_->geometry().height();

    projection_.setToIdentity();

    projection_.perspective(
        PERSPECTIVE_VERTICAL_ANGLE,
        ratio,
        PERSPECTIVE_NEAR_PLANE,
        PERSPECTIVE_FAR_PLANE);

    for (const auto &it : mesh_->triangles())
    {
        QPolygonF polygon;

        polygon << transform(it.p1()).toPointF();
        polygon << transform(it.p2()).toPointF();
        polygon << transform(it.p3()).toPointF();

        scene_->addPolygon(polygon);
    }

    view_->show();
}

void fill(QImage &image, double *z_buffer, QVector3D s1, QVector3D s2, QVector3D f1, QVector3D f2, QColor value)
{
    if (s1.y() == f1.y())
        return;

    int w = image.width(), h = image.height();

    double delta_y = f1.y() - s1.y();
    double dx1 = (f1.x() - s1.x()) / delta_y;
    double dz1 = (f1.z() - s1.z()) / delta_y;
    double dx2 = (f2.x() - s2.x()) / delta_y;
    double dz2 = (f2.z() - s2.z()) / delta_y;

    int s1yi = floor(s1.y()), f1yi = floor(f1.y());

    if (s1yi < 0)
    {
        s1.setX(s1.x() + dx1 * (-s1yi));
        s1.setZ(s1.z() + dz1 * (-s1yi));
        s2.setX(s2.x() + dx2 * (-s1yi));
        s2.setZ(s2.z() + dz2 * (-s1yi));
        s1yi = 0;
    }

    if (f1yi > h)
    {
        f1yi = h;
    }

    for (int y = s1yi; y < f1yi; y++)
    {
        double x1 = s1.x(), x2 = s2.x();
        double z1 = s1.z(), z2 = s2.z();

        if (x1 > x2)
        {
            std::swap(x1, x2);
            std::swap(z1, z2);
        }

        double delta_x = x2 - x1;
        double dz = (z2 - z1) / delta_x;

        int x1i = floor(x1), x2i = floor(x2);

        if (x1i < 0)
        {
            z1 += dz * (-x1i);
            x1i = 0;
        }

        if (x2i >= w)
        {
            x2i = w - 1;
        }

        for (int x = x1i; x <= x2i; x++)
        {
            if (z1 > z_buffer[y * w + x] && z1 > 0)
            {
                z_buffer[y * w + x] = z1;
                image.setPixelColor(x, y, value);
            }

            z1 += dz;
        }

        s1.setX(s1.x() + dx1);
        s1.setZ(s1.z() + dz1);

        s2.setX(s2.x() + dx2);
        s2.setZ(s2.z() + dz2);
    }
}

void QtMeshDrawer::drawMesh()
{
    color_rg_.seed(0);
    int w = view_->geometry().width();
    int h = view_->geometry().height();

    QImage color_buffer(w, h, QImage::Format::Format_ARGB32_Premultiplied);
    auto z_buffer = std::make_unique<double[]>(w * h);

    color_buffer.fill(Qt::white);
    for (int i = 0; i < w * h; i++)
        z_buffer[i] = -INFINITY;

    double ratio = (double)w / h;

    projection_.setToIdentity();

    projection_.perspective(
        PERSPECTIVE_VERTICAL_ANGLE,
        ratio,
        PERSPECTIVE_NEAR_PLANE,
        PERSPECTIVE_FAR_PLANE);

    for (const auto &it : mesh_->triangles())
    {
        auto c = color(it);
        auto p1 = transform(it.p1());
        auto p2 = transform(it.p2());
        auto p3 = transform(it.p3());

        // sort p1, p2, p3 by y
        if (p2.y() < p1.y())
            std::swap(p1, p2);

        if (p3.y() < p2.y())
        {
            std::swap(p2, p3);
            if (p2.y() < p1.y())
                std::swap(p2, p1);
        }

        if (p3.y() == p1.y())
        {
            // points of triangle is on a line
            // TODO: draw line or just skip it

            continue;
        }
        else
        {
            double k = (p2.y() - p1.y()) / (p3.y() - p1.y());
            auto p13 = p1 * (1 - k) + p3 * k;

            fill(color_buffer, &z_buffer[0], p1, p1, p13, p2, c);
            fill(color_buffer, &z_buffer[0], p13, p2, p3, p3, c);
        }
    }

    scene_->clear();
    scene_->setSceneRect(0, 0, w, h);

    scene_->addPixmap(QPixmap::fromImage(color_buffer));

    view_->show();
}

QVector3D QtMeshDrawer::transform(const CGCP::Vec3Df &p)
{
    double w = view_->geometry().width(), h = view_->geometry().height();

    QVector3D origin = Vec3Df2QVector3D(mesh_->origin());
    QVector3D window_scale(w, h, 1);
    QVector3D center(w / 2, h / 2, 0);

    auto v = transformMesh(p);

    v = projection_.map(v);
    v *= window_scale;
    v += center;

    return v;
}

QVector3D QtMeshDrawer::transformMesh(const CGCP::Vec3Df &p)
{
    QVector3D v = Vec3Df2QVector3D(p);
    QVector3D origin = Vec3Df2QVector3D(mesh_->origin());

    v -= origin;
    v *= scale_;
    v = rotate_.map(v);
    v += origin - translate_;

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

    if (QVector3D::dotProduct(normal, transformMesh(mesh_->origin()) - p1) > 0)
    {
        normal = -normal;
    }

    double c = QVector3D::dotProduct(normal, light_direction);

    if (c < 0)
    {
        c = 0;
    }

    return QColor(
        dark_color.red() * (1 - c) + light_color.red() * c,
        dark_color.green() * (1 - c) + light_color.green() * c,
        dark_color.blue() * (1 - c) + light_color.blue() * c);
}

void QtMeshDrawer::resizeEvent(QResizeEvent *event)
{
    drawMesh();
    QWidget::resizeEvent(event);
}

void QtMeshDrawer::rotate(const CGCP::Vec3Df &axis, double phi)
{
    rotate_.rotate(phi, axis.x(), axis.y(), axis.z());
    drawMesh();
}

void QtMeshDrawer::translate(const CGCP::Vec3Df &offset)
{
    translate_ = Vec3Df2QVector3D(offset);
    drawMesh();
}

void QtMeshDrawer::scale(const CGCP::Vec3Df &scale)
{
    scale_ = Vec3Df2QVector3D(scale);
    drawMesh();
}

QtMeshDrawer::~QtMeshDrawer()
{
}