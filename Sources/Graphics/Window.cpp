#include "Window.h"
#include "../Utils/Exception.h"

using namespace Graphics;

Window::Window(const std::uint32_t& pWidth, const std::uint32_t& pHeight)
: mWidth(pWidth), mHeight(pHeight)
{
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        if(mWindow = glfwCreateWindow(pWidth, pHeight, "D", nullptr, nullptr); !mWindow)
            throw std::runtime_error("Graphics::Window failed to create mWindow!");
        
        glfwMakeContextCurrent(mWindow);
        
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("Arch::AppWindow Failed to load glad!");
        
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        
        if(mode)
        {
            int xPos = mode->width / 2 - (pWidth / 2);
            int yPos = mode->height / 2 - (pHeight / 2);
            glfwSetWindowPos(mWindow, xPos, yPos);
        }
        
        glViewport(0, 0, pWidth, pHeight);
}

void Window::SetResolution(const std::uint32_t& pWidth, const std::uint32_t& pHeight)
{
        mWidth = pWidth; 
        mHeight = pHeight;
}