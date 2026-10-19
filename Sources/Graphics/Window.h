#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <cstdint>

namespace Graphics
{

class Window
{
private:
    std::uint32_t mWidth {}, mHeight {};
    GLFWwindow* mWindow {};
public:
    explicit Window();
    Window(const std::uint32_t& pWidth, const std::uint32_t& pHeight);
    ~Window() = default;
    
    Window(const Window&) = delete;
    Window operator=(const Window&) = delete;
    
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;
    
    inline GLFWwindow* Get() const { return mWindow; }
    inline float GetWidth() const { return static_cast<float>(mWidth); }
    inline float GetHeight() const { return static_cast<float>(mHeight); }
    
    void SetResolution(const std::uint32_t& pWidth, const std::uint32_t& pHeight);
};

}

// namespace Arch
// {
//     
// class AppWindow
// {
// private:
//     GLFWwindow* mWindow {};
// public:
//     ~AppWindow() 
//     {
//         if(mWindow)
//             glfwDestroyWindow(mWindow);
//      }

//     AppWindow(unsigned int pWidth, unsigned int pHeight)
//     {
//         glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//         glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
//         glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//         glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//         
//         if(mWindow = glfwCreateWindow(pWidth, pHeight, "D", nullptr, nullptr); !mWindow)
//             throw std::runtime_error("Arch::AppWindow Failed to create mWindow!");
//         
//         glfwMakeContextCurrent(mWindow);
//         
//         if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//             throw std::runtime_error("Arch::AppWindow Failed to load glad!");
//         
//         const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
//         
//         if(mode)
//         {
//             int xPos = mode->width / 2 - (pWidth / 2);
//             int yPos = mode->height / 2 - (pHeight / 2);
//             glfwSetWindowPos(mWindow, xPos, yPos);
//         }
//         
//         glViewport(0, 0, pWidth, pHeight);
//     }
//     
//     inline GLFWwindow* Get() const { return mWindow; }
// };

// }