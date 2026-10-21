#include "Framebuffer.hpp"
#include <iostream>

using namespace glWrap;

// void Framebuffer::PushAttachmentDescriptor(AttachmentDescriptor& pAttachmentDescriptor)
// {
//     mAttachmentDescriptors.emplace_back(pAttachmentDescriptor);
// }


void Framebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mId);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
