#include "../DMCPolygonizer.h"
#include <omp.h>
#include <random>

namespace CGCP
{
    DMCPolygonizer::TrianglesPtr DMCPolygonizer::enumerateTriangles(
        std::vector<TreeNodePtr> &children)
    {
        std::size_t total_size = dim_.x() * dim_.y() * dim_.z();

#ifdef __USE_PARALLEL_DMC__
        std::vector<std::vector<Triangle3Df>> triangles(omp_get_max_threads());
#define TRIANGLES triangles[omp_get_thread_num()]

        std::vector<std::size_t> indices(total_size);

        for (std::size_t i = 0; i < total_size; ++i)
        {
            indices[i] = i;
        }

        std::mt19937 gen;
        std::shuffle(indices.begin(), indices.end(), gen);

#pragma omp parallel for schedule(dynamic)
        for (std::size_t i = 0; i < total_size; i++)
        {
            if (isCancelled())
                continue;

            auto j = indices[i];
#else // __USE_PARALLEL_DMC__
        auto triangles = std::make_shared<std::vector<Triangle3Df>>();
#define TRIANGLES (*triangles)

        for (std::size_t i = 0; i < total_size; i++)
        {
            if (isCancelled())
                return nullptr;

            auto j = i;
#endif // __USE_PARALLEL_DMC__

            auto ix = j % dim_.x();
            auto iy = j / dim_.x() % dim_.y();
            auto iz = j / dim_.x() / dim_.y();

            auto ix0 = ix;
            auto ix1 = ix + 1;
            auto iy0 = iy * dim_.x();
            auto iy1 = (iy + 1) * dim_.x();
            auto iz0 = iz * dim_.x() * dim_.y();
            auto iz1 = (iz + 1) * dim_.x() * dim_.y();

            enumerateCell(TRIANGLES, children[j]);

            if (ix < dim_.x() - 1)
            {
                enumerateFaceX(
                    TRIANGLES,
                    children[ix0 + iy0 + iz0],
                    children[ix1 + iy0 + iz0]);
            }

            if (iy < dim_.y() - 1)
            {
                enumerateFaceY(
                    TRIANGLES,
                    children[ix0 + iy0 + iz0],
                    children[ix0 + iy1 + iz0]);
            }

            if (iz < dim_.z() - 1)
            {
                enumerateFaceZ(
                    TRIANGLES,
                    children[ix0 + iy0 + iz0],
                    children[ix0 + iy0 + iz1]);
            }

            if (ix < dim_.x() - 1 && iy < dim_.y() - 1)
            {
                enumerateEdgeXY(
                    TRIANGLES,
                    children[ix0 + iy0 + iz0],
                    children[ix1 + iy0 + iz0],
                    children[ix0 + iy1 + iz0],
                    children[ix1 + iy1 + iz0]);
            }

            if (iy < dim_.y() - 1 && iz < dim_.z() - 1)
            {
                enumerateEdgeYZ(
                    TRIANGLES,
                    children[ix0 + iy0 + iz0],
                    children[ix0 + iy1 + iz0],
                    children[ix0 + iy0 + iz1],
                    children[ix0 + iy1 + iz1]);
            }

            if (ix < dim_.x() - 1 && iz < dim_.z() - 1)
            {
                enumerateEdgeXZ(
                    TRIANGLES,
                    children[(ix0 + iy0 + iz0)],
                    children[(ix1 + iy0 + iz0)],
                    children[(ix0 + iy0 + iz1)],
                    children[(ix1 + iy0 + iz1)]);
            }

            if (ix < dim_.x() - 1 && iy < dim_.y() - 1 && iz < dim_.z() - 1)
            {
                enumerateVertex(
                    TRIANGLES,
                    children[ix0 + iy0 + iz0],
                    children[ix1 + iy0 + iz0],
                    children[ix0 + iy1 + iz0],
                    children[ix1 + iy1 + iz0],
                    children[ix0 + iy0 + iz1],
                    children[ix1 + iy0 + iz1],
                    children[ix0 + iy1 + iz1],
                    children[ix1 + iy1 + iz1]);
            }
#ifdef __USE_PARALLEL_DMC__
#pragma omp critical
            {
                increaseProgress();
            }
        }

        if (isCancelled())
        {
            return nullptr;
        }

        std::size_t total_triangles = 0;
        for (auto &it : triangles)
        {
            total_triangles += it.size();
        }

        auto result = std::make_shared<Triangles>(total_triangles);

        for (auto &it : triangles)
        {
            if (isCancelled())
                break;

            result->insert(result->end(), it.begin(), it.end());
        }

        if (isCancelled())
        {
            finished();
            return nullptr;
        }

        return result;
#else  // __USE_PARALLEL_DMC__
            increaseProgress();
        }

        return triangles;
#endif // __USE_PARALLEL_DMC__
    }
} // namespace CGCP
