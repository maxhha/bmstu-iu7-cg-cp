#pragma once

#include "Vec3D.h"

namespace CGCP
{
    class AABB
    {
    private:
        Vec3Df start_;
        Vec3Df end_;

    public:
        AABB(){};
        AABB(Vec3Df start, Vec3Df end) : start_(start), end_(end){};
        AABB(
            double start_x,
            double start_y,
            double start_z,
            double end_x,
            double end_y,
            double end_z) : start_(start_x,
                                   start_y,
                                   start_z),
                            end_(end_x,
                                 end_y,
                                 end_z){};

        Vec3Df &start() { return start_; };
        const Vec3Df &start() const { return start_; };

        Vec3Df &end() { return end_; };
        const Vec3Df &end() const { return end_; };

        Vec3Df size() const { return end_ - start_; };

        Vec3Df mix(const Vec3Df &k) const { return start_.mix(end_, k); };

        bool hasPoint(const Vec3Df &point)
        {
            return start_.x() >= point.x() &&
                   start_.y() >= point.y() &&
                   start_.z() >= point.z() &&
                   end_.x() <= point.x() &&
                   end_.y() <= point.y() &&
                   end_.z() <= point.z();
        };

        ~AABB() = default;
    };
} // namespace CGCP
