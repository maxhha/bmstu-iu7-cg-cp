#include "../DMCPolygonizer.h"

namespace CGCP
{
    void DMCPolygonizer::enumerateCell(
        Triangles &triangles,
        const TreeNodePtr n)
    {
        if (auto b = std::dynamic_pointer_cast<BranchTreeNode>(n))
        {
            enumerateCell(triangles, b->xyz());
            enumerateCell(triangles, b->Xyz());
            enumerateCell(triangles, b->xYz());
            enumerateCell(triangles, b->XYz());
            enumerateCell(triangles, b->xyZ());
            enumerateCell(triangles, b->XyZ());
            enumerateCell(triangles, b->xYZ());
            enumerateCell(triangles, b->XYZ());

            enumerateFaceX(triangles, b->xyz(), b->Xyz());
            enumerateFaceX(triangles, b->xYz(), b->XYz());
            enumerateFaceX(triangles, b->xyZ(), b->XyZ());
            enumerateFaceX(triangles, b->xYZ(), b->XYZ());

            enumerateFaceY(triangles, b->xyz(), b->xYz());
            enumerateFaceY(triangles, b->Xyz(), b->XYz());
            enumerateFaceY(triangles, b->xyZ(), b->xYZ());
            enumerateFaceY(triangles, b->XyZ(), b->XYZ());

            enumerateFaceZ(triangles, b->xyz(), b->xyZ());
            enumerateFaceZ(triangles, b->Xyz(), b->XyZ());
            enumerateFaceZ(triangles, b->xYz(), b->xYZ());
            enumerateFaceZ(triangles, b->XYz(), b->XYZ());

            enumerateEdgeXY(triangles, b->xyz(), b->Xyz(), b->xYz(), b->XYz());
            enumerateEdgeXY(triangles, b->xyZ(), b->XyZ(), b->xYZ(), b->XYZ());

            enumerateEdgeYZ(triangles, b->xyz(), b->xYz(), b->xyZ(), b->xYZ());
            enumerateEdgeYZ(triangles, b->Xyz(), b->XYz(), b->XyZ(), b->XYZ());

            enumerateEdgeXZ(triangles, b->xyz(), b->Xyz(), b->xyZ(), b->XyZ());
            enumerateEdgeXZ(triangles, b->xYz(), b->XYz(), b->xYZ(), b->XYZ());

            enumerateVertex(
                triangles,
                b->xyz(),
                b->Xyz(),
                b->xYz(),
                b->XYz(),
                b->xyZ(),
                b->XyZ(),
                b->xYZ(),
                b->XYZ());
        }
    }

    void DMCPolygonizer::enumerateFaceX(
        Triangles &triangles,
        const TreeNodePtr n_x, const TreeNodePtr n_X)
    {
        auto b_x = std::dynamic_pointer_cast<BranchTreeNode>(n_x);
        auto b_X = std::dynamic_pointer_cast<BranchTreeNode>(n_X);

        if (b_x || b_X)
        {
            enumerateFaceX(
                triangles,
                b_x ? b_x->Xyz() : n_x, b_X ? b_X->xyz() : n_X);
            enumerateFaceX(
                triangles,
                b_x ? b_x->XYz() : n_x, b_X ? b_X->xYz() : n_X);
            enumerateFaceX(
                triangles,
                b_x ? b_x->XyZ() : n_x, b_X ? b_X->xyZ() : n_X);
            enumerateFaceX(
                triangles,
                b_x ? b_x->XYZ() : n_x, b_X ? b_X->xYZ() : n_X);

            enumerateEdgeXY(
                triangles,

                b_x ? b_x->Xyz() : n_x,
                b_X ? b_X->xyz() : n_X,
                b_x ? b_x->XYz() : n_x,
                b_X ? b_X->xYz() : n_X);

            enumerateEdgeXY(
                triangles,

                b_x ? b_x->XyZ() : n_x,
                b_X ? b_X->xyZ() : n_X,
                b_x ? b_x->XYZ() : n_x,
                b_X ? b_X->xYZ() : n_X);

            enumerateEdgeXZ(
                triangles,

                b_x ? b_x->Xyz() : n_x,
                b_X ? b_X->xyz() : n_X,
                b_x ? b_x->XyZ() : n_x,
                b_X ? b_X->xyZ() : n_X);

            enumerateEdgeXZ(
                triangles,

                b_x ? b_x->XYz() : n_x,
                b_X ? b_X->xYz() : n_X,
                b_x ? b_x->XYZ() : n_x,
                b_X ? b_X->xYZ() : n_X);

            enumerateVertex(
                triangles,
                b_x ? b_x->Xyz() : n_x,
                b_X ? b_X->xyz() : n_X,
                b_x ? b_x->XYz() : n_x,
                b_X ? b_X->xYz() : n_X,
                b_x ? b_x->XyZ() : n_x,
                b_X ? b_X->xyZ() : n_X,
                b_x ? b_x->XYZ() : n_x,
                b_X ? b_X->xYZ() : n_X);
        }
    }

    void DMCPolygonizer::enumerateFaceY(
        Triangles &triangles,
        const TreeNodePtr n_y, const TreeNodePtr n_Y)
    {
        auto b_y = std::dynamic_pointer_cast<BranchTreeNode>(n_y);
        auto b_Y = std::dynamic_pointer_cast<BranchTreeNode>(n_Y);

        if (b_y || b_Y)
        {
            enumerateFaceY(
                triangles,
                b_y ? b_y->XYz() : n_y, b_Y ? b_Y->Xyz() : n_Y);
            enumerateFaceY(
                triangles,
                b_y ? b_y->xYZ() : n_y, b_Y ? b_Y->xyZ() : n_Y);
            enumerateFaceY(
                triangles,
                b_y ? b_y->XYZ() : n_y, b_Y ? b_Y->XyZ() : n_Y);
            enumerateFaceY(
                triangles,
                b_y ? b_y->xYz() : n_y, b_Y ? b_Y->xyz() : n_Y);

            enumerateEdgeXY(
                triangles,

                b_y ? b_y->xYz() : n_y,
                b_y ? b_y->XYz() : n_y,
                b_Y ? b_Y->xyz() : n_Y,
                b_Y ? b_Y->Xyz() : n_Y);

            enumerateEdgeXY(
                triangles,

                b_y ? b_y->xYZ() : n_y,
                b_y ? b_y->XYZ() : n_y,
                b_Y ? b_Y->xyZ() : n_Y,
                b_Y ? b_Y->XyZ() : n_Y);

            enumerateEdgeYZ(
                triangles,

                b_y ? b_y->XYz() : n_y,
                b_Y ? b_Y->Xyz() : n_Y,
                b_y ? b_y->XYZ() : n_y,
                b_Y ? b_Y->XyZ() : n_Y);

            enumerateEdgeYZ(
                triangles,

                b_y ? b_y->xYz() : n_y,
                b_Y ? b_Y->xyz() : n_Y,
                b_y ? b_y->xYZ() : n_y,
                b_Y ? b_Y->xyZ() : n_Y);

            enumerateVertex(
                triangles,
                b_y ? b_y->xYz() : n_y,
                b_y ? b_y->XYz() : n_y,
                b_Y ? b_Y->xyz() : n_Y,
                b_Y ? b_Y->Xyz() : n_Y,
                b_y ? b_y->xYZ() : n_y,
                b_y ? b_y->XYZ() : n_y,
                b_Y ? b_Y->xyZ() : n_Y,
                b_Y ? b_Y->XyZ() : n_Y);
        }
    }

    void DMCPolygonizer::enumerateFaceZ(
        Triangles &triangles,
        const TreeNodePtr n_z, const TreeNodePtr n_Z)
    {
        auto b_z = std::dynamic_pointer_cast<BranchTreeNode>(n_z);
        auto b_Z = std::dynamic_pointer_cast<BranchTreeNode>(n_Z);

        if (b_z || b_Z)
        {
            enumerateFaceZ(
                triangles,
                b_z ? b_z->xyZ() : n_z,
                b_Z ? b_Z->xyz() : n_Z);
            enumerateFaceZ(
                triangles,
                b_z ? b_z->XyZ() : n_z,
                b_Z ? b_Z->Xyz() : n_Z);
            enumerateFaceZ(
                triangles,
                b_z ? b_z->xYZ() : n_z,
                b_Z ? b_Z->xYz() : n_Z);
            enumerateFaceZ(
                triangles,
                b_z ? b_z->XYZ() : n_z,
                b_Z ? b_Z->XYz() : n_Z);

            enumerateEdgeXZ(
                triangles,
                b_z ? b_z->xyZ() : n_z,
                b_z ? b_z->XyZ() : n_z,
                b_Z ? b_Z->xyz() : n_Z,
                b_Z ? b_Z->Xyz() : n_Z);

            enumerateEdgeXZ(
                triangles,
                b_z ? b_z->xYZ() : n_z,
                b_z ? b_z->XYZ() : n_z,
                b_Z ? b_Z->xYz() : n_Z,
                b_Z ? b_Z->XYz() : n_Z);

            enumerateEdgeYZ(
                triangles,
                b_z ? b_z->xyZ() : n_z,
                b_z ? b_z->xYZ() : n_z,
                b_Z ? b_Z->xyz() : n_Z,
                b_Z ? b_Z->xYz() : n_Z);

            enumerateEdgeYZ(
                triangles,
                b_z ? b_z->XyZ() : n_z,
                b_z ? b_z->XYZ() : n_z,
                b_Z ? b_Z->Xyz() : n_Z,
                b_Z ? b_Z->XYz() : n_Z);

            enumerateVertex(
                triangles,
                b_z ? b_z->xyZ() : n_z,
                b_z ? b_z->XyZ() : n_z,
                b_z ? b_z->xYZ() : n_z,
                b_z ? b_z->XYZ() : n_z,
                b_Z ? b_Z->xyz() : n_Z,
                b_Z ? b_Z->Xyz() : n_Z,
                b_Z ? b_Z->xYz() : n_Z,
                b_Z ? b_Z->XYz() : n_Z);
        }
    }

    void DMCPolygonizer::enumerateEdgeXY(
        Triangles &triangles,
        const TreeNodePtr n_xy, const TreeNodePtr n_Xy,
        const TreeNodePtr n_xY, const TreeNodePtr n_XY)
    {
        auto b_xy = std::dynamic_pointer_cast<BranchTreeNode>(n_xy);
        auto b_Xy = std::dynamic_pointer_cast<BranchTreeNode>(n_Xy);
        auto b_xY = std::dynamic_pointer_cast<BranchTreeNode>(n_xY);
        auto b_XY = std::dynamic_pointer_cast<BranchTreeNode>(n_XY);

        if (b_xy || b_Xy || b_xY || b_XY)
        {
            enumerateEdgeXY(
                triangles,
                b_xy ? b_xy->XYz() : n_xy,
                b_Xy ? b_Xy->xYz() : n_Xy,
                b_xY ? b_xY->Xyz() : n_xY,
                b_XY ? b_XY->xyz() : n_XY);

            enumerateEdgeXY(
                triangles,
                b_xy ? b_xy->XYZ() : n_xy,
                b_Xy ? b_Xy->xYZ() : n_Xy,
                b_xY ? b_xY->XyZ() : n_xY,
                b_XY ? b_XY->xyZ() : n_XY);

            enumerateVertex(
                triangles,
                b_xy ? b_xy->XYz() : n_xy,
                b_Xy ? b_Xy->xYz() : n_Xy,
                b_xY ? b_xY->Xyz() : n_xY,
                b_XY ? b_XY->xyz() : n_XY,
                b_xy ? b_xy->XYZ() : n_xy,
                b_Xy ? b_Xy->xYZ() : n_Xy,
                b_xY ? b_xY->XyZ() : n_xY,
                b_XY ? b_XY->xyZ() : n_XY);
        }
    }

    void DMCPolygonizer::enumerateEdgeXZ(
        Triangles &triangles,
        const TreeNodePtr n_xz, const TreeNodePtr n_Xz,
        const TreeNodePtr n_xZ, const TreeNodePtr n_XZ)
    {
        auto b_xz = std::dynamic_pointer_cast<BranchTreeNode>(n_xz);
        auto b_Xz = std::dynamic_pointer_cast<BranchTreeNode>(n_Xz);
        auto b_xZ = std::dynamic_pointer_cast<BranchTreeNode>(n_xZ);
        auto b_XZ = std::dynamic_pointer_cast<BranchTreeNode>(n_XZ);

        if (b_xz || b_Xz || b_xZ || b_XZ)
        {
            enumerateEdgeXZ(
                triangles,
                b_xz ? b_xz->XyZ() : n_xz,
                b_Xz ? b_Xz->xyZ() : n_Xz,
                b_xZ ? b_xZ->Xyz() : n_xZ,
                b_XZ ? b_XZ->xyz() : n_XZ);

            enumerateEdgeXZ(
                triangles,
                b_xz ? b_xz->XYZ() : n_xz,
                b_Xz ? b_Xz->xYZ() : n_Xz,
                b_xZ ? b_xZ->XYz() : n_xZ,
                b_XZ ? b_XZ->xYz() : n_XZ);

            enumerateVertex(
                triangles,
                b_xz ? b_xz->XyZ() : n_xz,
                b_Xz ? b_Xz->xyZ() : n_Xz,
                b_xz ? b_xz->XYZ() : n_xz,
                b_Xz ? b_Xz->xYZ() : n_Xz,
                b_xZ ? b_xZ->Xyz() : n_xZ,
                b_XZ ? b_XZ->xyz() : n_XZ,
                b_xZ ? b_xZ->XYz() : n_xZ,
                b_XZ ? b_XZ->xYz() : n_XZ);
        }
    }

    void DMCPolygonizer::enumerateEdgeYZ(
        Triangles &triangles,
        const TreeNodePtr n_yz, const TreeNodePtr n_Yz,
        const TreeNodePtr n_yZ, const TreeNodePtr n_YZ)
    {
        auto b_yz = std::dynamic_pointer_cast<BranchTreeNode>(n_yz);
        auto b_Yz = std::dynamic_pointer_cast<BranchTreeNode>(n_Yz);
        auto b_yZ = std::dynamic_pointer_cast<BranchTreeNode>(n_yZ);
        auto b_YZ = std::dynamic_pointer_cast<BranchTreeNode>(n_YZ);

        if (b_yz || b_Yz || b_yZ || b_YZ)
        {
            enumerateEdgeYZ(
                triangles,
                b_yz ? b_yz->xYZ() : n_yz,
                b_Yz ? b_Yz->xyZ() : n_Yz,
                b_yZ ? b_yZ->xYz() : n_yZ,
                b_YZ ? b_YZ->xyz() : n_YZ);

            enumerateEdgeYZ(
                triangles,
                b_yz ? b_yz->XYZ() : n_yz,
                b_Yz ? b_Yz->XyZ() : n_Yz,
                b_yZ ? b_yZ->XYz() : n_yZ,
                b_YZ ? b_YZ->Xyz() : n_YZ);

            enumerateVertex(
                triangles,
                b_yz ? b_yz->xYZ() : n_yz,
                b_yz ? b_yz->XYZ() : n_yz,
                b_Yz ? b_Yz->xyZ() : n_Yz,
                b_Yz ? b_Yz->XyZ() : n_Yz,
                b_yZ ? b_yZ->xYz() : n_yZ,
                b_yZ ? b_yZ->XYz() : n_yZ,
                b_YZ ? b_YZ->xyz() : n_YZ,
                b_YZ ? b_YZ->Xyz() : n_YZ);
        }
    }

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

            marchingCube(
                triangles,
                vertices);
        }
    }
} // namespace CGCP
