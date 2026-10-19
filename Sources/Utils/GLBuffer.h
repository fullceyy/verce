#pragma once

#include <utility>

#include "glad/glad.h"

namespace Utils
{

class GLBuffer
{
private:
    GLuint id_;
public:
    GLBuffer() = default;
    GLBuffer(const void* pData, std::size_t pBytes, GLbitfield pFlags = 0)
    {
        glCreateBuffers(1, &id_);
        glNamedBufferStorage(id_, (GLsizeiptr)pBytes, pData, pFlags);
    }

    ~GLBuffer() { glDeleteBuffers(1, &id_); }
    
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;
    GLBuffer(GLBuffer&& other) noexcept : id_(std::exchange(other.id_, 0)) {}
    GLBuffer& operator=(GLBuffer&& other) noexcept
    {
        if(this != &other)
        {
            glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }
    
    [[nodiscard]] GLuint GetId() const { return id_; }
};

class GLVertexArray
{
private:
    GLuint id_;
public:
    GLVertexArray() = default;
    
    GLVertexArray(bool)
    {
        glCreateVertexArrays(1, &id_);
    }
    
    GLVertexArray(GLuint pVboId, std::size_t pStride)
    {
        glCreateVertexArrays(1, &id_);
        glVertexArrayVertexBuffer(id_, 0, pVboId, 0, pStride);
    }
    
    ~GLVertexArray() { glDeleteVertexArrays(1, &id_); }
    
    GLVertexArray(const GLVertexArray&) = delete;
    GLVertexArray& operator=(const GLVertexArray&) = delete;
    GLVertexArray(GLVertexArray&& other) : id_(std::exchange(other.id_, 0)) {}
    GLVertexArray& operator=(GLVertexArray&& other) noexcept
    {
        if(this != &other)
        {
            glDeleteVertexArrays(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }
    
    [[nodiscard]] GLuint GetId() const { return id_; }
};

class GLUniformBuffer
{
private:
    GLuint id_;
public:
    GLUniformBuffer() = default;
    
    GLUniformBuffer(bool)
    {
        glCreateBuffers(1, &id_);
    }
    
    GLUniformBuffer(GLsizeiptr pSize, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT)
    {
        glCreateBuffers(1, &id_);
        glNamedBufferStorage(id_, pSize, nullptr, flags);
    }
    
    GLUniformBuffer(const GLUniformBuffer&) = delete;
    GLUniformBuffer& operator=(const GLUniformBuffer&) = delete;
    GLUniformBuffer(GLUniformBuffer&& other) : id_(std::exchange(other.id_, 0)) {}
    GLUniformBuffer& operator=(GLUniformBuffer&& other) noexcept
    {
        if(this != &other)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
            glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }
    
    [[nodiscard]] GLuint GetId() const { return id_; }
    
    void Upload(GLintptr pOffset, GLsizeiptr pSize, const void* pData) const  { glNamedBufferSubData(id_, pOffset, pSize, pData); CHECKGLERROR; }
};

}