#pragma once

#include "Mesh.h"
#include <map>
#include <memory>
#include <string>

namespace CGCP
{
    class MeshDrawer
    {
    private:
    protected:
        using Config = std::map<std::string, std::string>;
        std::shared_ptr<Mesh> mesh_;

    public:
        virtual void setMesh(const std::shared_ptr<Mesh> mesh) { mesh_ = mesh; };
        virtual void resetTransformation(){};
        virtual void rotate(const Vec3Df &axis, double phi){};
        virtual void translate(const Vec3Df &offset){};
        virtual void scale(const Vec3Df &scale){};
        virtual void config(const Config &config){};

        virtual ~MeshDrawer() = default;
    };
} // namespace CGCP
