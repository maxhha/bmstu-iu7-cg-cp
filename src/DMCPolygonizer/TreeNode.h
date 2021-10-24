#pragma once

#include "../Vec3D.h"
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
        using Children = std::array<std::shared_ptr<TreeNode>, 8>;
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

        virtual ~BranchTreeNode() override = default;
    };

    class LeafTreeNode : public TreeNode
    {
    private:
        Vec3Df center_;
        Vec3Df offset_;

    public:
        LeafTreeNode(const Vec3Df &center, const Vec3Df &offset) : center_(center), offset_(offset){};

        LeafTreeNode &operator=(const LeafTreeNode &other)
        {
            center_ = other.center_;
            offset_ = other.offset_;

            return *this;
        };

        Vec3Df &center() { return center_; };
        const Vec3Df &center() const { return center_; };

        Vec3Df &offset() { return offset_; };
        const Vec3Df &offset() const { return offset_; };
    };
} // namespace CGCP
