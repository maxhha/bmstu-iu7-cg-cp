#pragma once

#include "ContinuesFunction.h"
#include "TomographyScan.h"
#include <memory>

namespace CGCP
{

    class TIFunction : public ContinuesFunction
    {
    private:
        using ScanPtr = std::shared_ptr<TomographyScan>;

        ScanPtr scan_;
        bool clamp_ = false;

        double at(long long x, long long y, long long z) const;

    public:
        TIFunction(ScanPtr &scan, bool clamp, bool with_edges)
            : ContinuesFunction(AABB(
                  scan->scale() * Vec3Df(with_edges ? -2 : 0),
                  scan->scale() * (scan->shape() + Vec3Ds(with_edges ? 2 : 0)))),
              scan_(scan),
              clamp_(clamp){};

        virtual double
        operator()(const Vec3Df &position) const override;

        ~TIFunction() = default;
    };
} // namespace CGCP
