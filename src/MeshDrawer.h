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

        virtual ~MeshDrawer() = default;
    };
} // namespace CGCP
