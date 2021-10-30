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
            auto to = function_->domain().mix((v + Vec3Df(1, 1, 1)) / dim);

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

            if (ix != dim.x() - 1 && iy != dim.y() - 1 && iz != dim.z() - 1)
            {

                enumerateVertex(triangles[omp_get_thread_num()],
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

        if (depth >= max_depth_ || error < tolerance_ * tolerance_)
        {
            return std::make_shared<LeafTreeNode>(FieldVertex(center, offset));
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

    void DMCPolygonizer::enumerateVertex(
        Triangles &triangles,
        const TreeNodePtr n_xyz, const TreeNodePtr n_Xyz,
        const TreeNodePtr n_xYz, const TreeNodePtr n_XYz,
        const TreeNodePtr n_xyZ, const TreeNodePtr n_XyZ,
        const TreeNodePtr n_xYZ, const TreeNodePtr n_XYZ)
    {
        auto b_xyz = std::dynamic_pointer_cast<BranchTreeNode>(n_xyz);
        auto b_Xyz = std::dynamic_pointer_cast<BranchTreeNode>(n_Xyz);
        auto b_xYz = std::dynamic_pointer_cast<BranchTreeNode>(n_xYz);
        auto b_XYz = std::dynamic_pointer_cast<BranchTreeNode>(n_XYz);
        auto b_xyZ = std::dynamic_pointer_cast<BranchTreeNode>(n_xyZ);
        auto b_XyZ = std::dynamic_pointer_cast<BranchTreeNode>(n_XyZ);
        auto b_xYZ = std::dynamic_pointer_cast<BranchTreeNode>(n_xYZ);
        auto b_XYZ = std::dynamic_pointer_cast<BranchTreeNode>(n_XYZ);

        if (b_xyz || b_Xyz || b_xYz || b_XYz || b_xyZ || b_XyZ || b_xYZ || b_XYZ)
        {
            enumerateVertex(
                triangles,
                b_xyz ? b_xyz->XYZ() : n_xyz,
                b_Xyz ? b_Xyz->xYZ() : n_Xyz,
                b_xYz ? b_xYz->XyZ() : n_xYz,
                b_XYz ? b_XYz->xyZ() : n_XYz,
                b_xyZ ? b_xyZ->XYz() : n_xyZ,
                b_XyZ ? b_XyZ->xYz() : n_XyZ,
                b_xYZ ? b_xYZ->Xyz() : n_xYZ,
                b_XYZ ? b_XYZ->xyz() : n_XYZ);
        }
        else
        {
            auto l_xyz = std::dynamic_pointer_cast<LeafTreeNode>(n_xyz);
            auto l_Xyz = std::dynamic_pointer_cast<LeafTreeNode>(n_Xyz);
            auto l_xYz = std::dynamic_pointer_cast<LeafTreeNode>(n_xYz);
            auto l_XYz = std::dynamic_pointer_cast<LeafTreeNode>(n_XYz);
            auto l_xyZ = std::dynamic_pointer_cast<LeafTreeNode>(n_xyZ);
            auto l_XyZ = std::dynamic_pointer_cast<LeafTreeNode>(n_XyZ);
            auto l_xYZ = std::dynamic_pointer_cast<LeafTreeNode>(n_xYZ);
            auto l_XYZ = std::dynamic_pointer_cast<LeafTreeNode>(n_XYZ);

            std::array<const FieldVertex *, 8> vertices = {
                &l_xyz->vertex(),
                &l_Xyz->vertex(),
                &l_xYz->vertex(),
                &l_XYz->vertex(),
                &l_xyZ->vertex(),
                &l_XyZ->vertex(),
                &l_xYZ->vertex(),
                &l_XYZ->vertex(),
            };

            marchingCube(triangles, vertices);
        }
    }

    void DMCPolygonizer::marchingCube(Triangles &triangles, const std::array<const FieldVertex *, 8> &vertices)
    {
        static struct
        {
            int trans[8];
            bool flip;
            int code;
        } table[256] =
            {
                /* 00000000 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00000000},
                /* 00000001 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00000001},
                /* 00000010 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00000001},
                /* 00000011 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00000011},
                /* 00000100 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00000001},
                /* 00000101 */ {{0, 2, 1, 3, 4, 6, 5, 7}, 1, 0b00000011},
                /* 00000110 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00000110},
                /* 00000111 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00000111},
                /* 00001000 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b00000001},
                /* 00001001 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00000110},
                /* 00001010 */ {{1, 3, 0, 2, 5, 7, 4, 6}, 0, 0b00000011},
                /* 00001011 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00000111},
                /* 00001100 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00000011},
                /* 00001101 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00000111},
                /* 00001110 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b00000111},
                /* 00001111 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00001111},
                /* 00010000 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00000001},
                /* 00010001 */ {{0, 4, 2, 6, 1, 5, 3, 7}, 1, 0b00000011},
                /* 00010010 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b00000110},
                /* 00010011 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b00000111},
                /* 00010100 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b00000110},
                /* 00010101 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b00000111},
                /* 00010110 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00010110},
                /* 00010111 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00010111},
                /* 00011000 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00011000},
                /* 00011001 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00011001},
                /* 00011010 */ {{1, 0, 5, 4, 3, 2, 7, 6}, 0, 0b00011001},
                /* 00011011 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00011011},
                /* 00011100 */ {{2, 0, 6, 4, 3, 1, 7, 5}, 1, 0b00011001},
                /* 00011101 */ {{0, 2, 1, 3, 4, 6, 5, 7}, 1, 0b00011011},
                /* 00011110 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00011110},
                /* 00011111 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00011111},
                /* 00100000 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00000001},
                /* 00100001 */ {{1, 0, 5, 4, 3, 2, 7, 6}, 0, 0b00000110},
                /* 00100010 */ {{1, 5, 3, 7, 0, 4, 2, 6}, 0, 0b00000011},
                /* 00100011 */ {{1, 0, 5, 4, 3, 2, 7, 6}, 0, 0b00000111},
                /* 00100100 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b00011000},
                /* 00100101 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b00011001},
                /* 00100110 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00011001},
                /* 00100111 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b00011011},
                /* 00101000 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b00000110},
                /* 00101001 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00010110},
                /* 00101010 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b00000111},
                /* 00101011 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00010111},
                /* 00101100 */ {{3, 1, 7, 5, 2, 0, 6, 4}, 0, 0b00011001},
                /* 00101101 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00011110},
                /* 00101110 */ {{1, 3, 0, 2, 5, 7, 4, 6}, 0, 0b00011011},
                /* 00101111 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00011111},
                /* 00110000 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00000011},
                /* 00110001 */ {{4, 5, 0, 1, 6, 7, 2, 3}, 0, 0b00000111},
                /* 00110010 */ {{5, 4, 1, 0, 7, 6, 3, 2}, 1, 0b00000111},
                /* 00110011 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b00001111},
                /* 00110100 */ {{4, 6, 0, 2, 5, 7, 1, 3}, 1, 0b00011001},
                /* 00110101 */ {{0, 4, 1, 5, 2, 6, 3, 7}, 0, 0b00011011},
                /* 00110110 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b00011110},
                /* 00110111 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b00011111},
                /* 00111000 */ {{5, 7, 1, 3, 4, 6, 0, 2}, 0, 0b00011001},
                /* 00111001 */ {{1, 0, 5, 4, 3, 2, 7, 6}, 0, 0b00011110},
                /* 00111010 */ {{1, 5, 0, 4, 3, 7, 2, 6}, 1, 0b00011011},
                /* 00111011 */ {{1, 0, 5, 4, 3, 2, 7, 6}, 0, 0b00011111},
                /* 00111100 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00111100},
                /* 00111101 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00111101},
                /* 00111110 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b00111101},
                /* 00111111 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b00111111},
                /* 01000000 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00000001},
                /* 01000001 */ {{2, 0, 6, 4, 3, 1, 7, 5}, 1, 0b00000110},
                /* 01000010 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b00011000},
                /* 01000011 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b00011001},
                /* 01000100 */ {{2, 6, 0, 4, 3, 7, 1, 5}, 0, 0b00000011},
                /* 01000101 */ {{2, 0, 6, 4, 3, 1, 7, 5}, 1, 0b00000111},
                /* 01000110 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00011001},
                /* 01000111 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b00011011},
                /* 01001000 */ {{2, 3, 6, 7, 0, 1, 4, 5}, 0, 0b00000110},
                /* 01001001 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00010110},
                /* 01001010 */ {{3, 2, 7, 6, 1, 0, 5, 4}, 1, 0b00011001},
                /* 01001011 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00011110},
                /* 01001100 */ {{2, 3, 6, 7, 0, 1, 4, 5}, 0, 0b00000111},
                /* 01001101 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00010111},
                /* 01001110 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00011011},
                /* 01001111 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00011111},
                /* 01010000 */ {{4, 6, 5, 7, 0, 2, 1, 3}, 0, 0b00000011},
                /* 01010001 */ {{4, 6, 0, 2, 5, 7, 1, 3}, 1, 0b00000111},
                /* 01010010 */ {{4, 5, 0, 1, 6, 7, 2, 3}, 0, 0b00011001},
                /* 01010011 */ {{0, 4, 2, 6, 1, 5, 3, 7}, 1, 0b00011011},
                /* 01010100 */ {{6, 4, 2, 0, 7, 5, 3, 1}, 0, 0b00000111},
                /* 01010101 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b00001111},
                /* 01010110 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b00011110},
                /* 01010111 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b00011111},
                /* 01011000 */ {{6, 7, 2, 3, 4, 5, 0, 1}, 1, 0b00011001},
                /* 01011001 */ {{2, 0, 6, 4, 3, 1, 7, 5}, 1, 0b00011110},
                /* 01011010 */ {{0, 2, 1, 3, 4, 6, 5, 7}, 1, 0b00111100},
                /* 01011011 */ {{0, 2, 1, 3, 4, 6, 5, 7}, 1, 0b00111101},
                /* 01011100 */ {{2, 6, 0, 4, 3, 7, 1, 5}, 0, 0b00011011},
                /* 01011101 */ {{2, 0, 6, 4, 3, 1, 7, 5}, 1, 0b00011111},
                /* 01011110 */ {{2, 0, 3, 1, 6, 4, 7, 5}, 0, 0b00111101},
                /* 01011111 */ {{0, 2, 1, 3, 4, 6, 5, 7}, 1, 0b00111111},
                /* 01100000 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00000110},
                /* 01100001 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00010110},
                /* 01100010 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00011001},
                /* 01100011 */ {{4, 5, 0, 1, 6, 7, 2, 3}, 0, 0b00011110},
                /* 01100100 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00011001},
                /* 01100101 */ {{4, 6, 0, 2, 5, 7, 1, 3}, 1, 0b00011110},
                /* 01100110 */ {{0, 4, 2, 6, 1, 5, 3, 7}, 1, 0b00111100},
                /* 01100111 */ {{0, 4, 2, 6, 1, 5, 3, 7}, 1, 0b00111101},
                /* 01101000 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b00010110},
                /* 01101001 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b01101001},
                /* 01101010 */ {{7, 5, 3, 1, 6, 4, 2, 0}, 1, 0b00011110},
                /* 01101011 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b01101011},
                /* 01101100 */ {{7, 6, 3, 2, 5, 4, 1, 0}, 0, 0b00011110},
                /* 01101101 */ {{0, 2, 1, 3, 4, 6, 5, 7}, 1, 0b01101011},
                /* 01101110 */ {{3, 7, 1, 5, 2, 6, 0, 4}, 1, 0b00111101},
                /* 01101111 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b01101111},
                /* 01110000 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00000111},
                /* 01110001 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00010111},
                /* 01110010 */ {{4, 5, 0, 1, 6, 7, 2, 3}, 0, 0b00011011},
                /* 01110011 */ {{4, 5, 0, 1, 6, 7, 2, 3}, 0, 0b00011111},
                /* 01110100 */ {{4, 6, 0, 2, 5, 7, 1, 3}, 1, 0b00011011},
                /* 01110101 */ {{4, 6, 0, 2, 5, 7, 1, 3}, 1, 0b00011111},
                /* 01110110 */ {{4, 0, 6, 2, 5, 1, 7, 3}, 0, 0b00111101},
                /* 01110111 */ {{0, 4, 2, 6, 1, 5, 3, 7}, 1, 0b00111111},
                /* 01111000 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b00011110},
                /* 01111001 */ {{0, 4, 2, 6, 1, 5, 3, 7}, 1, 0b01101011},
                /* 01111010 */ {{5, 7, 4, 6, 1, 3, 0, 2}, 1, 0b00111101},
                /* 01111011 */ {{0, 1, 4, 5, 2, 3, 6, 7}, 1, 0b01101111},
                /* 01111100 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00111101},
                /* 01111101 */ {{0, 2, 4, 6, 1, 3, 5, 7}, 0, 0b01101111},
                /* 01111110 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b01111110},
                /* 01111111 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b01111111},
                /* 10000000 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b00000001},
                /* 10000001 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b00011000},
                /* 10000010 */ {{3, 1, 7, 5, 2, 0, 6, 4}, 0, 0b00000110},
                /* 10000011 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b00011001},
                /* 10000100 */ {{3, 2, 7, 6, 1, 0, 5, 4}, 1, 0b00000110},
                /* 10000101 */ {{2, 3, 6, 7, 0, 1, 4, 5}, 0, 0b00011001},
                /* 10000110 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b00010110},
                /* 10000111 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b00011110},
                /* 10001000 */ {{3, 7, 1, 5, 2, 6, 0, 4}, 1, 0b00000011},
                /* 10001001 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b00011001},
                /* 10001010 */ {{3, 1, 7, 5, 2, 0, 6, 4}, 0, 0b00000111},
                /* 10001011 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b00011011},
                /* 10001100 */ {{3, 2, 7, 6, 1, 0, 5, 4}, 1, 0b00000111},
                /* 10001101 */ {{2, 3, 6, 7, 0, 1, 4, 5}, 0, 0b00011011},
                /* 10001110 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b00010111},
                /* 10001111 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b00011111},
                /* 10010000 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00000110},
                /* 10010001 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00011001},
                /* 10010010 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00010110},
                /* 10010011 */ {{5, 4, 1, 0, 7, 6, 3, 2}, 1, 0b00011110},
                /* 10010100 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00010110},
                /* 10010101 */ {{6, 4, 2, 0, 7, 5, 3, 1}, 0, 0b00011110},
                /* 10010110 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b01101001},
                /* 10010111 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b01101011},
                /* 10011000 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b00011001},
                /* 10011001 */ {{1, 5, 3, 7, 0, 4, 2, 6}, 0, 0b00111100},
                /* 10011010 */ {{5, 7, 1, 3, 4, 6, 0, 2}, 0, 0b00011110},
                /* 10011011 */ {{1, 5, 3, 7, 0, 4, 2, 6}, 0, 0b00111101},
                /* 10011100 */ {{6, 7, 2, 3, 4, 5, 0, 1}, 1, 0b00011110},
                /* 10011101 */ {{2, 6, 0, 4, 3, 7, 1, 5}, 0, 0b00111101},
                /* 10011110 */ {{1, 3, 0, 2, 5, 7, 4, 6}, 0, 0b01101011},
                /* 10011111 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b01101111},
                /* 10100000 */ {{5, 7, 4, 6, 1, 3, 0, 2}, 1, 0b00000011},
                /* 10100001 */ {{5, 4, 1, 0, 7, 6, 3, 2}, 1, 0b00011001},
                /* 10100010 */ {{5, 7, 1, 3, 4, 6, 0, 2}, 0, 0b00000111},
                /* 10100011 */ {{1, 5, 3, 7, 0, 4, 2, 6}, 0, 0b00011011},
                /* 10100100 */ {{7, 6, 3, 2, 5, 4, 1, 0}, 0, 0b00011001},
                /* 10100101 */ {{1, 3, 0, 2, 5, 7, 4, 6}, 0, 0b00111100},
                /* 10100110 */ {{3, 1, 7, 5, 2, 0, 6, 4}, 0, 0b00011110},
                /* 10100111 */ {{1, 3, 0, 2, 5, 7, 4, 6}, 0, 0b00111101},
                /* 10101000 */ {{7, 5, 3, 1, 6, 4, 2, 0}, 1, 0b00000111},
                /* 10101001 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b00011110},
                /* 10101010 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b00001111},
                /* 10101011 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b00011111},
                /* 10101100 */ {{3, 7, 1, 5, 2, 6, 0, 4}, 1, 0b00011011},
                /* 10101101 */ {{3, 1, 2, 0, 7, 5, 6, 4}, 1, 0b00111101},
                /* 10101110 */ {{3, 1, 7, 5, 2, 0, 6, 4}, 0, 0b00011111},
                /* 10101111 */ {{1, 3, 0, 2, 5, 7, 4, 6}, 0, 0b00111111},
                /* 10110000 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00000111},
                /* 10110001 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00011011},
                /* 10110010 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00010111},
                /* 10110011 */ {{5, 4, 1, 0, 7, 6, 3, 2}, 1, 0b00011111},
                /* 10110100 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00011110},
                /* 10110101 */ {{4, 6, 5, 7, 0, 2, 1, 3}, 0, 0b00111101},
                /* 10110110 */ {{1, 5, 3, 7, 0, 4, 2, 6}, 0, 0b01101011},
                /* 10110111 */ {{1, 0, 5, 4, 3, 2, 7, 6}, 0, 0b01101111},
                /* 10111000 */ {{5, 7, 1, 3, 4, 6, 0, 2}, 0, 0b00011011},
                /* 10111001 */ {{5, 1, 7, 3, 4, 0, 6, 2}, 1, 0b00111101},
                /* 10111010 */ {{5, 7, 1, 3, 4, 6, 0, 2}, 0, 0b00011111},
                /* 10111011 */ {{1, 5, 3, 7, 0, 4, 2, 6}, 0, 0b00111111},
                /* 10111100 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b00111101},
                /* 10111101 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b01111110},
                /* 10111110 */ {{1, 3, 5, 7, 0, 2, 4, 6}, 1, 0b01101111},
                /* 10111111 */ {{1, 0, 3, 2, 5, 4, 7, 6}, 1, 0b01111111},
                /* 11000000 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00000011},
                /* 11000001 */ {{6, 4, 2, 0, 7, 5, 3, 1}, 0, 0b00011001},
                /* 11000010 */ {{7, 5, 3, 1, 6, 4, 2, 0}, 1, 0b00011001},
                /* 11000011 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00111100},
                /* 11000100 */ {{6, 7, 2, 3, 4, 5, 0, 1}, 1, 0b00000111},
                /* 11000101 */ {{2, 6, 3, 7, 0, 4, 1, 5}, 1, 0b00011011},
                /* 11000110 */ {{3, 2, 7, 6, 1, 0, 5, 4}, 1, 0b00011110},
                /* 11000111 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00111101},
                /* 11001000 */ {{7, 6, 3, 2, 5, 4, 1, 0}, 0, 0b00000111},
                /* 11001001 */ {{2, 3, 6, 7, 0, 1, 4, 5}, 0, 0b00011110},
                /* 11001010 */ {{3, 7, 2, 6, 1, 5, 0, 4}, 0, 0b00011011},
                /* 11001011 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b00111101},
                /* 11001100 */ {{2, 3, 6, 7, 0, 1, 4, 5}, 0, 0b00001111},
                /* 11001101 */ {{2, 3, 6, 7, 0, 1, 4, 5}, 0, 0b00011111},
                /* 11001110 */ {{3, 2, 7, 6, 1, 0, 5, 4}, 1, 0b00011111},
                /* 11001111 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b00111111},
                /* 11010000 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00000111},
                /* 11010001 */ {{4, 6, 5, 7, 0, 2, 1, 3}, 0, 0b00011011},
                /* 11010010 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00011110},
                /* 11010011 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00111101},
                /* 11010100 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00010111},
                /* 11010101 */ {{6, 4, 2, 0, 7, 5, 3, 1}, 0, 0b00011111},
                /* 11010110 */ {{2, 6, 0, 4, 3, 7, 1, 5}, 0, 0b01101011},
                /* 11010111 */ {{2, 0, 6, 4, 3, 1, 7, 5}, 1, 0b01101111},
                /* 11011000 */ {{6, 7, 2, 3, 4, 5, 0, 1}, 1, 0b00011011},
                /* 11011001 */ {{6, 2, 4, 0, 7, 3, 5, 1}, 1, 0b00111101},
                /* 11011010 */ {{7, 5, 6, 4, 3, 1, 2, 0}, 0, 0b00111101},
                /* 11011011 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b01111110},
                /* 11011100 */ {{6, 7, 2, 3, 4, 5, 0, 1}, 1, 0b00011111},
                /* 11011101 */ {{2, 6, 0, 4, 3, 7, 1, 5}, 0, 0b00111111},
                /* 11011110 */ {{2, 3, 6, 7, 0, 1, 4, 5}, 0, 0b01101111},
                /* 11011111 */ {{2, 3, 0, 1, 6, 7, 4, 5}, 1, 0b01111111},
                /* 11100000 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b00000111},
                /* 11100001 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00011110},
                /* 11100010 */ {{5, 7, 4, 6, 1, 3, 0, 2}, 1, 0b00011011},
                /* 11100011 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00111101},
                /* 11100100 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00011011},
                /* 11100101 */ {{6, 4, 7, 5, 2, 0, 3, 1}, 1, 0b00111101},
                /* 11100110 */ {{7, 3, 5, 1, 6, 2, 4, 0}, 0, 0b00111101},
                /* 11100111 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b01111110},
                /* 11101000 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b00010111},
                /* 11101001 */ {{3, 7, 1, 5, 2, 6, 0, 4}, 1, 0b01101011},
                /* 11101010 */ {{7, 5, 3, 1, 6, 4, 2, 0}, 1, 0b00011111},
                /* 11101011 */ {{3, 1, 7, 5, 2, 0, 6, 4}, 0, 0b01101111},
                /* 11101100 */ {{7, 6, 3, 2, 5, 4, 1, 0}, 0, 0b00011111},
                /* 11101101 */ {{3, 2, 7, 6, 1, 0, 5, 4}, 1, 0b01101111},
                /* 11101110 */ {{3, 7, 1, 5, 2, 6, 0, 4}, 1, 0b00111111},
                /* 11101111 */ {{3, 2, 1, 0, 7, 6, 5, 4}, 0, 0b01111111},
                /* 11110000 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00001111},
                /* 11110001 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00011111},
                /* 11110010 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b00011111},
                /* 11110011 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b00111111},
                /* 11110100 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00011111},
                /* 11110101 */ {{4, 6, 5, 7, 0, 2, 1, 3}, 0, 0b00111111},
                /* 11110110 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b01101111},
                /* 11110111 */ {{4, 5, 6, 7, 0, 1, 2, 3}, 1, 0b01111111},
                /* 11111000 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b00011111},
                /* 11111001 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b01101111},
                /* 11111010 */ {{5, 7, 4, 6, 1, 3, 0, 2}, 1, 0b00111111},
                /* 11111011 */ {{5, 4, 7, 6, 1, 0, 3, 2}, 0, 0b01111111},
                /* 11111100 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b00111111},
                /* 11111101 */ {{6, 7, 4, 5, 2, 3, 0, 1}, 0, 0b01111111},
                /* 11111110 */ {{7, 6, 5, 4, 3, 2, 1, 0}, 1, 0b01111111},
                /* 11111111 */ {{0, 1, 2, 3, 4, 5, 6, 7}, 0, 0b11111111},
            };

        int index = 0;

        for (int i = 0; i < 8; ++i)
            if (vertices[i]->offset() < 0)
                index += 1 << i;

        const auto &proc = table[index];

        if (proc.code == 0b00000000 || proc.code == 0b11111111)
            return;

        std::array<const FieldVertex *, 8> trans_vertices;

        for (int i = 0; i < 8; i++)
            trans_vertices[i] = vertices[proc.trans[i]];

        std::array<Vec3Df, 12> points;

        auto calculate_point = [&](int edge_index, int i, int j)
        {
            double v1 = trans_vertices[i]->offset();
            double v2 = trans_vertices[j]->offset();

            if (v1 * v2 < 0)
            {
                points[edge_index] = trans_vertices[i]->position().mix(
                    trans_vertices[j]->position(),
                    v1 / (v2 - v1));
            }
        };

        calculate_point(0, 0, 1);
        calculate_point(1, 2, 3);
        calculate_point(2, 4, 5);
        calculate_point(3, 6, 7);
        calculate_point(4, 0, 2);
        calculate_point(5, 1, 3);
        calculate_point(6, 4, 6);
        calculate_point(7, 5, 7);
        calculate_point(8, 0, 4);
        calculate_point(9, 1, 5);
        calculate_point(10, 2, 6);
        calculate_point(11, 3, 7);

        auto append = [&](int index1, int index2, int index3)
        {
            const auto &p1 = points[index1];
            const auto &p2 = points[index2];
            const auto &p3 = points[index3];

            if (proc.flip)
            {
                triangles.emplace_back(p1, p2, p3);
            }
            else
            {
                triangles.emplace_back(p3, p2, p1);
            }
        };

        switch (proc.code)
        {
        case 0b00000001:
            append(0, 4, 8);
            break;
        case 0b00000011:
            append(4, 8, 9);
            append(5, 4, 9);
            break;
        case 0b00000110:
            append(0, 9, 4);
            append(4, 9, 10);
            append(10, 9, 5);
            append(10, 5, 1);
            break;
        case 0b00000111:
            append(8, 9, 10);
            append(1, 10, 9);
            append(5, 1, 9);
            break;
        case 0b00001111:
            append(8, 9, 10);
            append(9, 11, 10);
            break;
        case 0b00010110:
            append(0, 8, 4);
            append(1, 10, 5);
            append(5, 10, 9);
            append(9, 10, 2);
            append(2, 10, 6);
            break;
        case 0b00010111:
            append(1, 10, 5);
            append(5, 10, 9);
            append(2, 9, 10);
            append(2, 10, 6);
            break;
        case 0b00011000:
            append(1, 5, 11);
            append(2, 8, 6);
            break;
        case 0b00011001:
            append(1, 4, 6);
            append(1, 6, 11);
            append(11, 6, 2);
            append(11, 2, 5);
            append(5, 2, 0);
            break;
        case 0b00011011:
            append(2, 9, 6);
            append(6, 9, 11);
            append(6, 11, 1);
            append(1, 4, 6);
            break;
        case 0b00011110:
            append(0, 8, 4);
            append(2, 9, 6);
            append(6, 9, 10);
            append(10, 9, 11);
            break;
        case 0b00011111:
            append(2, 9, 6);
            append(6, 9, 10);
            append(10, 9, 11);
            break;
        case 0b00111100:
            append(4, 5, 8);
            append(8, 5, 9);
            append(10, 6, 11);
            append(11, 6, 7);
            break;
        case 0b00111101:
            append(6, 7, 10);
            append(10, 7, 11);
            append(0, 5, 9);
            break;
        case 0b00111111:
            append(6, 7, 10);
            append(10, 7, 11);
            break;
        case 0b01101001:
            append(0, 5, 9);
            append(1, 4, 10);
            append(2, 6, 8);
            append(3, 7, 11);
            break;
        case 0b01101011:
            append(1, 4, 8);
            append(1, 8, 11);
            append(11, 8, 7);
            append(7, 8, 2);
            append(3, 6, 10);
            break;
        case 0b01101111:
            append(2, 6, 8);
            append(3, 7, 11);
            break;
        case 0b01111110:
            append(0, 8, 4);
            append(3, 7, 11);
            break;
        case 0b01111111:
            append(3, 7, 11);
            break;
        }
    }
} // namespace CGCP
