#pragma once

#include <stacktrace>
#include <stdexcept>
#include <string>

namespace Utils
{

class Exception : public std::runtime_error
{
public:
    Exception(const std::string& what);
    
    std::string Stacktrace() const;
private:
    std::stacktrace mTrace;
};

}

template<>
struct std::formatter<Utils::Exception>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return std::cbegin(ctx);
    }
    
    auto format(const Utils::Exception& obj, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "{}\n{}", obj.what(), obj.Stacktrace());
    }
};
