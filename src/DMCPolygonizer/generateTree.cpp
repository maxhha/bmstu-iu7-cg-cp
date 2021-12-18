#include "../DMCPolygonizer.h"
#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>

#define N 4

namespace CGCP
{
    std::shared_ptr<TreeNode> DMCPolygonizer::generateTree(
#ifdef __USE_PARALLEL_DMC__
        ObjectPool<LeafTreeNode> &leafs,
        ObjectPool<BranchTreeNode> &branches,
#endif // __USE_PARALLEL_DMC__
        const Vec3Df &minimum,
        const Vec3Df &maximum,
        int depth)
    {
        if (isCancelled())
        {
            return nullptr;
        }

        std::array<Vec3Df, N * N * N> points;

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                for (int k = 0; k < N; ++k)
                {
                    points[i * N * N + j * N + k] = minimum.mix(
                        maximum,
                        Vec3Df(
                            i / (N - 1),
                            j / (N - 1),
                            k / (N - 1)));
                }
            }
        }

        std::array<Vec3Df, N * N * N> grads;
        std::array<double, N * N * N> values;

        std::transform(
            points.begin(),
            points.end(),
            grads.begin(),
            [&](const auto &p)
            { return function_->grad(p); });

        std::transform(
            points.begin(),
            points.end(),
            values.begin(),
            [&](const auto &p)
            { return (*function_)(p); });

        Eigen::Matrix<double, 3 + N * N * N, 4> a;

        int i;

        for (i = 0; i < N * N * N; ++i)
        {
            a(i, 0) = grads[i].x();
            a(i, 1) = grads[i].y();
            a(i, 2) = grads[i].z();
            a(i, 3) = -1.0;
        }

        a(i, 0) = nominal_weight_;
        a(i, 1) = 0.0;
        a(i, 2) = 0.0;
        a(i, 3) = 0.0;
        ++i;
        a(i, 0) = 0.0;
        a(i, 1) = nominal_weight_;
        a(i, 2) = 0.0;
        a(i, 3) = 0.0;
        ++i;
        a(i, 0) = 0.0;
        a(i, 1) = 0.0;
        a(i, 2) = nominal_weight_;
        a(i, 3) = 0.0;

        Eigen::Matrix<double, 3 + N * N * N, 1> b;

        auto medium = (minimum + maximum) / 2;

        for (i = 0; i < N * N * N; ++i)
            b(i) = grads[i].dot(points[i] - medium) - values[i];

        b(i++) = 0.0;
        b(i++) = 0.0;
        b(i++) = 0.0;

        auto opts = Eigen::ComputeFullU | Eigen::ComputeFullV;
        Eigen::Matrix<double, 4, 1> x = a.jacobiSvd(opts).solve(b);

        auto center = Vec3Df(x(0), x(1), x(2)) + medium;

        if (
            center.x() < minimum.x() ||
            center.y() < minimum.y() ||
            center.z() < minimum.z() ||
            center.x() > maximum.x() ||
            center.y() > maximum.y() ||
            center.z() > maximum.z())
        {
            auto l = std::max({std::abs(x(0)), std::abs(x(1)), std::abs(x(2))});
            auto k = Vec3Df(x(0), x(1), x(2)) / Vec3Df(l * 2) + Vec3Df(0.5);
            center = minimum.mix(maximum, k);
        }

        auto offset = (*function_)(center);

        double error = 0;
        for (int i = 0; i < N * N * N; ++i)
        {
            double ti = values[i] + grads[i].dot(center - points[i]);
            double error_i = offset - ti;
            error += error_i * error_i / (1 + grads[i].dot(grads[i]));
        }

        if (depth >= max_depth_ || error < tolerance_)
        {
#ifdef __USE_PARALLEL_DMC__
            return leafs.store(LeafTreeNode(FieldVertex(center, offset)));
#else  // __USE_PARALLEL_DMC__
            return std::make_shared<LeafTreeNode>(FieldVertex(center, offset));
#endif // __USE_PARALLEL_DMC__
        }
        else
        {
#ifdef __USE_PARALLEL_DMC__
#define GENERATE_TREE(__from__, __to__) \
    generateTree(leafs, branches, __from__, __to__, depth + 1)
#else // __USE_PARALLEL_DMC__
#define GENERATE_TREE(__from__, __to__) \
    generateTree(__from__, __to__, depth + 1)
#endif // __USE_PARALLEL_DMC__

            std::array<std::shared_ptr<TreeNode>, 8> nodes({
                GENERATE_TREE(
                    Vec3Df(minimum.x(), minimum.y(), minimum.z()),
                    Vec3Df(medium.x(), medium.y(), medium.z())),
                GENERATE_TREE(
                    Vec3Df(medium.x(), minimum.y(), minimum.z()),
                    Vec3Df(maximum.x(), medium.y(), medium.z())),
                GENERATE_TREE(
                    Vec3Df(minimum.x(), medium.y(), minimum.z()),
                    Vec3Df(medium.x(), maximum.y(), medium.z())),
                GENERATE_TREE(
                    Vec3Df(medium.x(), medium.y(), minimum.z()),
                    Vec3Df(maximum.x(), maximum.y(), medium.z())),
                GENERATE_TREE(
                    Vec3Df(minimum.x(), minimum.y(), medium.z()),
                    Vec3Df(medium.x(), medium.y(), maximum.z())),
                GENERATE_TREE(
                    Vec3Df(medium.x(), minimum.y(), medium.z()),
                    Vec3Df(maximum.x(), medium.y(), maximum.z())),
                GENERATE_TREE(
                    Vec3Df(minimum.x(), medium.y(), medium.z()),
                    Vec3Df(medium.x(), maximum.y(), maximum.z())),
                GENERATE_TREE(
                    Vec3Df(medium.x(), medium.y(), medium.z()),
                    Vec3Df(maximum.x(), maximum.y(), maximum.z())),
            });

#undef GENERATE_TREE

#ifdef __USE_PARALLEL_DMC__
            return branches.store(BranchTreeNode(nodes));
#else  // __USE_PARALLEL_DMC__
            return std::make_shared<BranchTreeNode>(nodes);
#endif // __USE_PARALLEL_DMC__
        }
    };
} // namespace CGCP
