#include <chrono>

#include "DMCPolygonizer.h"
#include "DMCPolygonizer/TreeNode.h"
#include "Exception.h"

namespace CGCP

{
    const char *DMCPolygonizer::GRID_DIM_X = "grid_dim_x";
    const char *DMCPolygonizer::GRID_DIM_Y = "grid_dim_y";
    const char *DMCPolygonizer::GRID_DIM_Z = "grid_dim_z";
    const char *DMCPolygonizer::MAX_DEPTH = "max_depth";
    const char *DMCPolygonizer::NOMINAL_WEIGHT = "nominal_weight";
    const char *DMCPolygonizer::TOLERANCE = "tolerance";

    DMCPolygonizer::DMCPolygonizer()
    {
        setConfig({
            {GRID_DIM_X, "5"},
            {GRID_DIM_Y, "5"},
            {GRID_DIM_Z, "5"},
            {MAX_DEPTH, "4"},
            {NOMINAL_WEIGHT, "0.1"},
            {TOLERANCE, "0.1"},
        });
    };

    void DMCPolygonizer::setConfig(const DMCPolygonizer::Config &config)
    {
        auto dim = Vec3D<long long>(
            std::stoll(config.at(GRID_DIM_X)),
            std::stoll(config.at(GRID_DIM_Y)),
            std::stoll(config.at(GRID_DIM_Z)));

        auto max_depth = std::stod(config.at(MAX_DEPTH));
        auto tolerance = std::stod(config.at(TOLERANCE));
        auto nominal_weight = std::stod(config.at(NOMINAL_WEIGHT));

        // #TODO: Validate values

        dim_ = dim;
        max_depth_ = max_depth;
        tolerance_ = tolerance;
        nominal_weight_ = nominal_weight;
        config_ = config;
    }

    Polygonizer &DMCPolygonizer::config(const Config &config)
    {
        setConfig(config);
        return *this;
    }

    void DMCPolygonizer::threadRun(ProgressCallback progress_receiver)
    {
        progress_receiver(nullptr, 0);

        progress_receiver_ = progress_receiver;

        std::size_t total_size = dim_.x() * dim_.y() * dim_.z();

        progress_ = 0;
        total_progress_ = 2 * total_size + 1;

        auto forest = generateForest();

        if (isCancelled())
        {
            finished();
            return;
        }

        auto triangles = enumerateTriangles(forest);

        if (isCancelled())
        {
            finished();
            return;
        }

        auto origin = function_->domain().mix(Vec3Df(0.5));

        progress_receiver(
            std::make_shared<Mesh>(triangles, origin),
            1);

        finished();
    };

    void DMCPolygonizer::increaseProgress()
    {
        if (!progress_receiver_)
            return;

        progress_receiver_(nullptr, (double)(++progress_) / total_progress_);
    }
} // namespace CGCP
