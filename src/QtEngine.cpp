#include "QtEngine.h"

#include "QtMeshDrawer.h"

QtEngine::QtEngine(QGraphicsView *view) : CGCP::Engine()
{
    drawer().add("main", [=]() -> std::unique_ptr<CGCP::MeshDrawer> { return std::make_unique<QtMeshDrawer>(view); });
};