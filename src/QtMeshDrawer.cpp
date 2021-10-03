#include "QtMeshDrawer.h"
#include <QDebug>

static inline QVector3D Vec3Df2QVector3D(const CGCP::Vec3Df &v)
{
    return QVector3D(v.x(), v.y(), v.z());
}

QtMeshDrawer::QtMeshDrawer(QGraphicsView *view) : view_(view), scene_(new QGraphicsScene(view))
{
    view->setScene(scene_);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
};

void QtMeshDrawer::setMesh(const std::shared_ptr<CGCP::Mesh> mesh)
{
    MeshDrawer::setMesh(mesh);
    drawMesh();
};

void QtMeshDrawer::drawMesh()
{
    scene_->clear();

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
    QVector3D v = Vec3Df2QVector3D(p);
    QVector3D origin = Vec3Df2QVector3D(mesh_->origin());
    QVector3D center(view_->geometry().width() / 2, view_->geometry().height() / 2, 0);

    v -= origin;
    v = transformation_.map(v);
    v += center;

    return v.toPointF();
}

void QtMeshDrawer::rotate(const CGCP::Vec3Df &axis, double phi)
{
    transformation_.rotate(phi, axis.x(), axis.y(), axis.z());
    drawMesh();
}

void QtMeshDrawer::translate(const CGCP::Vec3Df &offset)
{
    transformation_.translate(offset.x(), offset.y(), offset.z());
    drawMesh();
}

void QtMeshDrawer::scale(const CGCP::Vec3Df &scale)
{
    transformation_.scale(scale.x(), scale.y(), scale.z());
    drawMesh();
}

QtMeshDrawer::~QtMeshDrawer()
{
}