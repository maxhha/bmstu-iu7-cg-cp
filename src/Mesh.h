#pragma once

#include "Vec3D.h"
#include <memory>
#include <vector>

#include "AABB.h"
#include "Triangle.h"

namespace CGCP
{
    class Mesh
    {
    private:
        using Triangles = std::vector<Triangle3Df>;
        using TrianglesPtr = std::shared_ptr<Triangles>;
        TrianglesPtr triangles_;
        Vec3Df origin_;
        AABB domain_;

    public:
        Mesh() : triangles_(std::make_shared<std::vector<Triangle3Df>>()){};
        explicit Mesh(TrianglesPtr triangles) : triangles_(triangles){};
        Mesh(std::initializer_list<Triangle3Df> items);
        Mesh(TrianglesPtr triangles, const Vec3Df &origin, const AABB &domain)
            : triangles_(triangles),
              origin_(origin),
              domain_(domain){};

        const Triangles &triangles() const { return *triangles_; };
        Triangles &triangles() { return *triangles_; };

        const Vec3Df &origin() const { return origin_; };
        Vec3Df &origin() { return origin_; };

        const AABB &domain() const { return domain_; };
        AABB &domain() { return domain_; };

        ~Mesh(){};
    };
} // namespace CGCP
