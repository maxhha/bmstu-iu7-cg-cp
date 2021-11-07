#pragma once

#include <atomic>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include "Error.h"
#include "TomographyScan.h"

namespace CGCP
{
    class TomographyLoader
    {
    private:
        std::atomic_bool cancelled_ = false;
        std::atomic_bool finished_ = true;
        pthread_t load_thread_;

    protected:
        using ScanPtr = std::shared_ptr<TomographyScan>;
        using ProgressCallback =
            std::function<void(Error, ScanPtr result, double percent)>;

        virtual void threadLoad(std::string path, ProgressCallback progress) = 0;

        bool isCancelled() { return cancelled_; };
        void finished() { finished_ = true; };

    public:
        TomographyLoader(){};
        void load(std::string path, ProgressCallback progress_receiver);
        void cancel();

        virtual ~TomographyLoader();
    };
} // namespace CGCP
