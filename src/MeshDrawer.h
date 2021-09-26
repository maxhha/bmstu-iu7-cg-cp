#pragma once

#include "Mesh.h"
#include <memory>

namespace CGCP
{
    class MeshDrawer
    {
    private:
    public:
        virtual void setMesh(std::shared_ptr<Mesh> mesh) = 0;

        virtual ~MeshDrawer() = default;
    };
} // namespace CGCP
