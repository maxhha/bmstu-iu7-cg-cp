#include "DMCPolygonizer.h"
#include <QDebug>
#include <chrono>
#include <functional>

namespace CGCP
{
    void DMCPolygonizer::validate(const Config &config){};

    void DMCPolygonizer::run(ProgressCallback progress)
    {
        if (!finished_)
        {
            // TODO: throw exception
            return;
        }

        finished_ = false;
        std::thread thr(&DMCPolygonizer::threadRun, std::ref(*this), progress);
        run_thread_ = thr.native_handle();
        thr.detach();
    };

    void DMCPolygonizer::threadRun(ProgressCallback progress)
    {
        std::shared_ptr<Mesh> result;

        for (int i = 0; i < 10; i++)
        {
            progress(result, (double)i / 10);
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));

            if (cancelled_)
            {
                finished_ = true;
                return;
            }
        }

        CGCP::Vec3Df v0(-5, -5, -5);
        CGCP::Vec3Df v1(-5, -5, +5);
        CGCP::Vec3Df v2(-5, +5, -5);
        CGCP::Vec3Df v3(-5, +5, +5);
        CGCP::Vec3Df v4(+5, -5, -5);
        CGCP::Vec3Df v5(+5, -5, +5);
        CGCP::Vec3Df v6(+5, +5, -5);
        CGCP::Vec3Df v7(+5, +5, +5);

        result = std::make_shared<CGCP::Mesh>(CGCP::Mesh({
            CGCP::Triangle3Df(v0, v1, v3),
            CGCP::Triangle3Df(v0, v2, v3),
            CGCP::Triangle3Df(v4, v5, v7),
            CGCP::Triangle3Df(v4, v6, v7),

            CGCP::Triangle3Df(v0, v1, v5),
            CGCP::Triangle3Df(v0, v4, v5),
            CGCP::Triangle3Df(v2, v3, v7),
            CGCP::Triangle3Df(v2, v6, v7),

            CGCP::Triangle3Df(v1, v3, v7),
            CGCP::Triangle3Df(v1, v5, v7),
            CGCP::Triangle3Df(v0, v4, v6),
            CGCP::Triangle3Df(v0, v2, v6),
        }));

        result->origin() = CGCP::Vec3Df(0, 0, 0);

        progress(result, 1);

        finished_ = true;
    };

    void DMCPolygonizer::cancel()
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
