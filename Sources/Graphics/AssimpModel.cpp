#include "AssimpModel.h"
#include <print>

using namespace Graphics;

AssimpModel::~AssimpModel()
{
    if(!mTextures.empty())
        for(auto& t : mTextures)
            delete t;

    if(!mMeshes.empty())
        for(auto& m : mMeshes)
            delete m;
}

BaseTexture* AssimpModel::LoadMaterialTexture(aiMaterial* pMaterial, aiTextureType pTextureType)
{
    if(pMaterial->GetTextureCount(pTextureType) == 0)
        return nullptr;
    
    aiString str;
    if(pMaterial->GetTexture(pTextureType, 0, &str) != AI_SUCCESS)
    {
        std::println("LoadMaterialTexture() couldn't find the texture with specified type!");
        return nullptr;
    }

    std::string resolvedPath = ResolvePath(mPath, str.C_Str());

    if(auto it = mLoadedTexturePaths.find(resolvedPath); it != mLoadedTexturePaths.end())
        return it->second;

    if(resolvedPath.ends_with(".jpg"))
        mTextures.push_back(new Texture{resolvedPath});
    
    // if(resolvedPath.ends_with(".ktx"))
    //     mTextures.push_back(new KtxTexture{resolvedPath, pTextureType});

    auto* emplaced = mTextures.back();
    mLoadedTexturePaths.emplace(resolvedPath, emplaced);
    return emplaced;
}

// Material AssimpModel::ProcessMaterial(aiMaterial* pMaterial)
// {
//     return Material
//     {
//         LoadMaterialTexture(pMaterial, aiTextureType_DIFFUSE),
//         LoadMaterialTexture(pMaterial, aiTextureType_SPECULAR),
//         LoadMaterialTexture(pMaterial, aiTextureType_DIFFUSE_ROUGHNESS),
//         LoadMaterialTexture(pMaterial, aiTextureType_NORMALS),
//         LoadMaterialTexture(pMaterial, aiTextureType_HEIGHT)
//     };
// }

// Material AssimpModel::ProcessMaterial(aiMaterial* pMaterial)
// {
//     
// }

void AssimpModel::Load()
{
    Assimp::Importer Importer;
    const aiScene* scene = Importer.ReadFile(mPath, ASSIMP_LOAD_FLAGS);
    if(!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        throw std::runtime_error("Assimp failed to load " + mPath + "': " + Importer.GetErrorString());

    mDir = mPath.substr(0, mPath.find_last_of('/'));

    // mMaterials.reserve(scene->mNumMaterials);
    std::println("Materials: ", scene->mNumMaterials);
    
    for(unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* mat = scene->mMaterials[i];
        aiString name;
        mat->Get(AI_MATKEY_NAME, name);
        std::println("-- ", name.C_Str());
        
        BaseTexture* diffuse = LoadMaterialTexture(mat, aiTextureType_DIFFUSE);
        // mTextures.push_back(diffuse);
        
        if(diffuse)
        {
            aiString path;
            mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            std::println("   Diffuse: {}", path.C_Str());
        }
    }
    
    ProcessNode(scene->mRootNode, scene);
    
    std::cout << "Loaded " << mMeshes.size() << " meshes\n";
}


void AssimpModel::ProcessNode(aiNode* pNode, const aiScene* pScene)
{ // process all meshes
    for(unsigned int i = 0; i < pNode->mNumMeshes; i++)
    {
        aiMesh* mesh = pScene->mMeshes[pNode->mMeshes[i]];
        mMeshes.push_back(ProcessMesh(mesh, pScene));
    }
    
    for(unsigned int i = 0; i < pNode->mNumChildren; i++)
        ProcessNode(pNode->mChildren[i], pScene);
}

Mesh* AssimpModel::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
{
    constexpr unsigned int floatsPerVertex = 8;
    
    std::vector<float> vertices;
    vertices.reserve(pMesh->mNumVertices * floatsPerVertex);
    
    std::vector<AttributeDescriptor> desc;
    std::vector<unsigned int> indices;
    indices.reserve(pMesh->mNumFaces * 3);
    
    if(pMesh->HasPositions())
        desc.push_back(AttributeDescriptor{3, 0});
        
    if(pMesh->HasNormals())
        desc.push_back(AttributeDescriptor{3, 1});
        
    if(pMesh->HasTextureCoords(0))
        desc.push_back(AttributeDescriptor{2, 2});
    
    for(unsigned int i = 0; i < pMesh->mNumVertices; i++)
    {
        vertices.push_back(pMesh->mVertices[i].x);
        vertices.push_back(pMesh->mVertices[i].y);
        vertices.push_back(pMesh->mVertices[i].z);
        
        vertices.push_back(pMesh->mNormals[i].x);
        vertices.push_back(pMesh->mNormals[i].y);
        vertices.push_back(pMesh->mNormals[i].z);
        
        if(pMesh->HasTextureCoords(0))
        {
            vertices.push_back(pMesh->mTextureCoords[0][i].x);
            vertices.push_back(pMesh->mTextureCoords[0][i].y);
        } else {
            vertices.push_back(0.f);
            vertices.push_back(0.f);
        }
    }
    
    for(unsigned int i = 0; i < pMesh->mNumFaces; i++)
    {
        const aiFace& face = pMesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // if(pMesh->mMaterialIndex >= 0)
    // { each mesh has an index that points to scenes list of materials
    //     aiMaterial* mat = pScene->mMaterials[pMesh->mMaterialIndex];
    // }

    return new Mesh{vertices, desc, indices, pMesh->mMaterialIndex};
}

// void AssimpModel::BindMaterial(Shader& pShader, const Material& pMaterial) const
// {
//     glActiveTexture(GL_TEXTURE0);
//     glBindTexture(GL_TEXTURE_2D, pMaterial.diffuse ? pMaterial.diffuse->GetId() : 0);
//     pShader.IntUniform("uDiffuseMap", 0);

//     glActiveTexture(GL_TEXTURE1);
//     glBindTexture(GL_TEXTURE_2D, pMaterial.specular ? pMaterial.specular->GetId() : 0);
//     pShader.IntUniform("uSpecularMap", 1);

//     glActiveTexture(GL_TEXTURE2);
//     glBindTexture(GL_TEXTURE_2D, pMaterial.roughness ? pMaterial.roughness->GetId() : 0);
//     pShader.IntUniform("uRoughnessMap", 2);
//     
//     glActiveTexture(GL_TEXTURE3);
//     glBindTexture(GL_TEXTURE_2D, pMaterial.normal ? pMaterial.normal->GetId() : 0);
//     pShader.IntUniform("uNormalMap", 3);
// }

void AssimpModel::Render(Shader& pShader) const
{
    pShader.Bind();
    
    for(const Mesh* mesh : mMeshes)
    {
        // const unsigned int materialIndex = mesh->GetMaterialIndex();
        // BaseTexture* diffuse = mTextures[materialIndex];
        // if(diffuse)
        // {
        //     glActiveTexture(GL_TEXTURE0);
        //     glBindTexture(GL_TEXTURE_2D, diffuse->GetId());
        // }
        // else
        // {
        //     glBindTexture(GL_TEXTURE_2D, 0);
        // }

        mesh->Render();
    }
}