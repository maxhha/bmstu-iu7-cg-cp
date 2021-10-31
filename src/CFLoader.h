#pragma once

#include <atomic>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include "ContinuesFunction.h"
#include "Error.h"

namespace CGCP
{
    class CFLoader
    {
    private:
        std::atomic_bool cancelled_ = false;
        std::atomic_bool finished_ = true;
        pthread_t load_thread_;

    protected:
        using Function = std::unique_ptr<ContinuesFunction>;
        using ProgressCallback =
            std::function<void(Error, Function result, double percent)>;

        virtual void threadLoad(std::string path, ProgressCallback progress) = 0;

        bool isCancelled() { return cancelled_; };
        void finished() { finished_ = true; };

    public:
        CFLoader(){};
        void load(std::string path, ProgressCallback progress_receiver);
        void cancel();

        virtual ~CFLoader();
    };
} // namespace CGCP
