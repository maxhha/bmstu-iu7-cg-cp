#pragma once

#include <fmt/core.h>
#include <iostream>
#include <memory>

#define THROW_EXCEPTION(...) \
    throw Exception(__FILE__, __LINE__, fmt::format(__VA_ARGS__))

namespace CGCP
{
    class Exception : public std::exception
    {
    public:
        Exception(std::string file, int line, std::string msg);
        virtual const char *what() const noexcept override;

        virtual ~Exception(){};

    protected:
        std::string message;
    };

    using ExceptionPtr = std::unique_ptr<Exception>;
} // namespace CGCP
