#pragma once

#include "DMCPolygonizer/TreeNode.h"
#include "Polygonizer.h"

namespace CGCP
{
    class DMCPolygonizer : public Polygonizer
    {
    private:
        using Triangles = std::vector<Triangle3Df>;
        using TreeNodePtr = std::shared_ptr<TreeNode>;
        std::shared_ptr<TreeNode> generateTree(const Vec3Df &from, const Vec3Df &to, int depth = 0);

        // void enumerateCell(Triangles &triangles,
        //                    const TreeNodePtr n);

        // void enumerateFaceX(Triangles &triangles,
        //                     const TreeNodePtr n1, const TreeNodePtr n2);
        // void enumerateFaceY(Triangles &triangles,
        //                     const TreeNodePtr n1, const TreeNodePtr n2);
        // void enumerateFaceZ(Triangles &triangles,
        //                     const TreeNodePtr n1, const TreeNodePtr n2);

        // void enumerateEdgeXY(Triangles &triangles,
        //                      const TreeNodePtr n1, const TreeNodePtr n2,
        //                      const TreeNodePtr n3, const TreeNodePtr n4);
        // void enumerateEdgeYZ(Triangles &triangles,
        //                      const TreeNodePtr n1, const TreeNodePtr n2,
        //                      const TreeNodePtr n3, const TreeNodePtr n4);
        // void enumerateEdgeXZ(Triangles &triangles,
        //                      const TreeNodePtr n1, const TreeNodePtr n2,
        //                      const TreeNodePtr n3, const TreeNodePtr n4);

        void enumerateVertex(Triangles &triangles,
                             const TreeNodePtr n1, const TreeNodePtr n2,
                             const TreeNodePtr n3, const TreeNodePtr n4,
                             const TreeNodePtr n5, const TreeNodePtr n6,
                             const TreeNodePtr n7, const TreeNodePtr n8);

        void marchingCube(
            Triangles &triangles,
            const std::array<const FieldVertex *, 8> &verticies);

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
