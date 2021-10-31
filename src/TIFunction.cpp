#include "TIFunction.h"

namespace CGCP
{
    double TIFunction::operator()(const Vec3Df &position) const
    {
        Vec3Df p = (position / scan_->scale());
        Vec3Ds limit = scan_->shape() - Vec3Ds(1);
        p = p.clamp(Vec3Ds(0), limit);
        Vec3Ds ps(p);

        double xyz = scan_->at(ps + Vec3Ds(0, 0, 0));
        double Xyz = scan_->at(ps + Vec3Ds(1, 0, 0));
        double xYz = scan_->at(ps + Vec3Ds(0, 1, 0));
        double XYz = scan_->at(ps + Vec3Ds(1, 1, 0));
        double xyZ = scan_->at(ps + Vec3Ds(0, 0, 1));
        double XyZ = scan_->at(ps + Vec3Ds(1, 0, 1));
        double xYZ = scan_->at(ps + Vec3Ds(0, 1, 1));
        double XYZ = scan_->at(ps + Vec3Ds(1, 1, 1));

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
