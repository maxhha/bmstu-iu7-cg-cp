#pragma once

#include "DrawerSolution.h"
#include <memory>

namespace CGCP
{
    class Engine
    {
    private:
        std::unique_ptr<DrawerSolution> drawer_;

    public:
        Engine() : drawer_(std::make_unique<CGCP::DrawerSolution>()){};

        DrawerSolution &drawer() { return *drawer_; };

        virtual ~Engine() = default;
    };
} // namespace CGCP