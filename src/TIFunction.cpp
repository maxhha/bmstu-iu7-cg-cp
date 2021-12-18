#include "TIFunction.h"
#include <algorithm>

namespace CGCP
{
    double TIFunction::at(long long x, long long y, long long z) const
    {
        if (clamp_)
        {
            x = std::clamp<long long>(x, 0, (long long)scan_->shape().x() - 1);
            y = std::clamp<long long>(y, 0, (long long)scan_->shape().y() - 1);
            z = std::clamp<long long>(z, 0, (long long)scan_->shape().z() - 1);
        }
        else if (
            x < 0 ||
            y < 0 ||
            z < 0 ||
            x >= scan_->shape().x() ||
            y >= scan_->shape().y() ||
            z >= scan_->shape().z())
        {
            return -1;
        }

        return scan_->at(Vec3Ds(x, y, z));
    }

    double TIFunction::operator()(const Vec3Df &position) const
    {
        Vec3Df p = position / scan_->scale();
        Vec3Dll ps(p);

        double xyz = at(ps.x() + 0, ps.y() + 0, ps.z() + 0);
        double Xyz = at(ps.x() + 1, ps.y() + 0, ps.z() + 0);
        double xYz = at(ps.x() + 0, ps.y() + 1, ps.z() + 0);
        double XYz = at(ps.x() + 1, ps.y() + 1, ps.z() + 0);
        double xyZ = at(ps.x() + 0, ps.y() + 0, ps.z() + 1);
        double XyZ = at(ps.x() + 1, ps.y() + 0, ps.z() + 1);
        double xYZ = at(ps.x() + 0, ps.y() + 1, ps.z() + 1);
        double XYZ = at(ps.x() + 1, ps.y() + 1, ps.z() + 1);

        p = p - ps;

        double yz = xyz + (Xyz - xyz) * p.x();
        double Yz = xYz + (XYz - xYz) * p.x();
        double yZ = xyZ + (XyZ - xyZ) * p.x();
        double YZ = xYZ + (XYZ - xYZ) * p.x();

        double z = yz + (Yz - yz) * p.y();
        double Z = yZ + (YZ - yZ) * p.y();

        return z + (Z - z) * p.z();
    }

} // namespace CGCP
