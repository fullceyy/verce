#pragma once
#include "glad/glad.h"
#include "Utils/Exception.h"

#include <vector>
#include <utility>
/*
    Reflections need an image of the scene from somewhere other than the current camera
    (or of the current frame itself), and an FBO is how you get that image.
    
    Planar Reflection Techinque uses stencil buffer. No static & dynamic variant for FBO.
    
    Planar Render-to-Texture -> 
        Renders scene from the mirrored camera ->  2D Texture -> 
                Re-rendered whenever the camera or scene changes(Dynamic Variant). No static variant for FBO.
    
    Environment Mapping -> Scene from the probe, 6 times -> cube map ->
        static variant baked once at startup -> 
            re-rendered per frame, or spread across frames in dynamic variant.
    
    SSR -> the current frame's color, depth and normals -> several textures ->
        no static variant ->
        always per frame in dynamic variant.
        
    
    Planar Reflection with stencil buffer:
        It's done without FBO, mark the mirror's pixels in the stencil buffer,
        draw the reflected scene only where stencil is set, then draw the normal scene.
    
    Planar Reflection Render-to-Texture:
        This version uses an FBO, and it is more flexible in terms of adding a blur,
        distortion or rendering at reduced resolution.
        
    Static vs Dynamic Variants:
        SSR is always dynamic;
        
        Planar Reflection is baked once from viewport and static;
        
        Environment Mapping is the only technique with bake-once(static) vs render-every-frame(dynamic)
    
    A consistent definition across all three is therefore: 
        static = static scene, only the camera moves; 
        dynamic = the scene contains moving objects; 
    
     the comparison asks a clear question: 
         can this technique reflect moving objects, and at what cost?
         
         Planar: Yes, at full cost(?).
         SSR: Yes, but only objects visible on screen.
         Dynamic Cube Map: Yes, with cost depending on update frequency.
         Baked Cube Map: No.
    
*/

namespace Graphics
{

class Framebuffer
{
private:
    GLuint mId {};
    GLsizei mWidth {}, mHeight {};
    std::vector<GLenum> mColorAttachments;
    
    void Release()
    {
        if(!mColorAttachments.empty())
            mColorAttachments.clear();
            // for(auto i = 0uz; i < mColorAttachments.size(); i++)
                // glClearNamedFramebufferfv(mId, GL_COLOR, i, )
        glDeleteFramebuffers(1, &mId);
        // mId = 0; -> exchange on move does this
    }
public:
    ~Framebuffer() { Release(); }
    
    Framebuffer(int pWidth, int pHeight)
    : mWidth(static_cast<GLsizei>(pWidth)), mHeight(static_cast<GLsizei>(pHeight))
    {
        glCreateFramebuffers(1, &mId);
    }
    
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    
    Framebuffer(Framebuffer&& other) noexcept
    : mId(std::exchange(other.mId, 0)), 
    mWidth(other.mWidth), 
    mHeight(other.mHeight) {}
    
    Framebuffer& operator=(Framebuffer&& other) noexcept
    {
        if(this != &other)
        {
            Release();
            mId = std::exchange(other.mId, 0);
            mWidth = other.mWidth;
            mHeight = other.mHeight;
            mColorAttachments = std::move(other.mColorAttachments);
            other.mColorAttachments.clear();
        }
        return *this;
    }
    
    void Bind() const;
    void Unbind() const;
    void Attach(GLenum pAttachmentTarget, GLuint pRenderTextureId, GLint level = 0);
    void SetDrawBuffers();
    void Validate() const;
    
    [[nodiscard("Framebuffer ID discarded!")]] inline GLuint GetId() const { return mId; }
    [[nodiscard]] inline GLsizei GetWidth() const { return mWidth; }
    [[nodiscard]] inline GLsizei GetHeight() const { return mHeight; }
};

}