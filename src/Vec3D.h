#pragma once

namespace CGCP
{
    template <typename T>
    class Vec3D
    {
    private:
        T x_;
        T y_;
        T z_;

    public:
        Vec3D(){};
        Vec3D(T x, T y, T z) : x_(x), y_(y), z_(z){};

        T &x() { return x_; };
        T &y() { return y_; };
        T &z() { return z_; };

        const T &x() const { return x_; };
        const T &y() const { return y_; };
        const T &z() const { return z_; };

        template <typename B>
        Vec3D<T> operator+(const Vec3D<B> &other) const
        {
            Vec3D<T> v(x_ + other.x(), y_ + other.y(), z_ + other.z());
            return v;
        };

        template <typename B>
        Vec3D<T> operator-(const Vec3D<B> &other) const
        {
            Vec3D<T> v(x_ - other.x(), y_ - other.y(), z_ - other.z());
            return v;
        };

        ~Vec3D(){};
    };

    using Vec3Df = Vec3D<double>;
    using Vec3Di = Vec3D<int>;
} // namespace CGCP
