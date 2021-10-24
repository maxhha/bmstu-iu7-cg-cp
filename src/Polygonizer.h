#pragma once

#include <atomic>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include "ContinuesFunction.h"
#include "Mesh.h"

namespace CGCP
{
    class Polygonizer
    {
    private:
        std::atomic_bool cancelled_ = false;
        std::atomic_bool finished_ = true;
        pthread_t run_thread_;

    protected:
        using Config = std::map<std::string, std::string>;
        using Function = std::unique_ptr<ContinuesFunction>;
        using ProgressCallback = std::function<void(std::shared_ptr<Mesh> result, double percent)>;

        Config config_;
        Function function_;

        virtual void validate(const Config &config) = 0;
        virtual void threadRun(ProgressCallback progress) = 0;

        bool isCancelled() { return cancelled_; };
        void finished() { finished_ = true; };

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
        Polygonizer &function(Function &function)
        {
            function_ = std::move(function);
            return *this;
        };

        void run(ProgressCallback progress);
        void cancel();

        virtual ~Polygonizer();
    };
} // namespace CGCP
