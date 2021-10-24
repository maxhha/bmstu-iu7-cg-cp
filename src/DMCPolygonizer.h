#pragma once

#include "DMCPolygonizer/TreeNode.h"
#include "Polygonizer.h"

namespace CGCP
{
    class DMCPolygonizer : public Polygonizer
    {
    private:
        std::shared_ptr<TreeNode> generateTree(const Vec3Df &from, const Vec3Df &to, std::size_t depth = 0);

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

        DMCPolygonizer() : Polygonizer({
                               {GRID_DIM_X, "20"},
                               {GRID_DIM_Y, "20"},
                               {GRID_DIM_Z, "20"},
                               {GRID_SIZE_X, "10"},
                               {GRID_SIZE_Y, "10"},
                               {GRID_SIZE_Z, "10"},
                               {MAX_DEPTH, "10"},
                           }){};

        virtual ~DMCPolygonizer() override = default;
    };
} // namespace CGCP
