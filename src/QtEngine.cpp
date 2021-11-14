#include "QtEngine.h"

#include "DMCPolygonizer.h"
#include "QtMeshDrawer.h"
#include "RAWLoader.h"

QtEngine::QtEngine(QGraphicsView *view) : CGCP::Engine()
{
    drawer().add(
        "main",
        [=]() -> std::unique_ptr<CGCP::MeshDrawer> {
            return std::make_unique<QtMeshDrawer>(view);
        });

    polygonizer().add(
        "dmc",
        []() -> std::unique_ptr<CGCP::Polygonizer> {
            return std::make_unique<CGCP::DMCPolygonizer>();
        });

    loader().add(
        "raw",
        []() -> std::unique_ptr<CGCP::TomographyLoader> {
            return std::make_unique<CGCP::RAWLoader>();
        });
};
