#pragma once

#include "../Vec3D.h"

namespace CGCP
{
    class FieldVertex
    {
    private:
        Vec3Df position_;
        double offset_;

    public:
        FieldVertex(){};
        FieldVertex(const Vec3Df &position, double offset) : position_(position), offset_(offset){};

        Vec3Df &position() { return position_; };
        const Vec3Df &position() const { return position_; };

        double &offset() { return offset_; };
        double offset() const { return offset_; };

        ~FieldVertex() = default;
    };
} // namespace CGCP
