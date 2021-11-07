#include "QtEngine.h"

#include "DMCPolygonizer.h"
#include "FieldFunction.h"
#include "QtMeshDrawer.h"
#include "RAWLoader.h"

using Config = std::map<std::string, std::string>;
using CFPtr = std::shared_ptr<CGCP::ContinuesFunction>;

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

    function()
        .addType(
            "average",
            [&](const Config &config, CFPtr f) -> CFPtr
            {
                int size = std::stoi(config.at("size"));
                double step_x = std::stod(config.at("step_x"));
                double step_y = std::stod(config.at("step_y"));
                double step_z = std::stod(config.at("step_z"));

                return std::make_shared<CGCP::FieldFunction>(
                    [=](double x, double y, double z) -> double
                    {
                        double sum = 0;

                        for (int i = -size; i <= size; i++)
                        {
                            for (int j = -size; j <= size; j++)
                            {
                                for (int k = -size; k <= size; k++)
                                {
                                    sum += (*f)(CGCP::Vec3Df(
                                        x + i * step_x,
                                        y + j * step_y,
                                        z + k * step_z));
                                }
                            }
                        }

                        return sum / (size * 2 + 1) / (size * 2 + 1) / (size * 2 + 1);
                    },
                    f->domain());
            })
        .addType(
            "offset",
            [&](const Config &config, CFPtr f) -> CFPtr
            {
                double offset = std::stod(config.at("value"));

                return std::make_shared<CGCP::FieldFunction>(
                    [=](double x, double y, double z) -> double
                    {
                        return (*f)(CGCP::Vec3Df(x, y, z)) + offset;
                    },
                    f->domain());
            });
};
