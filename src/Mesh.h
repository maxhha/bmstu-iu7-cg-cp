#pragma once

#include "Vec3D.h"
#include <memory>
#include <vector>

#include "Triangle.h"

namespace CGCP
{
    class Mesh
    {
    private:
        using Triangles = std::vector<Triangle3Df>;
        using TrianglesPtr = std::shared_ptr<Triangles>;
        TrianglesPtr triangles_;

    public:
        Mesh() : triangles_(std::make_shared<std::vector<Triangle3Df>>()){};
        Mesh(TrianglesPtr triangles) : triangles_(triangles){};
        Mesh(std::initializer_list<Triangle3Df> items);

        const Triangles &triangles() const { return *triangles_; };
        Triangles &triangles() { return *triangles_; };

        ~Mesh(){};
    };
} // namespace CGCP
