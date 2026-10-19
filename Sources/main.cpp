#include "Utils/Exception.h"
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Camera.h"
// #include "Graphics/AssimpModel.h"
#include "Graphics/Model.h"

#include <iostream>
#include <print>

struct MouseState
{
    glm::vec2 mPos {0.f};
    bool mPressedLeft = false;
} MouseState;

glm::vec3 cPos {0.f};
glm::vec3 cTarget {0.f, 0.f, -1.f};
glm::vec3 cUp {0.f, 1.f, 0.f};

Graphics::CameraPositioner_FirstPerson AppCameraInterface {cPos, cTarget, cUp};
Graphics::FinalCamera AppCameraFinal {AppCameraInterface};

int main(void)
{
    std::println("Hello World!");
    try
    {
    glfwInit();
        
    Graphics::Window ApplicationWindow {1600, 900};

    glEnable(GL_DEPTH_TEST);
    glClearColor(.1f, .1f, .1f, 1.f);

    glfwSetKeyCallback(ApplicationWindow.Get(), 
    [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        const bool pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_ESCAPE && pressed)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (key == GLFW_KEY_W)
        {
        	AppCameraInterface.movement_.mFrw = pressed;
        	std::print("W");
    	}
        if (key == GLFW_KEY_S)
        {
        	AppCameraInterface.movement_.mBkw = pressed;
        	std::print("S");
    	}
        if (key == GLFW_KEY_A)
        {
        	AppCameraInterface.movement_.mLeft = pressed;
        	std::print("A");
    	}
        if (key == GLFW_KEY_D)
        {
            AppCameraInterface.movement_.mRight = pressed;
            std::print("D");
        }
        if (key == GLFW_KEY_1)
        	AppCameraInterface.movement_.mUp = pressed;
        if (key == GLFW_KEY_2)
        	AppCameraInterface.movement_.mDown = pressed;
        if (mods & GLFW_MOD_SHIFT)
        	AppCameraInterface.movement_.mFast = pressed;
    });
    
    glfwSetCursorPosCallback(ApplicationWindow.Get(), [](GLFWwindow* window, double x, double y)
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        MouseState.mPos.x = static_cast<float>(x / width);
        MouseState.mPos.y = static_cast<float>(y / height);
    });
    
    glfwSetMouseButtonCallback(ApplicationWindow.Get(), [](GLFWwindow* window, int button, int action, int mods)
    {
        if(button == GLFW_MOUSE_BUTTON_LEFT)
            MouseState.mPressedLeft = action == GLFW_PRESS;
    });
    Graphics::Shader AppShader {"Assets/sibenik/shader.vert", "Assets/sibenik/shader.frag"};
    AppShader.Bind();
    // Graphics::AssimpModel AppModel {"Assets/sibenik/sibenik.obj"}; //write a simple path resolver, in utils.
    // AppModel.Load();
    Graphics::Model sib {"Assets/sibenik/sibenik.obj"};
    sib.Load();
    const float ratio = 1600 / (float)900;
    const glm::mat4 p = glm::perspective(glm::radians(45.f), ratio, 0.1f, 1000.f);
    double dt = 0.f;
    double lastTime = glfwGetTime();
    
    unsigned int uboMatrices;
    GLsizeiptr uboSize {2 * sizeof(glm::mat4)};
    glCreateBuffers(1, &uboMatrices);
    glNamedBufferStorage(uboMatrices, uboSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);
    auto view = AppCameraFinal.GetViewMatrix();
    // glm::mat4 view = glm::lookAt(
    //     glm::vec3(0.f, 5.f, 30.f),
    //     glm::vec3(0.f, 0.f, 0.f),
    //     glm::vec3(0.f, 1.f, 0.f)
    // );


    glNamedBufferSubData(
        uboMatrices,
        0,
        sizeof(glm::mat4),
        &view
    );
    
    glNamedBufferSubData(
        uboMatrices,
        sizeof(glm::mat4),
        sizeof(glm::mat4),
        &p
    );
    
    // Calling glBindBufferBase is equivalent to 
    // calling glBindBufferRange with offset zero 
    // and size equal to the size of the buffer.
    
    while(!glfwWindowShouldClose(ApplicationWindow.Get()))
    {
        double currentTime = glfwGetTime();
        dt = currentTime - lastTime;
        lastTime = currentTime;
        
        AppCameraInterface.Update(dt, MouseState.mPos, MouseState.mPressedLeft);
        
        AppShader.Mat4Uniform("uModel", glm::mat4(1.f));
        view = AppCameraFinal.GetViewMatrix();
        glNamedBufferSubData(uboMatrices, 0, sizeof(glm::mat4), &view);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // std::println("Camera position: {}, {}, {}",
        // AppCameraInterface.GetPosition().x,
        // AppCameraInterface.GetPosition().y,
        // AppCameraInterface.GetPosition().z);
        
        // AppModel.Render(AppShader);
        sib.Render(AppShader);
        glfwPollEvents();
        glfwSwapBuffers(ApplicationWindow.Get());
    }

    } catch(Utils::Exception& err) {
        std::println(std::cerr, "exception: {}", err);
    }
    
    return 0;
}