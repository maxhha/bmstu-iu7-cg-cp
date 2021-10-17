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

        Vec3Df &start() { return start_; };
        const Vec3Df &start() const { return start_; };

        Vec3Df &end() { return end_; };
        const Vec3Df &end() const { return end_; };

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
