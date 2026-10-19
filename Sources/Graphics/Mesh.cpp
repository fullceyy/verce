#include "Mesh.h"
// #include <print>

using namespace Graphics;

FMesh::FMesh(const std::vector<Vertex> pV, const std::vector<std::uint32_t> pI, std::uint8_t pMaterialIndex)
: handle_(1), vbo_(pV), ibo_(pI), mMaterialIndex(pMaterialIndex)
{
    GLuint id = handle_.GetId();
    glVertexArrayVertexBuffer(id, 0, vbo_.GetId(), 0, sizeof(Vertex));

    unsigned int attr_and_bind_idx {0};
    // Pos
    glVertexArrayAttribFormat(id, attr_and_bind_idx, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(id, attr_and_bind_idx, 0);
    glEnableVertexArrayAttrib(id, attr_and_bind_idx);
    
    attr_and_bind_idx += 1;
    // Normal
    glVertexArrayAttribFormat(id, attr_and_bind_idx, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, mNormal));
    glVertexArrayAttribBinding(id, attr_and_bind_idx, 0);
    glEnableVertexArrayAttrib(id, attr_and_bind_idx);
    
    attr_and_bind_idx += 1;
    // UV
    glVertexArrayAttribFormat(id, attr_and_bind_idx, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, mTexCoord));
    glVertexArrayAttribBinding(id, attr_and_bind_idx, 0);
    glEnableVertexArrayAttrib(id, attr_and_bind_idx);
    
    attr_and_bind_idx += 1;
    // Tangent
    glVertexArrayAttribFormat(id, attr_and_bind_idx, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, mTangent));
    glVertexArrayAttribBinding(id, attr_and_bind_idx, 0);
    glEnableVertexArrayAttrib(id, attr_and_bind_idx);
    
    attr_and_bind_idx += 1;
    // Bitangent
    glVertexArrayAttribFormat(id, attr_and_bind_idx, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, mBitangent));
    glVertexArrayAttribBinding(id, attr_and_bind_idx, 0);
    glEnableVertexArrayAttrib(id, attr_and_bind_idx);
    
    glVertexArrayElementBuffer(id, ibo_.GetId());
}

void FMesh::Render() const
{
    // std::println(
    //     "VAO={}, VBO={}, EBO={}, vertices={}, indices={}",
    //     handle_.GetId(),
    //     vbo_.GetId(),
    //     ibo_.GetId(),
    //     vbo_.GetCount(),
    //     ibo_.GetCount()
    // );

    glBindVertexArray(handle_.GetId());
    
    if(ibo_.GetCount() > 0)
        glDrawElements(GL_TRIANGLES, ibo_.GetCount(), GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(GL_TRIANGLES, 0, vbo_.GetCount());
    
    glBindVertexArray(0);
}