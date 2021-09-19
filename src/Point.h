#pragma once

namespace CGCP
{
    class Point
    {
    private:
        double x_, y_, z_;

    public:
        Point(double x, double y, double z) noexcept : x_(x), y_(y), z_(z) {}
        Point() noexcept : x_(0), y_(0), z_(0) {}
        ~Point() {}

        double x() const noexcept { return x_; }
        double y() const noexcept { return y_; }
        double z() const noexcept { return z_; }
    };
} // namespace CGCP
