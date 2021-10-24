#pragma once

#include "DMCPolygonizer/TreeNode.h"
#include "Polygonizer.h"

namespace CGCP
{
    class DMCPolygonizer : public Polygonizer
    {
    private:
        std::shared_ptr<TreeNode> generateTree(const Vec3Df &from, const Vec3Df &to, int depth = 0);

        double nominal_weight_;
        double tolerance_;
        int max_depth_;

    protected:
        using typename Polygonizer::ProgressCallback;

        virtual void threadRun(ProgressCallback progress) override;
        virtual void validate(const Config &config) override;

    public:
        static const char *GRID_DIM_X;
        static const char *GRID_DIM_Y;
        static const char *GRID_DIM_Z;
        static const char *GRID_SIZE_X;
        static const char *GRID_SIZE_Y;
        static const char *GRID_SIZE_Z;
        static const char *MAX_DEPTH;
        static const char *NOMINAL_WEIGHT;
        static const char *TOLERANCE;

        DMCPolygonizer() : Polygonizer({
                               {GRID_DIM_X, "5"},
                               {GRID_DIM_Y, "5"},
                               {GRID_DIM_Z, "5"},
                               {GRID_SIZE_X, "10"},
                               {GRID_SIZE_Y, "10"},
                               {GRID_SIZE_Z, "10"},
                               {MAX_DEPTH, "4"},
                               {NOMINAL_WEIGHT, "0.1"},
                               {TOLERANCE, "0.001"},
                           }){};

        virtual ~DMCPolygonizer() override = default;
    };
} // namespace CGCP
