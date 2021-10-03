#include "QtMeshDrawer.h"
#include <QDebug>

QtMeshDrawer::QtMeshDrawer(QGraphicsView *view) : view_(view), scene_(new QGraphicsScene(view))
{
    view->setScene(scene_);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene_->setSceneRect(0, 0, view->geometry().width(), view->geometry().height());

    // QPolygonF polygon;
    // polygon << QPointF(10, 100) << QPointF(100, 100) << QPointF(100, 10);

    // scene_->addPolygon(polygon);
    view->show();
};

void QtMeshDrawer::setMesh(const std::shared_ptr<CGCP::Mesh> mesh)
{
    MeshDrawer::setMesh(mesh);

    scene_->clear();

    for (const auto &it : mesh->triangles())
    {
        QPolygonF polygon;

        polygon << QPointF(it.p1().x(), it.p1().y());
        polygon << QPointF(it.p2().x(), it.p2().y());
        polygon << QPointF(it.p3().x(), it.p3().y());

        scene_->addPolygon(polygon);
    }

    view_->show();
};

QtMeshDrawer::~QtMeshDrawer()
{
}