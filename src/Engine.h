#pragma once

#include "Solution.h"
#include <memory>

namespace CGCP
{
    class Engine
    {
    private:
        std::unique_ptr<CFLoaderSolution> loader_;
        std::unique_ptr<DrawerSolution> drawer_;
        std::unique_ptr<PolygonizerSolution> polygonizer_;

    public:
        Engine()
            : loader_(std::make_unique<CGCP::CFLoaderSolution>()),
              drawer_(std::make_unique<CGCP::DrawerSolution>()),
              polygonizer_(std::make_unique<CGCP::PolygonizerSolution>()){};

        CFLoaderSolution &loader() { return *loader_; };
        DrawerSolution &drawer() { return *drawer_; };
        PolygonizerSolution &polygonizer() { return *polygonizer_; };

        virtual ~Engine() = default;
    };
} // namespace CGCP