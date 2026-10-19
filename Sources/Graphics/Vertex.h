#include "glad/glad.h"

#include <glm/glm.hpp>

namespace Graphics
{

struct Vertex
{
    glm::vec3 mPos {0.f};
    glm::vec3 mNormal {0.f};
    glm::vec2 mTexCoord {0.f};
    glm::vec3 mTangent {0.f};
    glm::vec3 mBitangent {0.f};
    
    static constexpr size_t mSize = 3 + 3 + 2 + 3 + 3;
};

}