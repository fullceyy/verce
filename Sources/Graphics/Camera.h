#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <algorithm>
/*
FinalCamera class will represent out main API to work with the 3D Camera.
The class itself stores the reference to an instance of the CameraPositionerInterace
class, which will represent a polymorphic implementation of the underlying camera model.

The interface of CameraPositionerInterace contains only pure virtual methods &
default virtual destructor.
*/
namespace Graphics
{

class CameraPositionerInterace
{
public:
    virtual ~CameraPositionerInterace() = default;
    virtual glm::mat4 GetViewMatrix() const = 0;
    virtual glm::vec3 GetPosition() const = 0;
};

class FinalCamera final
{
public:
    explicit FinalCamera(CameraPositionerInterace &pCameraPositioner)
    : mPositioner(pCameraPositioner)
    {
    }
    
    glm::mat4 GetViewMatrix() const { return mPositioner.GetViewMatrix(); }
    glm::vec3 GetPosition() const { return mPositioner.GetPosition(); }
private:
    CameraPositionerInterace& mPositioner;
};

class CameraPositioner_FirstPerson final : public CameraPositionerInterace
{
public:
    struct Movement
    {
        bool mFrw = false; 
        bool mBkw = false;
        bool mLeft = false;
        bool mRight = false;
        bool mUp = false;
        bool mDown = false;
        bool mFast = false;
    } movement_; // these define current motion state of our camera via bool flags.
    // This is useful to decouple camera logic from keyboard input.

    float mMouseSpeed = 4.f;
    float mAcceleration = 150.f;
    float mDamping = .2f;
    float mMaxSpeed = 10.f;
    float mFastCoef = 10.f;
    
private:
    glm::vec2 mMousePos = glm::vec2(0);
    glm::vec3 mCameraPosition = glm::vec3(0.f, 10.f, 10.f);
    // Quaternion contains four points of data
    // w, x, y, z & represents a rotation in 3D space
    // *** Avoids gimbal lock associated with Euler angles
    // *** Compact representation of rotation in 3D space
    glm::quat mCameraOrientation = glm::quat(glm::vec3(0)); // constructs to 1, 0, 0, 0
    glm::vec3 mMoveSpeed = glm::vec3(0);
public:
    CameraPositioner_FirstPerson() = default;

    // This constructor takes arguments that are normally used for glm::lookAt()
    // Camera's initial position, target position & vector pointing upward. 
    CameraPositioner_FirstPerson(const glm::vec3& pPos, const glm::vec3& pTarget, const glm::vec3& pUp)
    : mCameraPosition(pPos), mCameraOrientation(glm::lookAt(pPos, pTarget, pUp)) {}

    // Update() method should be called at every frame(inside of Run Loop).
    // & take the time elapsed since the prev. time (deltaTime),
    // as well as the mouse pos & mouse btn pressed bool flag.
    // In case when the mouse button is pressed, we calculate a Delta value
    // versus the prev. mouse position(mMousePos) & use it to construct a
    // rotation quaternion. This quat is then used to rotate a camera.
    // Once the camera position is applied, we should updated the mouse
    // position state.
    void Update(double pDeltaSeconds, const glm::vec2& pMousePos, bool pMousePressed)
    {
        if(pMousePressed)
        {
            const glm::vec2 delta = pMousePos - mMousePos; // how far the mouse moved since last frame
            // delta.y (mouse y pos) goes into x slot(pitch(rotation around X))
            // delta.x (mouse x pos) goes into y slot(yaw(rotation around Y))
            // 0.f is Z value (Roll) FPS camera does not roll sideways anyway
            const glm::quat deltaQuat = 
            glm::quat(glm::vec3(mMouseSpeed * delta.y, mMouseSpeed * delta.x, 0.f));

            // Now we apply mouse rotation to an existing camera orientation
            // which is then used in ...
            // glm::normalize() is used since quaternions must stay unit length
            // to represent pure rotation
            mCameraOrientation = glm::normalize(deltaQuat * mCameraOrientation);
        }
        mMousePos = pMousePos;

        // This converts the quaternion into an equivalent 4x4 rotation matrix
        // pure ROTATION, NO translation applied.
        // mCameraOrientation is cameras basis vectors expressed in world space
        const glm::mat4 viewMat = glm::mat4_cast(mCameraOrientation);
        // After the application of mCameraOrientation, which is pure rotation applied
        // to the camera, we extract NOW ROTATED cameras forward and right direction vectors(?)
        // AI suggestion of wording:
        /*Extracted lForward and lRight are world-space direction vectors telling you where, 
        in the world's coordinate system, the camera's forward and right currently point.*/
        const glm::vec3 lForward = -glm::vec3(viewMat[0][2], viewMat[1][2], viewMat[2][2]);
        const glm::vec3 lRight = glm::vec3(viewMat[0][0], viewMat[1][0], viewMat[2][0]);
        const glm::vec3 lUp = glm::cross(lRight, lForward); // cross product, perp. to two vectors

        // Currently at this stage of the code accel represents only a direction vector
        // Since the extracted lForward & lRight are normalized values (0, 1)
        glm::vec3 accel(0.f);
        if(movement_.mFrw) accel += lForward;
        if(movement_.mBkw) accel -= lForward;
        
        if(movement_.mRight) accel += lRight;
        if(movement_.mLeft) accel -= lRight;

        if(movement_.mUp) accel += lUp;
        if(movement_.mDown) accel -= lUp;

        if(movement_.mFast) accel *= mFastCoef;

        // friction/spring like model:
        // if accel is 0 then mMoveSpeed decays exponentially toward zero,
        // clamping via std::min ensures the speed does not overshoot backwards
        // makes decceleration feel smooth, without instantly stopping

        // if accel is NOT 0, then mMoveSpeed accumulates the direction of accel,
        // scaled by mAcceleration & deltaTime so speed ramps up smoothly rather than
        // snapping to max speed
        
        // mMoveSpeed is actually denormalized vector, meaning it both holds the
        // DIRECTION & MAGNITUDE(units traveled in that direction).
        if(accel == glm::vec3(0))
        {
            mMoveSpeed -= mMoveSpeed * std::min((1.f / mDamping) * static_cast<float>(pDeltaSeconds), 1.f);
        } else {
            mMoveSpeed += accel * mAcceleration * static_cast<float>(pDeltaSeconds);
            const float maxSpeed = movement_.mFast ? mMaxSpeed * mFastCoef : mMaxSpeed;
            if(glm::length(mMoveSpeed) > maxSpeed)
                mMoveSpeed = glm::normalize(mMoveSpeed) * maxSpeed;
        }
        // Standard Euler integration: 
        // position += velocity * dt. mMoveSpeed is being treated as a velocity vector 
        // (world-space units/second), applied over the elapsed frame time.
        mCameraPosition += mMoveSpeed * static_cast<float>(pDeltaSeconds);
    }

    // Since quaternion is used to describe cameras orientation,
    // we calculate view matrix from that same quat and camera position.
    // mCameraPosition is inversed for transposition and correct display of data
    // according to RHS coordinate system in OpenGL
    virtual glm::mat4 GetViewMatrix() const override 
    {
        const glm::mat4 t = glm::translate(glm::mat4(1.f), -mCameraPosition);
        const glm::mat4 r = glm::mat4_cast(mCameraOrientation);
        return r * t;
    }

    virtual glm::vec3 GetPosition() const override
    {
        return mCameraPosition;
    }
    
    void SetPosition(const glm::vec3& pNewPos)
    {
        mCameraPosition = pNewPos;
    }
    
    void ResetMouseRotation(const glm::vec2& pMouse)
    {
        mMousePos = pMouse;
    }
    
    void SetUpVector(const glm::vec3& pUp)
    {
        const auto view = GetViewMatrix();
        const glm::vec3 dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
        mCameraOrientation = glm::lookAt(mCameraPosition, mCameraPosition + dir, pUp);
    }
};

}