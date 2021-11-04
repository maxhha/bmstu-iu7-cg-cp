#pragma once

#include "Mesh.h"
#include <memory>

namespace CGCP
{
    class MeshDrawer
    {
    private:
    protected:
        std::shared_ptr<Mesh> mesh_;

    public:
        virtual void setMesh(const std::shared_ptr<Mesh> mesh) { mesh_ = mesh; };
        virtual void resetTransformation(){};
        virtual void rotate(const Vec3Df &axis, double phi){};
        virtual void translate(const Vec3Df &offset){};
        virtual void scale(const Vec3Df &scale){};

        virtual ~MeshDrawer() = default;
    };
} // namespace CGCP
