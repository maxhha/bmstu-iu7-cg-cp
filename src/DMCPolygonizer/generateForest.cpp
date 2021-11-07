#include "../DMCPolygonizer.h"
#include <omp.h>
#include <random>

namespace CGCP
{
    std::vector<std::shared_ptr<TreeNode>> DMCPolygonizer::generateForest()
    {
        std::size_t total_size = dim_.x() * dim_.y() * dim_.z();

        std::vector<std::shared_ptr<TreeNode>> children(total_size);
        std::vector<std::size_t> indices(total_size);

        for (std::size_t i = 0; i < total_size; ++i)
        {
            indices[i] = i;
        }

        std::mt19937 gen;
        std::shuffle(indices.begin(), indices.end(), gen);

        std::vector<ObjectPool<LeafTreeNode>> leafs(omp_get_max_threads());
        std::vector<ObjectPool<BranchTreeNode>> branches(omp_get_max_threads());

#pragma omp parallel for schedule(dynamic)
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(total_size); ++i)
        {
            if (isCancelled())
                continue;

            auto j = indices[i];
            Vec3Df v(
                j % dim_.x(),
                j / dim_.x() % dim_.y(),
                j / dim_.x() / dim_.y());

            auto from = function_->domain().mix(v / dim_);
            auto to = function_->domain().mix((v + Vec3Df(1)) / dim_);

            children[j] = generateTree(
                leafs[omp_get_thread_num()],
                branches[omp_get_thread_num()],
                from,
                to);

#pragma omp critical
            {
                increaseProgress();
            }
        }

        return std::move(children);
    }
} // namespace CGCP
