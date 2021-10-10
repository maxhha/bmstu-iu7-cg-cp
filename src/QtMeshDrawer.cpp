#include "QtMeshDrawer.h"
#include <QDebug>
#include <QEvent>

#define PERSPECTIVE_VERTICAL_ANGLE 90
#define PERSPECTIVE_NEAR_PLANE 0.1
#define PERSPECTIVE_FAR_PLANE 20

static inline QVector3D Vec3Df2QVector3D(const CGCP::Vec3Df &v)
{
    return QVector3D(v.x(), v.y(), v.z());
}

QtMeshDrawer::QtMeshDrawer(QGraphicsView *view) : QWidget(view), view_(view), scene_(new QGraphicsScene(view_))
{
    view_->setScene(scene_);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
};

void QtMeshDrawer::setMesh(const std::shared_ptr<CGCP::Mesh> mesh)
{
    MeshDrawer::setMesh(mesh);
    drawMesh();
};

void QtMeshDrawer::drawMesh()
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

        polygon << transform(it.p1());
        polygon << transform(it.p2());
        polygon << transform(it.p3());

        scene_->addPolygon(polygon);
    }

    view_->show();
}

QPointF QtMeshDrawer::transform(const CGCP::Vec3Df &p)
{
    double w = view_->geometry().width(), h = view_->geometry().height();

    QVector3D v = Vec3Df2QVector3D(p);
    QVector3D origin = Vec3Df2QVector3D(mesh_->origin());
    QVector3D window_scale(w, h, 1);
    QVector3D center(w / 2, h / 2, 0);

    v -= origin;
    v *= scale_;
    v = rotate_.map(v);
    v += origin + translate_;
    v = projection_.map(v);
    v *= window_scale;
    v += center;

    return v.toPointF();
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