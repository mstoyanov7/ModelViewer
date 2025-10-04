#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Camera {
public:
    void setPerspective(float fovyRad, float aspect, float zNear, float zFar);
    void lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

    const glm::mat4& view() const { return view_; }
    const glm::mat4& proj() const { return proj_; }
    void setAspect(float aspect);

protected:
    glm::mat4 view_{1.0f};
    glm::mat4 proj_{1.0f};
    float fovy_ = 1.0471975512f; // 60° in radians
    float aspect_ = 16.0f/9.0f;
    float zNear_ = 0.1f;
    float zFar_  = 100.0f;
};