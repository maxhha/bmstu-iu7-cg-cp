#pragma once

#include "CFLoader.h"
#include "TomographyScan.h"

namespace CGCP
{
    class TomographyLoader : public CFLoader
    {
    protected:
        using CFLoader::Function;
        std::unique_ptr<TomographyScan> scan_;

        Function buildFunction();

    public:
        TomographyLoader(){};
    };
} // namespace CGCP
