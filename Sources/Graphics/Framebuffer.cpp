#include "Framebuffer.h"

using namespace Graphics;

void Framebuffer::Bind() const
{
    glViewport(0, 0, mWidth, mHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, mId);
}

void Framebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Attach(GLenum pAttachmentTarget, GLuint pRenderTextureId, GLint level)
{
    glNamedFramebufferTexture(mId, pAttachmentTarget, pRenderTextureId, level);
}

void Framebuffer::SetDrawBuffers()
{
    if(!mColorAttachments.empty())
        // for(auto i = 0uz; i < mColorAttachments.size(); i++)
        glNamedFramebufferDrawBuffers(mId, static_cast<GLsizei>(mColorAttachments.size()), mColorAttachments.data());
    else
        glNamedFramebufferDrawBuffer(mId, GL_NONE);
}

void Framebuffer::Validate() const
{
    if(glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw Utils::Exception("Framebuffer::Validate() Framebuffer Status INCOMPLETE!");
}
