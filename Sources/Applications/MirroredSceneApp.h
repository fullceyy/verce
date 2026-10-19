#pragma once
#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Model.h"
#include "Graphics/Camera.h"
#include "Application.h"

#include "Utils/GLBuffer.h"

// #include "Utils/Exception.h"

/*    GLFWINIT() IS ALWAYS RAN BEFORE THE APPLICATION IS CREATED    */

class MirroredSceneApp : public WindowApp::Application
{
private:
    Graphics::CameraPositioner_FirstPerson FPSCAMERA {glm::vec3(0), glm::vec3(0, 0, -1.f), glm::vec3(0.f, 1.f, 0.f)};
    Graphics::FinalCamera mCamera;

    std::unique_ptr<Utils::GLUniformBuffer> mUbo;

    Graphics::Window mWindow;
    Graphics::Shader mShader;
    Graphics::Model mModel;
public:
    MirroredSceneApp(const std::string& pModelPath, 
        const std::string& pVert, const std::string& pFrag, 
        const std::uint32_t pWidth, const std::uint32_t pHeight) :
    WindowApp::Application("Mirrored Scene Implementation"),
    mWindow(pWidth, pHeight),
    mShader(pVert.c_str(), pFrag.c_str()),
    mCamera(FPSCAMERA),
    mModel(pModelPath)
    {
        mUbo = std::make_unique<Utils::GLUniformBuffer>((GLsizeiptr)(sizeof(glm::mat4) * 2), GL_DYNAMIC_STORAGE_BIT);
    }
    
    void Run() override;
};

