#pragma once

#include "GLFW/glfw3.h"

/*
    Refer to examples/imgui_impl_opengl3.cpp and imgui_impl_glfw.cpp for user defined rendering callbacks
    or the handling of flipped clipping rectangles. Another important part is to pass all of the necesarry
    GLFW events into ImGui, including numerous keyboard events, cursor shapes, scrolling and more.
*/

namespace Graphics
{

class ImGuiWindow
{
private:
    struct {
        bool MoveFromTitleBarOnly = true;
        bool IsDockingEnabled = false;
        bool isViewportEnabled = false;
    };
public:
    ImGuiWindow(GLFWwindow* pWindow);
    ~ImGuiWindow();
    
    void RenderStart();
    void RenderStop();
};

}