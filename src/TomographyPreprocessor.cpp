#include "TomographyPreprocessor.h"
#include "Exception.h"
#include <chrono>
#include <thread>

namespace CGCP
{
    TomographyPreprocessor &TomographyPreprocessor::add(Type type, TypeFunc f)
    {
        if (!finished_)
        {
            THROW_EXCEPTION("TomographyPreprocessor not finished");
            return *this;
        }
        preprocessors_[type] = f;
        return *this;
    };

    void TomographyPreprocessor::run(const ScanPtr scan, const Plan &p, ProgressCallback progress)
    {
        if (!finished_)
        {
            THROW_EXCEPTION("TomographyPreprocessor not finished");
            return;
        }

        if (!scan)
        {
            THROW_EXCEPTION("Scan is not set");
            return;
        }

        for (auto &it : p)
        {
            if (preprocessors_.count(it.first) == 0)
            {
                THROW_EXCEPTION("Unknown preprocessor: {}", it.first);
            }
        }

        finished_ = false;
        std::thread thr(threadRun, std::ref(*this), scan, p, progress);
        run_thread_ = thr.native_handle();
        thr.detach();
    };

    void TomographyPreprocessor::threadRun(
        const ScanPtr scan,
        Plan plan,
        ProgressCallback progress_receiver)
    {
        progress_receiver(Error::OK, nullptr, 0);

        double progress = 0;
        double weight = 1.0 / (plan.size() + 1);
        auto s = std::shared_ptr<TomographyScan>(scan);

        for (auto &it : plan)
        {
            if (cancelled_)
                break;

            s = preprocessors_[it.first](
                it.second, s,
                [&](Error err, double p) -> bool
                {
                    progress_receiver(err, nullptr, progress + weight * p);

                    return cancelled_;
                });

            progress += weight;
        }

        finished_ = true;

        if (cancelled_)
        {
            return;
        }

        progress_receiver(Error::OK, s, 1);
    };

    void TomographyPreprocessor::cancel()
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
} // namespace CGCP
