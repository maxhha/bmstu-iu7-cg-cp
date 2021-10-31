#pragma once

#include "TomographyLoader.h"

namespace CGCP
{
    class RAWLoader : public TomographyLoader
    {
    protected:
        virtual void threadLoad(
            std::string path,
            ProgressCallback progress) override;

    public:
        RAWLoader(){};
        virtual ~RAWLoader() override = default;
    };
} // namespace CGCP
