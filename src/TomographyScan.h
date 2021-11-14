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

        double at(std::size_t x, std::size_t y, std::size_t z) const
        {
            return data_[x +
                         y * shape_.x() +
                         z * shape_.x() * shape_.y()];
        };

        double &at(std::size_t x, std::size_t y, std::size_t z)
        {
            return data_[x +
                         y * shape_.x() +
                         z * shape_.x() * shape_.y()];
        };

        double at(const Vec3Ds &p) const
        {
            return at(p.x(), p.y(), p.z());
        };

        double &at(const Vec3Ds &p)
        {
            return at(p.x(), p.y(), p.z());
        };

        ~TomographyScan() = default;
    };
} // namespace CGCP
