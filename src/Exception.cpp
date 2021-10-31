#include "Exception.h"

namespace CGCP
{
    Exception::Exception(std::string file,
                         int line,
                         std::string msg = "Unknown error")
    {
        message = std::string("") +
                  "\n" + "At " + file + ":" + std::to_string(line) +
                  "\n" + msg;
    }

    const char *Exception::what() const noexcept
    {
        return message.c_str();
    }
} // namespace CGCP
