#pragma once

#include "../Vec3D.h"
#include "FieldVertex.h"
#include <array>
#include <memory>

namespace CGCP
{
    class TreeNode
    {
    public:
        TreeNode() = default;
        virtual ~TreeNode() = default;
    };

    class BranchTreeNode : public TreeNode
    {
    private:
        using TreeNodePtr = std::shared_ptr<TreeNode>;
        using Children = std::array<TreeNodePtr, 8>;
        Children children_;

    public:
        explicit BranchTreeNode(const Children &children) : children_(children){};

        BranchTreeNode &operator=(const BranchTreeNode &other)
        {
            children_ = other.children_;
            return *this;
        };

        Children &children() { return children_; };
        const Children &children() const { return children_; };

        TreeNodePtr xyz() { return children_[0]; };
        TreeNodePtr Xyz() { return children_[1]; };
        TreeNodePtr xYz() { return children_[2]; };
        TreeNodePtr XYz() { return children_[3]; };
        TreeNodePtr xyZ() { return children_[4]; };
        TreeNodePtr XyZ() { return children_[5]; };
        TreeNodePtr xYZ() { return children_[6]; };
        TreeNodePtr XYZ() { return children_[7]; };

        virtual ~BranchTreeNode() override = default;
    };

    class LeafTreeNode : public TreeNode
    {
    private:
        FieldVertex vertex_;

    public:
        explicit LeafTreeNode(const FieldVertex &vertex) : vertex_(vertex){};

        FieldVertex &vertex() { return vertex_; };
        const FieldVertex &vertex() const { return vertex_; };

        LeafTreeNode &operator=(const LeafTreeNode &other)
        {
            vertex_ = other.vertex_;

            return *this;
        };

        virtual ~LeafTreeNode() override = default;
    };
} // namespace CGCP
