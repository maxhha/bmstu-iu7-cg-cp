#include "QtMeshDrawer.h"
#include <QDebug>

QtMeshDrawer::QtMeshDrawer(QGraphicsView *view) : view_(view), scene_(new QGraphicsScene(view))
{
    view->setScene(scene_);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene_->setSceneRect(0, 0, view->geometry().width(), view->geometry().height());

    QPolygonF polygon;
    polygon << QPointF(10, 100) << QPointF(100, 100) << QPointF(100, 10);

    scene_->addPolygon(polygon);
    view->show();
};

void QtMeshDrawer::setMesh(std::shared_ptr<CGCP::Mesh> mesh){

};

QtMeshDrawer::~QtMeshDrawer()
{
}