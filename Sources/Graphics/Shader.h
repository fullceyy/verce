#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <string>
#include <unordered_map>

namespace Graphics
{

class Shader
{
private:
    unsigned int mProgramId {};
    std::unordered_map<std::string, GLuint> mCachedLocations;
public:
    Shader(const char* pVertexSrc, const char* pFragSrc);
    ~Shader() = default;

    char* Read(const char* pSrc);
    void Compile(GLuint pId, const char* pSrc);
    void Link();

    void Bind() const;
    void Unbind() const;

    GLuint GetUniformLocation(const std::string& pName);
    void Vec3Uniform(const std::string& pName, glm::vec3 pValue);
    void Mat4Uniform(const std::string& pName, glm::mat4 pValue);
    void IntUniform(const std::string& pName, int pValue);
    void FloatUniform(const std::string& pName, float pValue);

    inline unsigned int GetId() const;
};


}