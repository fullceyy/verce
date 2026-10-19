#pragma once

#include "glad/glad.h"
#include "ktx.h"

#include <assimp/scene.h>
#include <string>
#include <string_view>

namespace Graphics
{

class BaseTexture
{
public:
    BaseTexture() = default;
    virtual ~BaseTexture() = default;

    virtual void CreateTexture() = 0;
    virtual GLenum GetTarget() const = 0;
    virtual GLuint GetId() const = 0;
};

class KtxTexture : public BaseTexture
{
public:
    KtxTexture(const std::string& pPath, aiTextureType pTextureType);
    ~KtxTexture() override;

    void CreateTexture() override;

    GLuint GetId() const override { return mId; }
    GLenum GetTarget() const override { return mTarget; }

private:
    GLuint mId {};
    GLenum mTarget {};
    int mWidth {}, mHeight {};
    aiTextureType mTextureType;
    std::string mPath;
};

class Texture : public BaseTexture
{
public:
    explicit Texture(const std::string& pPath);
    ~Texture() override;

    void CreateTexture() override;
    GLenum GetTarget() const override { return GL_TEXTURE_2D; }
    GLuint GetId() const override { return mId; }
    
    const std::string_view GetPath() const { return mPath; }
    void Bind(GLuint slot) { glBindTextureUnit(slot, mId); }
private:
    GLuint mId {};
    int mWidth {}, mHeight {};
    aiTextureType mTextureType;
    std::string mPath;
};

}