#pragma once

#include <vector>

#include "Vec3D.h"

namespace CGCP
{
    class TomographyScan
    {
    private:
        Vec3Ds shape_;
        Vec3Df scale_;
        std::vector<double> data_;

    public:
        TomographyScan(const Vec3Ds &shape, const Vec3Df &scale = Vec3Df(1))
            : shape_(shape),
              scale_(scale),
              data_(shape.x() * shape.y() * shape.z()){};

        Vec3Ds &shape() { return shape_; };
        const Vec3Ds &shape() const { return shape_; };

        Vec3Df &scale() { return scale_; };
        const Vec3Df &scale() const { return scale_; };

        double at(const Vec3Ds &p) const
        {
            return data_[p.x() +
                         p.y() * shape_.x() +
                         p.z() * shape_.x() * shape_.y()];
        };

        double &at(const Vec3Ds &p)
        {
            return data_[p.x() +
                         p.y() * shape_.x() +
                         p.z() * shape_.x() * shape_.y()];
        };

        ~TomographyScan() = default;
    };
} // namespace CGCP
