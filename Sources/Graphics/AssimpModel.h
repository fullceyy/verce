#pragma once
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <glm/glm.hpp>

#include "Transform.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"

// AssimpLoader wraps Assimp::Importer, imports the .fbx into an aiScene.
// aiScene is something like a top of hierarchy of the model to be.
// aiScene holds/may hold Root Node, Meshes[], Materials[], Textures[].

// aiScene->mNumMeshes = 3; for example. Then we get aiMesh[3];
// aiMesh[] a singular Mesh may contain vertices[], normals[], texcoords[], faces[], material index.
// 

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

namespace Graphics
{

class AssimpModel
{
private:
    std::vector<FMesh*> mMeshes;
    // std::vector<BaseTexture*> mTextures;
    std::vector<Material> mMaterials;
    Transform mTransform;
    std::string mPath, mDir;
    std::unordered_map<std::string, BaseTexture*> mLoadedTexturePaths;
    
    std::string ResolvePath(const std::string& pModelFile, const std::string& pTexRef)
    {
        std::string modelDir = pModelFile.substr(0, pModelFile.find_last_of("/\\") + 1);
        std::string texRef = pTexRef;
        std::replace(texRef.begin(), texRef.end(), '\\', '/'); // normalize
        return modelDir + texRef;
    }
    
    void ProcessNode(aiNode* pNode, const aiScene* pScene);
    
    FMesh* ProcessMesh(aiMesh* pMesh, const aiScene* pScene);
    
    // Material ProcessMaterial(aiMaterial* pMaterial);
    
    BaseTexture* LoadMaterialTexture(aiMaterial* pMaterial, aiTextureType pTextureType);
    
public:
    ~AssimpModel();
    
    AssimpModel(const std::string& pPath): mPath(pPath) {}
    
    void Load();
    
    // void BindMaterial(Shader& pShader, const Material& pMaterial) const;
    
    void Render(Shader& pShader) const;
    
};

}