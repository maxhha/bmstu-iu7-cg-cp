#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "Error.h"
#include "Mesh.h"

namespace CGCP
{
    class MeshSaver
    {
    private:
        std::atomic_bool cancelled_ = false;
        std::atomic_bool finished_ = true;
        pthread_t run_thread_;

    protected:
        using MeshPtr = std::shared_ptr<Mesh>;
        using ProgressCallback = std::function<
            void(CGCP::Error err, bool done, double percent)>;

        virtual void threadSave(
            const std::string path,
            const MeshPtr mesh,
            ProgressCallback progress) = 0;

        bool isCancelled() { return cancelled_; };
        void finished() { finished_ = true; };

    public:
        MeshSaver(){};

        void save(
            const std::string &path,
            const MeshPtr mesh,
            ProgressCallback progress);

        void cancel();

        virtual ~MeshSaver();
    };
} // namespace CGCP
