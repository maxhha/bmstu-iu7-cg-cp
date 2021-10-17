#include "QtEngine.h"

#include "DMCPolygonizer.h"
#include "QtMeshDrawer.h"

QtEngine::QtEngine(QGraphicsView *view) : CGCP::Engine()
{
    drawer().add("main", [=]() -> std::unique_ptr<CGCP::MeshDrawer> { return std::make_unique<QtMeshDrawer>(view); });
    polygonizer().add("dmc", [=]() -> std::unique_ptr<CGCP::Polygonizer> { return std::make_unique<CGCP::DMCPolygonizer>(); });
};