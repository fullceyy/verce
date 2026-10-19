#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <initializer_list>
#include <vector>
#include <memory>

#ifdef NDEBUG
    #define CUSTOMASSERT(x, msg) ((void)0)
#else
    #define CUSTOMASSERT(x, msg) \
    do { \
        if(!(x)) { \
            std::fprintf(stderr, "Assertion failed: %s * Message: %s\n", \
            #x, msg); \
            std::abort(); \
        } \
    } while(0)
#endif

/*
    We have to attach at least one buffer (color, depth or stencil buffer).
    There should be at least one color attachment.
    All attachments should be complete as well (reserved memory).
    Each buffer should have the same number of samples.
*/

#define DEFAULT_FB_VPW 800
#define DEFAULT_FB_VPH 600
// I was annoyed by magic array access numbers
#define VP_WIDTH_POS 2 
#define VP_HEIGHT_POS 3

/*
Framebuffer --
This class is just a wrapper that keeps track of a framebuffer's GLuint ID. 
The ID is simply a handle used by OpenGL to identify the actual framebuffer internally. 
OpenGL doesn't know anything about this class - it only sees the ID when we pass it to an OpenGL 
functions that operate on the framebuffer/require ID to the framebuffer.
{glCreateFramebuffers, glBindFramebuffer, glNamedFramebufferTexture}

What does the class provide:
*** The class remembers the ID that refers to the created FB in GPU
*** The class provides methods such as Bind() & GetAttachmentId(), 
    that have correct GL calls & access to private var's
*** Destruction of the object, maintaince a lifetime

What 'Bookkeeping' is(in terms of FB) & what does the class do:
When we talk about bookkeeping we refer to the fact that it remembers how to interpret
framebuffer via it's settings that we define insode of the class
*/

/*
Current issues --
Framebuffer class has no flexibility — it's one fixed layout.
Right now it always creates exactly: 
color(RGBA16F) + normal(RGBA16F) + depth, sized to whatever GL_VIEWPORT happens to be at construction time. 
This will not survive contact with your actual plan:

*** Cubemap faces for env mapping need small square targets (128–512px), not "whatever the viewport is right now"

*** The mirror pass may want a different resolution than the main pass

*** Deferred G-buffer for SSR realistically needs 3–4 targets (position/depth, normal, albedo, roughness+metallic packed), not 2

*** Grabbing size via glGetIntegerv(GL_VIEWPORT, ...) in the constructor is a hidden ordering dependency 
it silently returns whatever was last set, which only happens to work now because Window runs first in declaration order. 
Pass width, height explicitly into the constructor instead. Don't rely on global GL state you didn't just set yourself.
*/

namespace glWrap
{

struct AttachmentDescriptor
{
    GLenum mCreateTarget {GL_INVALID_ENUM}; // GL_TEXTURE_2D for example;
    GLenum mInternalFormat {GL_INVALID_ENUM}; // GL_RGBA8, GL_DEPTH_COMPONENT32F for example;
    GLenum mAttachmentPoint {GL_INVALID_ENUM}; // GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0 for example;
};

class Framebuffer
{
private:
    // mId to refer to the FB object
    GLuint mId {};
    std::unique_ptr<int[]> mViewportArray;
    
    std::vector<std::pair<GLenum, GLuint>> mAttachmentsAndTheirIds;
    GLsizei mWidth {}, mHeight {};
public:
    ~Framebuffer()
    {
        glDeleteFramebuffers(1, &mId);
    }
    
    Framebuffer() { std::cout << "Framebuffer empty constructor call!\n"; }

    Framebuffer(int pWidth, int pHeight, const std::vector<AttachmentDescriptor>& pAttachmentDescriptorVector)
    : mWidth(pWidth), mHeight(pHeight) 
    {
        glCreateFramebuffers(1, &mId);
        std::vector<GLenum> lDrawBuffers;

        for(const auto& pDesc : pAttachmentDescriptorVector)
        {
            GLuint lNewTexId {};
            glCreateTextures(pDesc.mCreateTarget, 1, &lNewTexId);
            glTextureStorage2D(lNewTexId, 1, pDesc.mInternalFormat, mWidth, mHeight);
            glTextureParameteri(lNewTexId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(lNewTexId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glNamedFramebufferTexture(mId, pDesc.mAttachmentPoint, lNewTexId, 0);

            if(pDesc.mAttachmentPoint >= GL_COLOR_ATTACHMENT0 && pDesc.mAttachmentPoint <= GL_COLOR_ATTACHMENT15)
                lDrawBuffers.emplace_back(pDesc.mAttachmentPoint);
            
            if(!lDrawBuffers.empty())
                glNamedFramebufferDrawBuffers(mId, (GLsizei)lDrawBuffers.size(), lDrawBuffers.data());
            else
                glNamedFramebufferDrawBuffers(mId, 0, GL_NONE); // depth only FBO, e.g: shadow maps.
                
            mAttachmentsAndTheirIds.emplace_back(pDesc.mAttachmentPoint, lNewTexId);
        }

        if(glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer status incomplete!\n";
        unbind();
    }
    
    void bind();
    void unbind();
    inline GLuint GetFbo() const { return mId; }
    
    GLuint GetSpecifiedAttachmentId(GLenum pAttachment) const
    {
        for(const auto& [e, i] : mAttachmentsAndTheirIds) // where e -> enum & i -> ID
        {
            if(e == pAttachment)
                return i;
        }
        std::cout << "Attachment is not in the FB chain. NO ID\n!";
        return 0;
    }
};

}
