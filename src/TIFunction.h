#pragma once

#include "ContinuesFunction.h"
#include "TomographyScan.h"
#include <memory>

namespace CGCP
{

    class TIFunction : public ContinuesFunction
    {
    private:
        using ScanPtr = std::unique_ptr<TomographyScan>;

        ScanPtr scan_;

    public:
        explicit TIFunction(ScanPtr &scan)
            : ContinuesFunction(AABB(Vec3Df(0), scan->scale() * scan->shape())),
              scan_(std::move(scan)){};

        virtual double operator()(const Vec3Df &position) const override;

        ~TIFunction() = default;
    };
} // namespace CGCP
