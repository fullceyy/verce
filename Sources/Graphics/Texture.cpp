#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <iostream>

using namespace Graphics;

constexpr int DESIRED_CHANNELS = 4;

KtxTexture::KtxTexture(const std::string& pPath, aiTextureType pTextureType)
    : mPath(pPath), mTextureType(pTextureType)
{
    KtxTexture::CreateTexture();
}

KtxTexture::~KtxTexture()
{
    if (mId) glDeleteTextures(1, &mId);
}

void KtxTexture::CreateTexture()
{
    ktxTexture* kTexture;

    KTX_error_code ktxresult = ktxTexture_CreateFromNamedFile(
        mPath.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture);

    if (ktxresult != KTX_SUCCESS)
    {
        std::cout << "ktxTexture_CreateFromNamedFile() resulted in KTX_FAILURE!\n";
        return;
    }

    mWidth = kTexture->baseWidth;
    mHeight = kTexture->baseHeight;

    GLenum GLError;
    ktxresult = ktxTexture_GLUpload(kTexture, &mId, &mTarget, &GLError);
    ktxTexture_Destroy(kTexture);

    if (ktxresult != KTX_SUCCESS)
    {
        std::cout << "ktxTexture_GLUpload() resulted in KTX_FAILURE!\n";
        return;
    }

    glBindTexture(mTarget, mId);
    if (kTexture->numLevels == 1)
        glGenerateMipmap(mTarget);

    glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(mTarget, 0);
}


Texture::Texture(const std::string& pPath)
    : mPath(pPath)
{
    Texture::CreateTexture();
}

Texture::~Texture()
{
    if (mId) glDeleteTextures(1, &mId);
}

void Texture::CreateTexture()
{
    stbi_set_flip_vertically_on_load(true);

    int channels;
    auto data = stbi_load(mPath.c_str(), &mWidth, &mHeight, &channels, DESIRED_CHANNELS);
    if (!data)
    {
        std::cout << "Texture::CreateTexture() failure to load Texture: " << mPath << "\n";
        return;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &mId);
    glTextureStorage2D(mId, 1, GL_RGBA8, mWidth, mHeight);
    glTextureSubImage2D(mId, 0, 0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(mId);

    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}
