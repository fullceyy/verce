#include "ImGuiWindow.h"
#include "imgui_impl_glfw.h"

#include <print>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>

using namespace Graphics;

ImGuiWindow::~ImGuiWindow()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

ImGuiWindow::ImGuiWindow(GLFWwindow* pWindow)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    io.ConfigWindowsMoveFromTitleBarOnly = this->MoveFromTitleBarOnly;
    if(this->IsDockingEnabled)
    {
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        std::println("ImGuiWindow Docking Enabled!");
    }
    if(this->isViewportEnabled)
    {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }
    
    ImGui_ImplGlfw_InitForOpenGL(pWindow, false); // what callbacks?
    ImGui_ImplOpenGL3_Init("#version 450");
}

void ImGuiWindow::RenderStart()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiWindow::RenderStop()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    ImGuiIO& io = ImGui::GetIO();
    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}