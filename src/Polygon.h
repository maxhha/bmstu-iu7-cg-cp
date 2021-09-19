#pragma once

#include <vector>
#include "Point.h"

namespace CGCP
{
    class Polygon
    {
    private:
        std::vector<Point> points_;

    public:
        explicit Polygon(std::initializer_list<Point> items) : points_(items){};
        Polygon() : points_(){};
        std::vector<Point> &points() { return points_; };

        ~Polygon() {}
    };
} // namespace CGCP