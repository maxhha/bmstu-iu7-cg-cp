#pragma once

#include "AABB.h"

#define EPS 1e-6

namespace CGCP
{
    class ContinuesFunction
    {
    protected:
        AABB domain_;

    public:
        explicit ContinuesFunction(const AABB &domain) : domain_(domain){};

        const AABB &domain() const { return domain_; };

        virtual double operator()(const Vec3Df &position) const = 0;

        Vec3Df grad(const Vec3Df &p) const
        {
            auto s = domain_.size() * Vec3Df(EPS);
            return grad(p, s);
        };

        Vec3Df grad(const Vec3Df &p, const Vec3Df &delta) const
        {
            double c = operator()(p);
            Vec3Df v(
                operator()(p + Vec3Df(delta.x(), 0, 0)) - c,
                operator()(p + Vec3Df(0, delta.y(), 0)) - c,
                operator()(p + Vec3Df(0, 0, delta.z())) - c);
            return v / delta;
        };

        virtual ~ContinuesFunction() = default;
    };
} // namespace CGCP
