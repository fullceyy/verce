#include "GLErrors.h"
#include <string>
#include <print>

using namespace Utils;

void CheckGLError()
{
    GLenum e = glGetError();
    while(e != GL_NO_ERROR)
    {
        std::string errorstr;
        switch(e)
        {
            case GL_INVALID_OPERATION:
                errorstr = "INVALID_OPERATION";
                break;
            case GL_INVALID_ENUM:
                errorstr = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                errorstr = "INVALID_VALUE";
                break;
            case GL_OUT_OF_MEMORY:
                errorstr = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorstr = "INVALID_FRAMEBUFFER_OPERATION";
                break;
            default:
                errorstr = std::to_string(e);
                break;
        }
        
        std::println("GL Error detected: {}!", errorstr);
        e = glGetError();
    }
}