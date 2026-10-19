#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Utils/GLBuffer.h"
#include "Vertex.h"
#include "glad/glad.h"

namespace Graphics
{

class GLVertexBuffer
{
private:
    Utils::GLBuffer handle_;
    
    std::vector<Vertex> mVertices;
public:
    ~GLVertexBuffer() = default;
    
    GLVertexBuffer(const std::vector<Vertex>& pVertices)
    : mVertices(pVertices), handle_(pVertices.data(), pVertices.size() * sizeof(Vertex)) {}

    inline GLuint GetId() const { return handle_.GetId(); }
    inline size_t GetCount() const { return mVertices.size(); }
};

class GLIndexBuffer
{
private:
    Utils::GLBuffer handle_;
    
    std::vector<std::uint32_t> mIndices;
public:
    ~GLIndexBuffer() = default;
    
    GLIndexBuffer(const std::vector<std::uint32_t>& pIndices)
    : mIndices(pIndices), handle_(pIndices.data(), pIndices.size() * sizeof(std::uint32_t)) {}
    
    inline GLuint GetId() const { return handle_.GetId(); }
    inline size_t GetCount() const { return mIndices.size(); }
};

class FMesh final
{
private:
    Utils::GLVertexArray handle_;
    GLVertexBuffer vbo_;
    GLIndexBuffer ibo_;
    
    std::uint8_t mMaterialIndex;
public:
    ~FMesh() = default;

    FMesh(const FMesh&) = delete;
    FMesh& operator=(const FMesh&) = delete;

    FMesh(const std::vector<Vertex> pV, const std::vector<std::uint32_t> pI, std::uint8_t pMaterialIndex);
    void Render() const;
    
    inline unsigned int GetMaterialIndex() const { return static_cast<unsigned int>(mMaterialIndex); }
    
    inline size_t GetVertexCount() const { return vbo_.GetCount(); }
    inline size_t GetIndicesCount() const { return ibo_.GetCount(); }
};

}

// struct AttributeDescriptor
// {
//     uint8_t  mCount; element count per single Attribute
//     uint16_t mAttributeIndex; layout location index
//     uint32_t mRelativeOffset; offset per Vertex where attr. begins
//     /* GLenum   mDataType; */
//     
//     AttributeDescriptor(uint8_t f, uint16_t s, uint32_t t = 0 /*, GLenum h = GL_FLOAT*/)
//     : mCount(f), mAttributeIndex(s), mRelativeOffset(t) {}
// };

// class VertexBuffer
// {
// private:
//     std::vector<AttributeDescriptor> mAttributeDescriptors;
//     std::vector<float> mData;
//     unsigned int mId {};
//     unsigned int mStride {};
//     
//     void CalculateStrideAndOffset()
//     {
//         for(auto& a : mAttributeDescriptors)
//         {
//             a.mRelativeOffset = mStride;
//             mStride += (a.mCount * sizeof(float));
//         }
//     }
//     
// public:
//     ~VertexBuffer() { glDeleteBuffers(1, &mId); }
//         
//     VertexBuffer(const VertexBuffer&) = default;
//     VertexBuffer& operator=(const VertexBuffer&) = default;
//     
//     VertexBuffer(VertexBuffer&&) = default;
//     VertexBuffer& operator=(VertexBuffer&&) = default;
//     
//     VertexBuffer(const std::vector<float>& pData, 
//         const std::vector<AttributeDescriptor>& pDescriptorVector)
//     : mData(pData), mAttributeDescriptors(pDescriptorVector)
//     { 
//         glCreateBuffers(1, &mId);
//         CalculateStrideAndOffset();
//         glNamedBufferStorage(mId, mData.size() * sizeof(float), mData.data(), 0);
//     }
//     
//     const std::vector<AttributeDescriptor>& GetDescriptorsVector() const { return mAttributeDescriptors; }
//     inline unsigned int GetBufferId() const { return mId; }
//     inline unsigned int GetStride() const { return mStride; }
//     inline unsigned int GetVectorElementCount() const { return static_cast<unsigned int>(mData.size()); }
// };


// class ElementBuffer
// {
// private:
//     GLuint mId {};
//     std::vector<unsigned int> mData;
// public:
//     ~ElementBuffer() { glDeleteBuffers(1, &mId); }
//     
//     ElementBuffer(const ElementBuffer&) = default;
//     ElementBuffer& operator=(const ElementBuffer&) = default;
//     ElementBuffer(ElementBuffer&&) = default;
//     ElementBuffer& operator=(ElementBuffer&&) = default;
//     
//     ElementBuffer(const std::vector<unsigned int>& pData) 
//     : mData(pData) 
//     {
//         glCreateBuffers(1, &mId);
//         glNamedBufferStorage(mId, mData.size() * sizeof(unsigned int), mData.data(), 0);
//     }
//     
//     inline unsigned int GetBufferId() const { return mId; }
//     inline unsigned int GetVectorElementCount() const { return static_cast<unsigned int>(mData.size()); }
// };

// class Mesh final
// {
// private:
//     GLuint mId {};
//     unsigned int mVertexCount {}, mElementCount {}, mMaterialIndex {};
//     VertexBuffer mVertexBuffer;
//     ElementBuffer mElementBuffer;
// public:
//     ~Mesh();
//     
//     Mesh(const Mesh&) = default;
//     Mesh& operator=(const Mesh&) = default;
//     Mesh(Mesh&&) = default;
//     Mesh& operator=(Mesh&&) = default;
//     
//     Mesh(
//         const std::vector<float>& pVertexData, 
//         const std::vector<AttributeDescriptor>& pDescriptorVector,
//         const std::vector<unsigned int>& pElementData,
//         const unsigned int pMaterialIndex = 0);

//     void Render() const;
//     
//     inline unsigned int GetMaterialIndex() const { return mMaterialIndex; }
// };



















