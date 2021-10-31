#include "CFLoader.h"
#include "Exception.h"
#include <QDebug>
#include <chrono>
#include <functional>

namespace CGCP
{
    void CFLoader::load(std::string path, ProgressCallback progress)
    {
        if (!finished_)
        {
            THROW_EXCEPTION("Load not finished");
        }

        finished_ = false;
        std::thread thr(threadLoad, std::ref(*this), path, progress);
        load_thread_ = thr.native_handle();
        thr.detach();
    }

    void CFLoader::cancel()
    {
        if (finished_)
            return;

        cancelled_ = true;

        // wait 3 seconds before cancel thread using system
        for (int i = 30; !finished_ && i > 0; i--)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (!finished_)
        {
            pthread_cancel(load_thread_);
            finished_ = true;
        }

        cancelled_ = false;
    };

    CFLoader::~CFLoader()
    {
        cancel();
    };
} // namespace CGCP
