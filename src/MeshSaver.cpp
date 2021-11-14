#include "MeshSaver.h"
#include "Exception.h"

namespace CGCP
{
    void MeshSaver::save(const std::string &path, const MeshPtr mesh, ProgressCallback progress)
    {
        if (!finished_)
        {
            THROW_EXCEPTION("Save not finished");
            return;
        }

        finished_ = false;
        std::thread thr(threadSave, std::ref(*this), path, mesh, progress);
        run_thread_ = thr.native_handle();
        thr.detach();
    };

    void MeshSaver::cancel()
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

    MeshSaver::~MeshSaver()
    {
        cancel();
    };
} // namespace CGCP
