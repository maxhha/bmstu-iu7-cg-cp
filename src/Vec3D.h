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

        template <typename B>
        Vec3D<T> operator*(const Vec3D<B> &other) const
        {
            Vec3D<T> v(x_ * other.x(), y_ * other.y(), z_ * other.z());
            return v;
        };

        template <typename B>
        Vec3D<T> operator/(const Vec3D<B> &other) const
        {
            Vec3D<T> v(x_ / other.x(), y_ / other.y(), z_ / other.z());
            return v;
        };

        template <typename B>
        Vec3D<T> operator/(B s) const
        {
            Vec3D<T> v(x_ / s, y_ / s, z_ / s);
            return v;
        };

        template <typename B>
        explicit operator Vec3D<B>()
        {
            Vec3D<B> v((B)x_, (B)y_, (B)z_);
            return v;
        };

        template <typename B>
        Vec3D<T> &operator=(const Vec3D<B> &other)
        {
            x_ = other.x_;
            y_ = other.y_;
            z_ = other.z_;
            return *this;
        }

        template <typename B>
        Vec3D<T> mix(const Vec3D<T> &other, const Vec3D<B> &alpha) const
        {
            Vec3D<T> v(
                x_ * (1 - alpha.x()) + other.x() * alpha.x(),
                y_ * (1 - alpha.y()) + other.y() * alpha.y(),
                z_ * (1 - alpha.z()) + other.z() * alpha.z());
            return v;
        }

        template <typename B>
        T dot(const Vec3D<B> &other)
        {
            return x_ * other.x() + y_ * other.y() + z_ * other.z();
        }

        ~Vec3D() = default;
    };

    using Vec3Df = Vec3D<double>;
    using Vec3Di = Vec3D<int>;
} // namespace CGCP
