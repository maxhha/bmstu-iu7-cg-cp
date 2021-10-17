#pragma once

#include "Solution.h"
#include <memory>

namespace CGCP
{
    class Engine
    {
    private:
        std::unique_ptr<DrawerSolution> drawer_;
        std::unique_ptr<PolygonizerSolution> polygonizer_;

    public:
        Engine() : drawer_(std::make_unique<CGCP::DrawerSolution>()),
                   polygonizer_(std::make_unique<CGCP::PolygonizerSolution>()){};

        DrawerSolution &drawer() { return *drawer_; };
        PolygonizerSolution &polygonizer() { return *polygonizer_; };

        virtual ~Engine() = default;
    };
} // namespace CGCP