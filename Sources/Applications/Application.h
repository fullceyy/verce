#pragma once

#include <print>
#include <string_view>

namespace WindowApp
{

class Application
{
public:
    Application(std::string_view pName) { std::println("Application name: {}!", pName); }
    virtual void Run() = 0;
};

}