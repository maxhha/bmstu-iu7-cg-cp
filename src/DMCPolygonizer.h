#pragma once

#include "DMCPolygonizer/ObjectPool.h"
#include "DMCPolygonizer/TreeNode.h"
#include "Polygonizer.h"

namespace CGCP
{
    class DMCPolygonizer : public Polygonizer
    {
    private:
        using typename Polygonizer::Config;
        using typename Polygonizer::ProgressCallback;
        using Triangles = std::vector<Triangle3Df>;
        using TreeNodePtr = std::shared_ptr<TreeNode>;
        using TrianglesPtr = std::shared_ptr<Triangles>;

        void setConfig(const Config &config);

        void increaseProgress();

        std::vector<TreeNodePtr> generateForest();
        std::shared_ptr<TreeNode> generateTree(
            ObjectPool<LeafTreeNode> &leafs,
            ObjectPool<BranchTreeNode> &branches,
            const Vec3Df &from,
            const Vec3Df &to,
            int depth = 0);

        TrianglesPtr enumerateTriangles(std::vector<TreeNodePtr> &children);

        void enumerateCell(Triangles &triangles,
                           const TreeNodePtr n);

        void enumerateFaceX(Triangles &triangles,
                            const TreeNodePtr n1, const TreeNodePtr n2);
        void enumerateFaceY(Triangles &triangles,
                            const TreeNodePtr n1, const TreeNodePtr n2);
        void enumerateFaceZ(Triangles &triangles,
                            const TreeNodePtr n1, const TreeNodePtr n2);

        void enumerateEdgeXY(Triangles &triangles,
                             const TreeNodePtr n1, const TreeNodePtr n2,
                             const TreeNodePtr n3, const TreeNodePtr n4);

        void enumerateEdgeYZ(Triangles &triangles,
                             const TreeNodePtr n1, const TreeNodePtr n2,
                             const TreeNodePtr n3, const TreeNodePtr n4);

        void enumerateEdgeXZ(Triangles &triangles,
                             const TreeNodePtr n1, const TreeNodePtr n2,
                             const TreeNodePtr n3, const TreeNodePtr n4);

        void enumerateVertex(Triangles &triangles,
                             const TreeNodePtr n1, const TreeNodePtr n2,
                             const TreeNodePtr n3, const TreeNodePtr n4,
                             const TreeNodePtr n5, const TreeNodePtr n6,
                             const TreeNodePtr n7, const TreeNodePtr n8);

        void marchingCube(
            Triangles &triangles,
            const std::array<const FieldVertex *, 8> &verticies);

        Vec3Ds dim_;
        double nominal_weight_;
        double tolerance_;
        int max_depth_;
        ProgressCallback progress_receiver_;
        std::size_t progress_;
        std::size_t total_progress_;

    protected:
        virtual void
        threadRun(ProgressCallback progress) override;
        virtual Polygonizer &config(const Config &config) override;

    public:
        static const char *GRID_DIM_X;
        static const char *GRID_DIM_Y;
        static const char *GRID_DIM_Z;
        static const char *MAX_DEPTH;
        static const char *NOMINAL_WEIGHT;
        static const char *TOLERANCE;

        DMCPolygonizer();

        virtual ~DMCPolygonizer() override = default;
    };
} // namespace CGCP
