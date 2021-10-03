#pragma once

#include "Vec3D.h"

namespace CGCP
{
    template <class V>
    class Triangle
    {
    private:
        V p1_;
        V p2_;
        V p3_;

    public:
        Triangle(const V &p1, const V &p2, const V &p3) : p1_(p1), p2_(p2), p3_(p3){};

        const V &p1() const { return p1_; };
        const V &p2() const { return p2_; };
        const V &p3() const { return p3_; };

        ~Triangle(){};
    };

    using Triangle3Df = Triangle<Vec3Df>;
} // namespace CGCP
