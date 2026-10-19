#include "Model.h"

#include <print>

using namespace Graphics;

Model::~Model()
{
    if(!mMeshes.empty())
        for(auto& mesh : mMeshes)
            delete mesh;
}

void Model::Load()
{
    // InspectModel();
    Assimp::Importer Importer;
    const aiScene* scene = Importer.ReadFile(mPath, ASSIMP_LOAD_FLAGS);

    if(!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        throw std::runtime_error("Assimp failed to load " + mPath + "': " + Importer.GetErrorString());

    mDir = mPath.substr(0, mPath.find_last_of('/'));

    std::println("Materials: {}", scene->mNumMaterials);
    mMaterials.reserve(scene->mNumMaterials);

    for(auto i = 0u; i < scene->mNumMaterials; i++)
    {
        aiMaterial* mat = scene->mMaterials[i];
        mMaterials.push_back(ProcessMaterial(mat));
    }
    
    ProcessNode(scene->mRootNode, scene);
    
    std::println("Meshes loaded: {}", mMeshes.size());
    std::println("Materials loaded: {}", mMaterials.size());
}

void Model::ProcessNode(aiNode* pNode, const aiScene* pScene)
{
    for(unsigned int i = 0; i < pNode->mNumMeshes; i++)
    {
        aiMesh* mesh = pScene->mMeshes[pNode->mMeshes[i]];
        mMeshes.push_back(ProcessMesh(mesh, pScene));
    }
    
    for(unsigned int i = 0; i < pNode->mNumChildren; i++)
        ProcessNode(pNode->mChildren[i], pScene);
}

FMesh* Model::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
{
    std::vector<Vertex> vertices;
    vertices.reserve(pMesh->mNumVertices);
    
    std::vector<std::uint32_t> indices;
    indices.reserve(pMesh->mNumFaces * 3);
    
    for(auto i = 0u; i < pMesh->mNumVertices; i++)
    {
        Vertex v;
        
        v.mPos = glm::vec3(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);
        
        if(pMesh->HasNormals())
        {
            v.mNormal = glm::vec3(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);
        } else {
            v.mNormal = glm::vec3(0.f);
        }
        
        if(pMesh->HasTextureCoords(0))
        {
            v.mTexCoord = glm::vec2(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
        } else {
            v.mTexCoord = glm::vec2(0.f);
        }
        
        if(pMesh->HasTangentsAndBitangents())
        {
            v.mTangent = glm::vec3(pMesh->mTangents[i].x, pMesh->mTangents[i].y, pMesh->mTangents[i].z);
            v.mBitangent = glm::vec3(pMesh->mBitangents[i].x, pMesh->mBitangents[i].y, pMesh->mBitangents[i].z);
        } else {
            v.mTangent = glm::vec3(0.f);
            v.mBitangent = glm::vec3(0.f);
        }
        
        vertices.push_back(v);
    }
    
    for(auto i = 0u; i < pMesh->mNumFaces; i++)
    {
        const aiFace& face = pMesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }
    
    // std::println("vertices: {}, {}, {}", vertices[0].mPos.x, vertices[0].mPos.y, vertices[0].mPos.z);
    return new FMesh {vertices, indices, static_cast<std::uint8_t>(pMesh->mMaterialIndex)};
}

Material Model::ProcessMaterial(aiMaterial* pMaterial)
{
    aiColor4D diffuseColor;
    
    if(pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) != AI_SUCCESS)
        diffuseColor = aiColor4D {1.f, 1.f, 1.f, 1.f};
        
    Material mat {glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b)};
    
    if(pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString path;

        if(pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
        {
            std::string resolvedPath = mDir + "/" + path.C_Str();
            mat.SetAlbedoMap(std::make_shared<Texture>(resolvedPath));
        }
    }
    
    if(pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        aiString path;

        if(pMaterial->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
        {
            std::string resolvedPath = mDir + "/" + path.C_Str();
            mat.SetNormalMap(std::make_shared<Texture>(resolvedPath));
        }
    }

    if(pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        aiString path;

        if(pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &path) == AI_SUCCESS)
        {
            std::string resolvedPath = mDir + "/" + path.C_Str();
            mat.SetHeightMap(std::make_shared<Texture>(resolvedPath));
        }
    }
    return mat;
}

void Model::Render(Shader& pShader)
{
        // std::println(
        // "Mesh 0: vertices={}, indices={}, material={}",
        //     mMeshes[0]->GetVertexCount(),
        //     mMeshes[0]->GetIndicesCount(),
        //     mMeshes[0]->GetMaterialIndex()
        // );

    for(auto* mesh : mMeshes)
    {
        const Material& mat = mMaterials[mesh->GetMaterialIndex()];
        
        pShader.Vec3Uniform("uMaterial.albedo", mat.GetAlbedoValue());
        pShader.FloatUniform("uMaterial.roughness", mat.GetRoughness());
        pShader.FloatUniform("uMaterial.metallic", mat.GetMetallicness());
        
        if(mat.GetAlbedoMap())
        {
            mat.GetAlbedoMap()->Bind(0);
            pShader.IntUniform("uAlbedoMap", 0);
            pShader.IntUniform("uHasAlbedoMap", 1);
        }
        else
        {
            pShader.IntUniform("uHasAlbedoMap", 0);
        }
        
        // if(mat.GetNormalMap()) { mat.GetNormalMap()->Bind(1);}
        
        mesh->Render();
        
        }
}


// void Model::InspectModel()
// {
//     Assimp::Importer importer;

//     const aiScene* scene = importer.ReadFile(
//         mPath,
//         aiProcess_Triangulate |
//         aiProcess_GenSmoothNormals |
//         aiProcess_JoinIdenticalVertices |
//         aiProcess_FlipUVs
//     );

//     if(!scene ||
//        !scene->mRootNode ||
//        scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
//     {
//         throw std::runtime_error(
//             "Assimp failed: " + std::string(importer.GetErrorString()));
//     }

//     std::println("========================================");
//     std::println("Model: {}", mPath);
//     std::println("Meshes: {}", scene->mNumMeshes);
//     std::println("Materials: {}", scene->mNumMaterials);
//     std::println("========================================");

//     Materials

//     for(unsigned int i = 0; i < scene->mNumMaterials; ++i)
//     {
//         const aiMaterial* material = scene->mMaterials[i];

//         std::println("");
//         std::println("Material [{}]", i);
//         std::println("--------------------");

//         aiString name;

//         if(material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
//             std::println("Name: {}", name.C_Str());

//         Diffuse
//         for(unsigned int j = 0;
//             j < material->GetTextureCount(aiTextureType_DIFFUSE);
//             ++j)
//         {
//             aiString texturePath;

//             if(material->GetTexture(
//                    aiTextureType_DIFFUSE,
//                    j,
//                    &texturePath) == AI_SUCCESS)
//             {
//                 std::println(
//                     "Diffuse[{}]: {}",
//                     j,
//                     texturePath.C_Str());
//             }
//         }

//         Normals
//         for(unsigned int j = 0;
//             j < material->GetTextureCount(aiTextureType_NORMALS);
//             ++j)
//         {
//             aiString texturePath;

//             if(material->GetTexture(
//                    aiTextureType_NORMALS,
//                    j,
//                    &texturePath) == AI_SUCCESS)
//             {
//                 std::println(
//                     "Normal[{}]: {}",
//                     j,
//                     texturePath.C_Str());
//             }
//         }

//         Height / bump maps
//         for(unsigned int j = 0;
//             j < material->GetTextureCount(aiTextureType_HEIGHT);
//             ++j)
//         {
//             aiString texturePath;

//             if(material->GetTexture(
//                    aiTextureType_HEIGHT,
//                    j,
//                    &texturePath) == AI_SUCCESS)
//             {
//                 std::println(
//                     "Height/Bump[{}]: {}",
//                     j,
//                     texturePath.C_Str());
//             }
//         }

//         Diffuse color
//         aiColor4D diffuseColor;

//         if(material->Get(
//                AI_MATKEY_COLOR_DIFFUSE,
//                diffuseColor) == AI_SUCCESS)
//         {
//             std::println(
//                 "Diffuse Color: {}, {}, {}, {}",
//                 diffuseColor.r,
//                 diffuseColor.g,
//                 diffuseColor.b,
//                 diffuseColor.a);
//         }
//     }
// }










