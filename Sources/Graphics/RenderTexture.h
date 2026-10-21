#pragma once
#include "glad/glad.h"

#include <utility>

namespace Grapics
{

class RenderTexture
{
private:
    GLuint mId {};
    struct {
        GLenum mTarget {GL_TEXTURE_2D};
        GLenum mFormat {GL_RGBA16F};
        GLsizei mWidth {};
        GLsizei mHeight {};
        GLsizei mLevels {1}; // 0 full mip chain
    };
public:
    ~RenderTexture() { glDeleteTextures(1, &mId); }
    
    RenderTexture() = default;
    
    RenderTexture(GLenum pTarget, GLenum pFormat, GLsizei pWidth, GLsizei pHeight, GLsizei pLevels);
    
    RenderTexture(const RenderTexture&) = delete;
    
    RenderTexture& operator=(const RenderTexture&) = delete;
    
    RenderTexture(RenderTexture&& other) noexcept
    : mId(std::exchange(other.mId, 0)), 
    mTarget(other.mTarget),
    mFormat(other.mFormat),
    mWidth(other.mWidth),
    mHeight(other.mHeight),
    mLevels(other.mLevels) {}
    
    RenderTexture& operator=(RenderTexture&& other) noexcept
    {
        if(this != &other)
        {
            glDeleteTextures(1, &mId);
            mId = std::exchange(other.mId, 0);
            mTarget = other.mTarget;
            mFormat = other.mFormat;
            mWidth = other.mWidth;
            mHeight = other.mHeight;
            mLevels = other.mLevels;
        }
        return *this;
    }
    
    void Bind(GLuint pUnit) const;
    void GenerateMips() const;
    
    [[nodiscard("RenderTexture ID discarded!")]] GLuint GetId() const { return mId; }
    [[nodiscard]] GLenum GetTarget() const { return mTarget; }
    [[nodiscard]] GLenum GetFormat() const { return mFormat; }
    [[nodiscard]] GLsizei GetWidth() const { return mWidth; }
    [[nodiscard]] GLsizei GetHeight() const { return mHeight; }
    [[nodiscard]] GLsizei GetLevels() const { return mLevels; }
    [[nodiscard]] bool IsDepth() const;
    [[nodiscard]] std::size_t ByteSize() const;

};

}