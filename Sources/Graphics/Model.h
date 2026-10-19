#pragma once
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <glm/glm.hpp>

// #include "Transform.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

namespace Graphics
{

class Model
{
private:
    std::vector<FMesh*> mMeshes;
    std::vector<Material> mMaterials;
    std::string mPath, mDir;
public:
    ~Model();

    Model(const std::string& pPath) 
    : mPath(pPath)
    {
        // Load();
        std::println("Model store request, Path: '{}', Сall Load() Afterwards!", pPath);
    }

    void Load();
    
    void Render(Shader& pShader);
    
private:
    // void InspectModel();

    void ProcessNode(aiNode* pNode, const aiScene* pScene);
    
    FMesh* ProcessMesh(aiMesh* pMesh, const aiScene* pScene);
        
    Material ProcessMaterial(aiMaterial* pMaterial);

};

}