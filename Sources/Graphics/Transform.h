#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Graphics
{

struct Transform
{
    glm::vec3 mPos {0.f};
    glm::vec3 mRot {0.f};
    float mScale {1.f};
    
    [[nodiscard]] glm::mat4 GetModelMatrix() const
    {
        return
        glm::translate(glm::mat4(1.f), mPos) * 
        glm::eulerAngleYXZ(mRot.x, mRot.y, mRot.z) * 
        glm::scale(glm::mat4(1.f), glm::vec3(mScale));
    }
};

}