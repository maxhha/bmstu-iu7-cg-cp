#include <Eigen/Core>
#include <Eigen/Dense>
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
        std::size_t total_progress = total_size + 1;

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
            auto to = function_->domain().mix((v + Vec3Df(1, 1, 1)) / dim);

            children[j] = generateTree(from, to);

            // std::this_thread::sleep_for(std::chrono::milliseconds(1));

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

        auto triangles = std::make_shared<std::vector<CGCP::Triangle3Df>>();

        for (std::size_t i = 0; i < total_size; i++)
        {
            std::stack<std::shared_ptr<TreeNode>> tree_stack({children[i]});

            while (tree_stack.size() > 0)
            {
                auto cur = tree_stack.top();
                tree_stack.pop();

                auto cur_branch = std::dynamic_pointer_cast<BranchTreeNode>(cur);
                auto cur_leaf = std::dynamic_pointer_cast<LeafTreeNode>(cur);

                if (cur_branch)
                {
                    for (auto &it : cur_branch->children())
                    {
                        tree_stack.push(it);
                    }
                }
                else
                {
                    double x = cur_leaf->offset().x() * 0.95;
                    double y = cur_leaf->offset().y() * 0.95;
                    double z = cur_leaf->offset().z() * 0.95;

                    CGCP::Vec3Df v0(-x, -y, -z);
                    CGCP::Vec3Df v1(-x, -y, +z);
                    CGCP::Vec3Df v2(-x, +y, -z);
                    CGCP::Vec3Df v3(-x, +y, +z);
                    CGCP::Vec3Df v4(+x, -y, -z);
                    CGCP::Vec3Df v5(+x, -y, +z);
                    CGCP::Vec3Df v6(+x, +y, -z);
                    CGCP::Vec3Df v7(+x, +y, +z);

                    v0 = v0 + cur_leaf->center();
                    v1 = v1 + cur_leaf->center();
                    v2 = v2 + cur_leaf->center();
                    v3 = v3 + cur_leaf->center();
                    v4 = v4 + cur_leaf->center();
                    v5 = v5 + cur_leaf->center();
                    v6 = v6 + cur_leaf->center();
                    v7 = v7 + cur_leaf->center();

                    triangles->emplace_back(CGCP::Triangle3Df(v0, v1, v3));
                    triangles->emplace_back(CGCP::Triangle3Df(v0, v3, v2));
                    triangles->emplace_back(CGCP::Triangle3Df(v4, v7, v5));
                    triangles->emplace_back(CGCP::Triangle3Df(v4, v6, v7));

                    triangles->emplace_back(CGCP::Triangle3Df(v0, v5, v1));
                    triangles->emplace_back(CGCP::Triangle3Df(v0, v4, v5));
                    triangles->emplace_back(CGCP::Triangle3Df(v2, v3, v7));
                    triangles->emplace_back(CGCP::Triangle3Df(v2, v7, v6));

                    triangles->emplace_back(CGCP::Triangle3Df(v1, v7, v3));
                    triangles->emplace_back(CGCP::Triangle3Df(v1, v5, v7));
                    triangles->emplace_back(CGCP::Triangle3Df(v0, v6, v4));
                    triangles->emplace_back(CGCP::Triangle3Df(v0, v2, v6));
                }
            }
        }

        result = std::make_shared<Mesh>(triangles, function_->domain().mix(Vec3Df(0.5, 0.5, 0.5)));

        progress_receiver(result, 1);

        finished();
    };

    std::shared_ptr<TreeNode> DMCPolygonizer::generateTree(const Vec3Df &minimum, const Vec3Df &maximum, int depth)
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

        // qDebug() << "error" << error;

        if (depth >= max_depth_ || error < tolerance_ * tolerance_)
        {
            return std::make_shared<LeafTreeNode>(center, (maximum - minimum) / Vec3Df(2, 2, 2));
        }
        else
        {
            std::array<std::shared_ptr<TreeNode>, 8> nodes({
                generateTree(Vec3Df(minimum.x(), minimum.y(), minimum.z()), Vec3Df(medium.x(), medium.y(), medium.z()), depth + 1),
                generateTree(Vec3Df(medium.x(), minimum.y(), minimum.z()), Vec3Df(maximum.x(), medium.y(), medium.z()), depth + 1),
                generateTree(Vec3Df(minimum.x(), medium.y(), minimum.z()), Vec3Df(medium.x(), maximum.y(), medium.z()), depth + 1),
                generateTree(Vec3Df(medium.x(), medium.y(), minimum.z()), Vec3Df(maximum.x(), maximum.y(), medium.z()), depth + 1),
                generateTree(Vec3Df(minimum.x(), minimum.y(), medium.z()), Vec3Df(medium.x(), medium.y(), maximum.z()), depth + 1),
                generateTree(Vec3Df(medium.x(), minimum.y(), medium.z()), Vec3Df(maximum.x(), medium.y(), maximum.z()), depth + 1),
                generateTree(Vec3Df(minimum.x(), medium.y(), medium.z()), Vec3Df(medium.x(), maximum.y(), maximum.z()), depth + 1),
                generateTree(Vec3Df(medium.x(), medium.y(), medium.z()), Vec3Df(maximum.x(), maximum.y(), maximum.z()), depth + 1),
            });

            return std::make_shared<BranchTreeNode>(nodes);
        }
    };
} // namespace CGCP
