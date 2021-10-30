#include <QDebug>
#include <chrono>
#include <functional>
#include <omp.h>
#include <random>
#include <stack>

#include "DMCPolygonizer.h"
#include "DMCPolygonizer/TreeNode.h"

namespace CGCP

{
    const char *DMCPolygonizer::GRID_DIM_X = "grid_dim_x";
    const char *DMCPolygonizer::GRID_DIM_Y = "grid_dim_y";
    const char *DMCPolygonizer::GRID_DIM_Z = "grid_dim_z";
    const char *DMCPolygonizer::GRID_SIZE_X = "grid_size_x";
    const char *DMCPolygonizer::GRID_SIZE_Y = "grid_size_y";
    const char *DMCPolygonizer::GRID_SIZE_Z = "grid_size_z";
    const char *DMCPolygonizer::MAX_DEPTH = "max_depth";
    const char *DMCPolygonizer::NOMINAL_WEIGHT = "nominal_weight";
    const char *DMCPolygonizer::TOLERANCE = "tolerance";

    DMCPolygonizer::DMCPolygonizer() : Polygonizer({
                                           {GRID_DIM_X, "5"},
                                           {GRID_DIM_Y, "5"},
                                           {GRID_DIM_Z, "5"},
                                           {GRID_SIZE_X, "10"},
                                           {GRID_SIZE_Y, "10"},
                                           {GRID_SIZE_Z, "10"},
                                           {MAX_DEPTH, "1"},
                                           {NOMINAL_WEIGHT, "0.1"},
                                           {TOLERANCE, "0.001"},
                                       }){};

    void DMCPolygonizer::validate(const Config &config){};

    void DMCPolygonizer::threadRun(ProgressCallback progress_receiver)
    {
        std::shared_ptr<Mesh> result;
        progress_receiver(result, 0);

        Vec3Di dim(
            std::stoi(config_[GRID_DIM_X]),
            std::stoi(config_[GRID_DIM_Y]),
            std::stoi(config_[GRID_DIM_Z]));

        max_depth_ = std::stod(config_[MAX_DEPTH]);
        tolerance_ = std::stod(config_[TOLERANCE]);
        nominal_weight_ = std::stod(config_[NOMINAL_WEIGHT]);

        std::size_t total_size = (std::size_t)dim.x() * dim.y() * dim.z();

        std::vector<std::shared_ptr<TreeNode>> children(total_size);
        std::vector<std::size_t> indices(total_size);

        for (std::size_t i = 0; i < total_size; ++i)
        {
            indices[i] = i;
        }

        std::mt19937 gen;
        std::shuffle(indices.begin(), indices.end(), gen);

        std::size_t progress = 0;
        std::size_t total_progress = 2 * total_size + 1;

#pragma omp parallel for schedule(dynamic)
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(total_size); ++i)
        {
            if (isCancelled())
                continue;

            auto j = indices[i];
            Vec3Df v(
                j % dim.x(),
                j / dim.x() % dim.y(),
                j / dim.x() / dim.y());

            auto from = function_->domain().mix(v / dim);
            auto to = function_->domain().mix((v + Vec3Df(1)) / dim);

            children[j] = generateTree(from, to);

#pragma omp critical
            {
                progress_receiver(result, (double)(++progress) / total_progress);
            }
        }

        if (isCancelled())
        {
            finished();
            return;
        }

        // #define C 10
        // #define S (C / 2)
        // CGCP::Vec3Df v0(-S, -S, -S);
        // CGCP::Vec3Df v1(-S, -S, +S);
        // CGCP::Vec3Df v2(-S, +S, -S);
        // CGCP::Vec3Df v3(-S, +S, +S);
        // CGCP::Vec3Df v4(+S, -S, -S);
        // CGCP::Vec3Df v5(+S, -S, +S);
        // CGCP::Vec3Df v6(+S, +S, -S);
        // CGCP::Vec3Df v7(+S, +S, +S);

        // result = std::make_shared<CGCP::Mesh>(CGCP::Mesh({
        //     CGCP::Triangle3Df(v0, v1, v3),
        //     CGCP::Triangle3Df(v0, v3, v2),
        //     CGCP::Triangle3Df(v4, v7, v5),
        //     CGCP::Triangle3Df(v4, v6, v7),

        //     CGCP::Triangle3Df(v0, v5, v1),
        //     CGCP::Triangle3Df(v0, v4, v5),
        //     CGCP::Triangle3Df(v2, v3, v7),
        //     CGCP::Triangle3Df(v2, v7, v6),

        //     CGCP::Triangle3Df(v1, v7, v3),
        //     CGCP::Triangle3Df(v1, v5, v7),
        //     CGCP::Triangle3Df(v0, v6, v4),
        //     CGCP::Triangle3Df(v0, v2, v6),
        // }));

        // result->origin() = CGCP::Vec3Df(0, 0, 0);

        std::vector<std::vector<CGCP::Triangle3Df>> triangles(omp_get_max_threads());

#pragma omp parallel for schedule(dynamic)
        for (std::size_t i = 0; i < total_size; i++)
        {
            if (isCancelled())
                continue;

            auto j = indices[i];

            auto ix = j % dim.x();
            auto iy = j / dim.x() % dim.y();
            auto iz = j / dim.x() / dim.y();

            auto ix0 = ix;
            auto ix1 = ix + 1;
            auto iy0 = iy * dim.x();
            auto iy1 = (iy + 1) * dim.x();
            auto iz0 = iz * dim.x() * dim.z();
            auto iz1 = (iz + 1) * dim.x() * dim.z();

            enumerateCell(triangles[omp_get_thread_num()], children[j]);

            if (ix != dim.x() - 1)
            {
                enumerateFaceX(
                    triangles[omp_get_thread_num()],
                    children[ix0 + iy0 + iz0],
                    children[ix1 + iy0 + iz0]);
            }

            if (iy != dim.y() - 1)
            {
                enumerateFaceY(
                    triangles[omp_get_thread_num()],
                    children[ix0 + iy0 + iz0],
                    children[ix0 + iy1 + iz0]);
            }

            if (iz != dim.z() - 1)
            {
                enumerateFaceZ(
                    triangles[omp_get_thread_num()],
                    children[ix0 + iy0 + iz0],
                    children[ix0 + iy0 + iz1]);
            }

            if (ix != dim.x() - 1 && iy != dim.y() - 1)
            {
                enumerateEdgeXY(
                    triangles[omp_get_thread_num()],
                    children[ix0 + iy0 + iz0],
                    children[ix1 + iy0 + iz0],
                    children[ix0 + iy1 + iz0],
                    children[ix1 + iy1 + iz0]);
            }

            if (iy != dim.y() - 1 && iz != dim.z() - 1)
            {
                enumerateEdgeYZ(
                    triangles[omp_get_thread_num()],
                    children[ix0 + iy0 + iz0],
                    children[ix0 + iy1 + iz0],
                    children[ix0 + iy0 + iz1],
                    children[ix0 + iy1 + iz1]);
            }

            if (ix != dim.x() - 1 && iz != dim.z() - 1)
            {
                enumerateEdgeXZ(
                    triangles[omp_get_thread_num()],
                    children[(ix0 + iy0 + iz0)],
                    children[(ix1 + iy0 + iz0)],
                    children[(ix0 + iy0 + iz1)],
                    children[(ix1 + iy0 + iz1)]);
            }

            if (ix != dim.x() - 1 && iy != dim.y() - 1 && iz != dim.z() - 1)
            {
                enumerateVertex(
                    triangles[omp_get_thread_num()],
                    children[ix0 + iy0 + iz0],
                    children[ix1 + iy0 + iz0],
                    children[ix0 + iy1 + iz0],
                    children[ix1 + iy1 + iz0],
                    children[ix0 + iy0 + iz1],
                    children[ix1 + iy0 + iz1],
                    children[ix0 + iy1 + iz1],
                    children[ix1 + iy1 + iz1]);
            }

#pragma omp critical
            {
                progress_receiver(result, (double)(++progress) / total_progress);
            }
        }

        if (isCancelled())
        {
            finished();
            return;
        }

        result = std::make_shared<Mesh>();
        result->origin() = function_->domain().mix(Vec3Df(0.5, 0.5, 0.5));

        for (auto &it : triangles)
        {
            if (isCancelled())
                break;

            result->triangles().insert(
                result->triangles().end(),
                std::make_move_iterator(it.begin()),
                std::make_move_iterator(it.end()));

            it.erase(it.begin(), it.end());
        }

        if (isCancelled())
        {
            finished();
            return;
        }

        progress_receiver(result, 1);

        finished();
    };

} // namespace CGCP
