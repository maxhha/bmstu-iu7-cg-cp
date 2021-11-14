#pragma once

#include "Error.h"
#include "TomographyScan.h"
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <pthread.h>
#include <string>
#include <vector>

namespace CGCP
{
    class TomographyPreprocessor
    {
    private:
        using Config = std::map<std::string, std::string>;
        using ScanPtr = std::shared_ptr<TomographyScan>;
        using Type = std::string;
        using ProgressCallback = std::function<void(Error, ScanPtr, double progress)>;
        using TypeProgressCallback = std::function<bool(Error, double progress)>;
        using TypeFunc = std::function<ScanPtr(const Config &, ScanPtr, TypeProgressCallback)>;
        using Plan = std::vector<std::pair<Type, Config>>;

        std::atomic_bool cancelled_ = false;
        std::atomic_bool finished_ = true;
        pthread_t run_thread_;

        std::map<Type, TypeFunc> preprocessors_;

        void threadRun(const ScanPtr, Plan p, ProgressCallback progress_);

    public:
        TomographyPreprocessor(){};

        TomographyPreprocessor &add(Type type, TypeFunc f);

        void run(const ScanPtr scan, const Plan &plan, ProgressCallback progress);
        void cancel();

        ~TomographyPreprocessor() { cancel(); };
    };
} // namespace CGCP
