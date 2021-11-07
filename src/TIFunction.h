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

        double at(long long x, long long y, long long z) const;

    public:
        explicit TIFunction(ScanPtr &scan)
            : ContinuesFunction(AABB(
                  scan->scale() * Vec3Df(-1),
                  scan->scale() * (scan->shape() + Vec3Ds(1)))),
              scan_(std::move(scan)){};

        const ScanPtr scan() const { return scan_; };
        ScanPtr scan() { return scan_; };

        virtual double
        operator()(const Vec3Df &position) const override;

        ~TIFunction() = default;
    };
} // namespace CGCP
