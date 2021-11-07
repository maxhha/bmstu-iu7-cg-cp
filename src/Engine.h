#pragma once

#include "FunctionCreator.h"
#include "Solution.h"
#include <memory>

namespace CGCP
{
    class Engine
    {
    private:
        std::unique_ptr<TomographyLoaderSolution> loader_;
        std::unique_ptr<DrawerSolution> drawer_;
        std::unique_ptr<PolygonizerSolution> polygonizer_;
        FunctionCreator function_;

    public:
        Engine()
            : loader_(std::make_unique<CGCP::TomographyLoaderSolution>()),
              drawer_(std::make_unique<CGCP::DrawerSolution>()),
              polygonizer_(std::make_unique<CGCP::PolygonizerSolution>()){};

        TomographyLoaderSolution &loader() { return *loader_; };
        DrawerSolution &drawer() { return *drawer_; };
        PolygonizerSolution &polygonizer() { return *polygonizer_; };
        FunctionCreator &function() { return function_; };

        virtual ~Engine() = default;
    };
} // namespace CGCP