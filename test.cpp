#include <format>
#include <stacktrace>
#include <iostream>

int main()
{
    auto trace = std::stacktrace::current();
    std::cout << std::format("hello {}\n", 123);
}
