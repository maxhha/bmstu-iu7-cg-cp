#pragma once

#include "Solution.h"
#include "TomographyPreprocessor.h"
#include <memory>

namespace CGCP
{
    class Engine
    {
    private:
        std::unique_ptr<TomographyLoaderSolution> loader_;
        std::unique_ptr<DrawerSolution> drawer_;
        std::unique_ptr<PolygonizerSolution> polygonizer_;
        std::unique_ptr<TomographyPreprocessor> preprocessor_;
        std::unique_ptr<MeshSaverSolution> saver_;

    public:
        Engine();

        TomographyLoaderSolution &loader() { return *loader_; };
        DrawerSolution &drawer() { return *drawer_; };
        PolygonizerSolution &polygonizer() { return *polygonizer_; };
        TomographyPreprocessor &preprocessor() { return *preprocessor_; };
        MeshSaverSolution &saver() { return *saver_; };

        virtual ~Engine() = default;
    };
} // namespace CGCP