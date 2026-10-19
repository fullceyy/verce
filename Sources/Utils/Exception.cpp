#include "Exception.h"

#include <ranges>
#include <format>

using namespace Utils;

Exception::Exception(const std::string& what)
    : std::runtime_error(what), mTrace(std::stacktrace::current(1))
{
}

std::string Exception::Stacktrace() const
{
    return std::to_string(mTrace);
}

