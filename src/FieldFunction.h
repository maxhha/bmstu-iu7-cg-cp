#pragma once

#include <functional>

#include "ContinuesFunction.h"

namespace CGCP
{
    class FieldFunction : public ContinuesFunction
    {
        using Function = std::function<double(Vec3Df)>;

    private:
        Function f_;

    public:
        FieldFunction() = delete;
        FieldFunction(Function f, const AABB &domain) : ContinuesFunction(domain), f_(f){};

        virtual double operator()(const Vec3Df &position) const override
        {
            return f_(position);
        }

        virtual ~FieldFunction() override = default;
    };
} // namespace CGCP
