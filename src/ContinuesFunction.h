#pragma once

#include "AABB.h"

namespace CGCP
{
    class ContinuesFunction
    {
    private:
        AABB domain_;

    public:
        explicit ContinuesFunction(const AABB &domain) : domain_(domain){};

        const AABB &domain() const { return domain_; }

        virtual double operator()(const Vec3Df &position) const = 0;

        virtual ~ContinuesFunction() = default;
    };
} // namespace CGCP
