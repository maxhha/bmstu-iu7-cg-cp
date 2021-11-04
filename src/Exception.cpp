#include "Exception.h"

namespace CGCP
{
    Exception::Exception(std::string file,
                         int line,
                         std::string msg = "Unknown error") : message_(msg)
    {
        what_message_ = std::string("") +
                        "\n" + "At " + file + ":" + std::to_string(line) +
                        "\n" + msg;
    }

    const char *Exception::what() const noexcept
    {
        return what_message_.c_str();
    }
} // namespace CGCP
