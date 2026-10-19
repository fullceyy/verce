#include "MirroredSceneApp.h"

#include <glm/gtc/matrix_transform.hpp>

void MirroredSceneApp::Run()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(.1f, .1f, .1f, 1.f);
    
    const float ratio = mWindow.GetWidth() / mWindow.GetHeight();
    const glm::mat4 projection = glm::perspective(glm::radians(45.f), ratio, .1f, 1000.f);
    
    double dt = 0.f;
    double lastTime = glfwGetTime();

    mShader.Bind();
    
    glm::mat4 view = mCamera.GetViewMatrix();
    
    mUbo->Upload(0, static_cast<GLsizeiptr>(sizeof(glm::mat4)), &view);
    mUbo->Upload(static_cast<GLintptr>(sizeof(glm::mat4)),static_cast<GLsizeiptr>(sizeof(glm::mat4)), &projection);

    mModel.Load();
    
    while(!glfwWindowShouldClose(mWindow.Get()))
    {
        double currentTime = glfwGetTime();
        dt = currentTime - lastTime;
        lastTime = currentTime;
        
        view = mCamera.GetViewMatrix();
        mUbo->Upload(0, static_cast<GLsizeiptr>(sizeof(glm::mat4)), &view);
        
        mShader.Mat4Uniform("uModel", glm::mat4(1.f));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mModel.Render(mShader);
        
        glfwPollEvents();
        glfwSwapBuffers(mWindow.Get());
    }
    
    std::println("Exiting the Application!");
}