#pragma once

#include "MeshSaver.h"

namespace CGCP
{
    class STLSaver : public MeshSaver
    {
    protected:
        using MeshSaver::MeshPtr;
        using MeshSaver::ProgressCallback;

        virtual void threadSave(
            const std::string path,
            const MeshPtr mesh,
            ProgressCallback progress) override;

    public:
        STLSaver(){};
        virtual ~STLSaver() override = default;
    };

} // namespace CGCP
