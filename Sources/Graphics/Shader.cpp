#include "Shader.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ios>
#include <filesystem>
#include <print>

using namespace Graphics;

Shader::Shader(const char* pVertexSrc, const char* pFragSrc)
{
    std::println("Current path: {}", std::filesystem::current_path().string());
    const char* v_src = Read(pVertexSrc);
    const char* f_src = Read(pFragSrc);
    
    if(!v_src || !f_src)
        throw std::runtime_error("Failed to read shader source file(s)");

    GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);

    Compile(vertex_id, v_src);
    Compile(frag_id, f_src);

    if(mProgramId = glCreateProgram(); mProgramId == 0)
    {
        std::cout << "Failed to glCreateProgram()!\n";
    }

    glAttachShader(mProgramId, vertex_id);
    glAttachShader(mProgramId, frag_id);

    Link();
}

char* Shader::Read(const char* pSrc)
{
    std::ifstream file(pSrc, std::ios::binary | std::ios::ate);
    if(!file)
    {
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    char* buffer = new char[size + 1];
    if(!file.read(buffer, size))
    {
        delete[] buffer;
        return nullptr;
    }

    buffer[size] = '\0';
    return buffer;

}

void Shader::Compile(GLuint pId, const char* pSrc)
{
    GLint result;
    GLint log_len;
    glShaderSource(pId, 1, &pSrc, NULL);
    glCompileShader(pId);
    glGetShaderiv(pId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(pId, GL_INFO_LOG_LENGTH, &log_len);
    if(log_len > 0)
    {
        std::string info_log(log_len, '\0');
        glGetShaderInfoLog(pId, log_len, nullptr, info_log.data());
        std::cout << "Shader compile log: " << info_log << "\n";
    }
    if(result == GL_FALSE)
        std::cout << "Shader Compilation Failed!\n";
}

void Shader::Link()
{
    glLinkProgram(mProgramId);

    GLint result;
    GLint log_len;
    glGetProgramiv(mProgramId, GL_LINK_STATUS, &result);
    glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &log_len);
    if(log_len > 0)
    {
        GLchar* info_log = nullptr;
        glGetProgramInfoLog(mProgramId, log_len, nullptr, info_log);
        std::cout << "Shader Linking Failed!\n";
    }
}

void Shader::Bind() const
{
    glUseProgram(mProgramId);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

GLuint Shader::GetUniformLocation(const std::string& pName)
{
    if(mCachedLocations.find(pName) != mCachedLocations.end())
        return mCachedLocations[pName];

    GLuint loc = glGetUniformLocation(mProgramId, pName.c_str());
    mCachedLocations[pName] = loc;
    return loc;
}

void Shader::Vec3Uniform(const std::string& pName, glm::vec3 pValue)
{
    GLuint loc = GetUniformLocation(pName);
    glUniform3f(loc, pValue.x, pValue.y, pValue.z);
}

void Shader::Mat4Uniform(const std::string& pName, glm::mat4 pValue)
{
    GLuint loc = GetUniformLocation(pName);
    glUniformMatrix4fv(loc, 1, GL_FALSE, &pValue[0][0]);
}

void Shader::IntUniform(const std::string& pName, int pValue)
{
    GLuint loc = GetUniformLocation(pName);
    glUniform1i(loc, pValue);
}

void Shader::FloatUniform(const std::string& pName, float pValue)
{
    GLuint loc = GetUniformLocation(pName);
    glUniform1f(loc, pValue);
}

unsigned int Shader::GetId() const { return mProgramId; }