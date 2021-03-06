#pragma once

#include <memory>

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
        explicit Vec3D(T v) : x_(v), y_(v), z_(v){};
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
            x_ = other.x();
            y_ = other.y();
            z_ = other.z();
            return *this;
        }

        template <typename B>
        Vec3D<T> mix(const Vec3D<T> &other, const Vec3D<B> &alpha) const
        {
            Vec3D<T> v(
                x_ + (other.x() - x_) * alpha.x(),
                y_ + (other.y() - y_) * alpha.y(),
                z_ + (other.z() - z_) * alpha.z());
            return v;
        }

        template <typename B>
        Vec3D<T> mix(const Vec3D<T> &other, const B &alpha) const
        {
            Vec3D<T> v(
                x_ * (1 - alpha) + other.x() * alpha,
                y_ * (1 - alpha) + other.y() * alpha,
                z_ * (1 - alpha) + other.z() * alpha);
            return v;
        }

        template <typename B>
        T dot(const Vec3D<B> &other)
        {
            return x_ * other.x() + y_ * other.y() + z_ * other.z();
        }

        template <typename B>
        Vec3D<T> clamp(const Vec3D<B> &min, const Vec3D<B> &max) const
        {
            Vec3D<T> v(
                x_ < min.x()
                    ? min.x()
                : x_ > max.x()
                    ? max.x()
                    : x_,
                y_ < min.y()
                    ? min.y()
                : y_ > max.y()
                    ? max.y()
                    : y_,
                z_ < min.z()
                    ? min.z()
                : z_ > max.z()
                    ? max.z()
                    : z_);

            return v;
        }

        ~Vec3D() = default;
    };

    using Vec3Df = Vec3D<double>;
    using Vec3Di = Vec3D<int>;
    using Vec3Ds = Vec3D<std::size_t>;
    using Vec3Dll = Vec3D<long long>;
} // namespace CGCP
