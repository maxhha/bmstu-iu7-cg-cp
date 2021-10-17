#pragma once

#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>

#include "ContinuesFunction.h"
#include "Mesh.h"

namespace CGCP
{
    class Polygonizer
    {
    protected:
        using Config = std::map<std::string, std::string>;
        using Function = std::unique_ptr<ContinuesFunction>;
        using ProgressCallback = std::function<void(std::shared_ptr<Mesh> result, double percent)>;

        Config config_;
        Function function_;

        virtual void validate(const Config &config) = 0;

    public:
        explicit Polygonizer(std::initializer_list<std::pair<const std::string, std::string>> config) : config_(config){};

        const Config &config() const { return config_; };
        Polygonizer &config(const Config &config)
        {
            validate(config);
            config_ = config;
            return *this;
        }

        const Function &function() const { return function_; };
        virtual Polygonizer &function(Function &function)
        {
            function_ = std::move(function);
            return *this;
        };

        virtual void run(ProgressCallback progress) = 0;
        virtual void cancel() = 0;

        virtual ~Polygonizer() = default;
    };
} // namespace CGCP
