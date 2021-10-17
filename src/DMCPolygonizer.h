#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "Polygonizer.h"

namespace CGCP
{
    class DMCPolygonizer : public Polygonizer
    {
    private:
        using ProgressCallback = Polygonizer::ProgressCallback;
        std::atomic_bool cancelled_ = false;
        std::atomic_bool finished_ = true;
        pthread_t run_thread_;

        void threadRun(ProgressCallback progress);

    protected:
        virtual void validate(const Config &config) override;

    public:
        DMCPolygonizer() : Polygonizer({}){};

        virtual void run(ProgressCallback progress) override;
        virtual void cancel() override;

        virtual ~DMCPolygonizer() override { cancel(); };
    };
} // namespace CGCP
