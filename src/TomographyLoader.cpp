#include "TomographyLoader.h"
#include "TIFunction.h"

namespace CGCP
{
    TomographyLoader::Function TomographyLoader::buildFunction()
    {
        return std::make_unique<TIFunction>(scan_);
    }
} // namespace CGCP
