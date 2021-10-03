#include "QtMeshDrawer.h"
#include <QDebug>

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
    QPointF center(view_->geometry().width() / 2, view_->geometry().height() / 2);
    return transformation_.map(QVector3D(p.x(), p.y(), p.z())).toPointF() + center;
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