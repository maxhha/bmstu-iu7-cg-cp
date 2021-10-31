#include "Mesh.h"
#include "Exception.h"

namespace CGCP
{
    Mesh::Mesh(std::initializer_list<Triangle3Df> items)
        : triangles_(std::make_shared<Triangles>())
    {
        for (auto it : items)
        {
            triangles_->emplace_back(it);
        }
    }
} // namespace CGCP
