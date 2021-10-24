#include "Polygonizer.h"
#include <QDebug>
#include <chrono>
#include <functional>

namespace CGCP
{
    void Polygonizer::run(ProgressCallback progress)
    {
        if (!finished_)
        {
            // TODO: throw exception
            return;
        }

        if (function_ == nullptr)
        {
            // TODO: throw exception
            return;
        }

        finished_ = false;
        std::thread thr(threadRun, std::ref(*this), progress);
        run_thread_ = thr.native_handle();
        thr.detach();
    };

    void Polygonizer::cancel()
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
            pthread_cancel(run_thread_);
            finished_ = true;
        }

        cancelled_ = false;
    };

    Polygonizer::~Polygonizer()
    {
        cancel();
    };
} // namespace CGCP
