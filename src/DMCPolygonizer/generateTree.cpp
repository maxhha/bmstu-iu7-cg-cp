#include "../DMCPolygonizer.h"
#include <Eigen/Core>
#include <Eigen/Dense>

namespace CGCP
{
    std::shared_ptr<TreeNode> DMCPolygonizer::generateTree(
        ObjectPool<LeafTreeNode> &leafs,
        ObjectPool<BranchTreeNode> &branches,
        const Vec3Df &minimum,
        const Vec3Df &maximum,
        int depth)
    {
        std::array<Vec3Df, 8> points = {{
            {minimum.x(), minimum.y(), minimum.z()},
            {maximum.x(), minimum.y(), minimum.z()},
            {minimum.x(), maximum.y(), minimum.z()},
            {maximum.x(), maximum.y(), minimum.z()},
            {minimum.x(), minimum.y(), maximum.z()},
            {maximum.x(), minimum.y(), maximum.z()},
            {minimum.x(), maximum.y(), maximum.z()},
            {maximum.x(), maximum.y(), maximum.z()},
        }};

        std::array<Vec3Df, 8> grads;
        std::array<double, 8> values;

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

        Eigen::Matrix<double, 11, 4> a;

        for (int i = 0; i < 8; ++i)
        {
            a(i, 0) = grads[i].x();
            a(i, 1) = grads[i].y();
            a(i, 2) = grads[i].z();
            a(i, 3) = -1.0;
        }

        a(8, 0) = nominal_weight_;
        a(8, 1) = 0.0;
        a(8, 2) = 0.0;
        a(8, 3) = 0.0;
        a(9, 0) = 0.0;
        a(9, 1) = nominal_weight_;
        a(9, 2) = 0.0;
        a(9, 3) = 0.0;
        a(10, 0) = 0.0;
        a(10, 1) = 0.0;
        a(10, 2) = nominal_weight_;
        a(10, 3) = 0.0;

        Eigen::Matrix<double, 11, 1> b;

        auto medium = (minimum + maximum) / 2;

        for (int i = 0; i < 8; ++i)
            b(i) = grads[i].dot(points[i] - medium) - values[i];

        b(8) = 0.0;
        b(9) = 0.0;
        b(10) = 0.0;

        Eigen::Matrix<double, 4, 1> x = a.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(b);

        auto center = Vec3Df(x(0), x(1), x(2)) + medium;
        auto offset = (*function_)(center);

        double error = 0;
        for (int i = 0; i < 8; ++i)
        {
            double error_i = offset - values[i] - grads[i].dot(center - points[i]);
            error += error_i * error_i;
        }

        if (depth >= max_depth_ || error < tolerance_ * tolerance_)
        {
            return leafs.store(LeafTreeNode(FieldVertex(center, offset)));
        }
        else
        {
            std::array<std::shared_ptr<TreeNode>, 8> nodes({
                generateTree(leafs, branches,
                             Vec3Df(minimum.x(), minimum.y(), minimum.z()),
                             Vec3Df(medium.x(), medium.y(), medium.z()),
                             depth + 1),
                generateTree(leafs, branches,
                             Vec3Df(medium.x(), minimum.y(), minimum.z()),
                             Vec3Df(maximum.x(), medium.y(), medium.z()),
                             depth + 1),
                generateTree(leafs, branches,
                             Vec3Df(minimum.x(), medium.y(), minimum.z()),
                             Vec3Df(medium.x(), maximum.y(), medium.z()),
                             depth + 1),
                generateTree(leafs, branches,
                             Vec3Df(medium.x(), medium.y(), minimum.z()),
                             Vec3Df(maximum.x(), maximum.y(), medium.z()),
                             depth + 1),
                generateTree(leafs, branches,
                             Vec3Df(minimum.x(), minimum.y(), medium.z()),
                             Vec3Df(medium.x(), medium.y(), maximum.z()),
                             depth + 1),
                generateTree(leafs, branches,
                             Vec3Df(medium.x(), minimum.y(), medium.z()),
                             Vec3Df(maximum.x(), medium.y(), maximum.z()),
                             depth + 1),
                generateTree(leafs, branches,
                             Vec3Df(minimum.x(), medium.y(), medium.z()),
                             Vec3Df(medium.x(), maximum.y(), maximum.z()),
                             depth + 1),
                generateTree(leafs, branches,
                             Vec3Df(medium.x(), medium.y(), medium.z()),
                             Vec3Df(maximum.x(), maximum.y(), maximum.z()),
                             depth + 1),
            });

            return branches.store(BranchTreeNode(nodes));
        }
    };
} // namespace CGCP
