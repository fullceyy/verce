#pragma once

#include <glm/glm.hpp>
#include <print>
#include <memory>

#include "Texture.h"

namespace Graphics
{

// Material answers an important question, how does the surface
// of the model interact with sources of light.
// Material may contain a base color, roughness value, or shininess value
// it may also point to a texture in the files of the model.

// struct Material
// {
//     std::shared_ptr<Texture> mDiffuse;
//     std::shared_ptr<Texture> mNormal;
// };

class Material
{
private:
    glm::vec3 mAlbedo;
    float mRoughness;
    float mMetallicness;
    
    std::shared_ptr<Texture> mAlbedoMap;
    std::shared_ptr<Texture> mNormalMap;
    std::shared_ptr<Texture> mHeightMap;
public:
    Material(const glm::vec3& pDiffuseAlbedo = glm::vec3(.5f, .5f, .5f), float pRoughness = .15f, float pMetallicness = .9f)
    : mAlbedo(pDiffuseAlbedo), mRoughness(pRoughness), mMetallicness(pMetallicness)
    {
        // std::println("Material field details: \n -- Diffuse Albedo - {} {} {},\n Roughness - {}, \n Metallicness - {}", 
        // mAlbedo.x, mAlbedo.y, mAlbedo.z, mRoughness, mMetallicness);
    }
    
    void SetAlbedoMap(std::shared_ptr<Texture> pTex) { mAlbedoMap = std::move(pTex); }
    void SetNormalMap(std::shared_ptr<Texture> pTex) { mNormalMap = std::move(pTex); }
    void SetHeightMap(std::shared_ptr<Texture> pTex) { mHeightMap = std::move(pTex); }

    auto GetAlbedoMap() const { return mAlbedoMap; }
    auto GetNormalMap() const { return mNormalMap; }
    auto GetHeightMap() const { return mHeightMap; }

    inline glm::vec3 GetAlbedoValue() const { return mAlbedo; }
    inline float GetRoughness() const { return mRoughness; }
    inline float GetMetallicness() const { return mMetallicness; }
};

}