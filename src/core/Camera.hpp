#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera 
{
public:
    void setPerspective(float fovyRadians, float aspect, float zNear, float zFar);
    void lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

    const glm::mat4& view() const { return view_; }
    const glm::mat4& proj() const { return proj_; }

    void setAspect(float aspect) 
    { 
        aspect_ = aspect; setPerspective(fovy_, aspect_, zNear_, zFar_);
    }

private:
    glm::mat4 view_{1.0f};
    glm::mat4 proj_{1.0f};
    float fovy_ = 1.0f;   // radians
    float aspect_ = 16.0f / 9.0f;
    float zNear_ = 0.1f;
    float zFar_  = 100.0f;
};