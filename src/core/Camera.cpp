#include "core/Camera.hpp"

void Camera::setPerspective(float fovyRadians, float aspect, float zNear, float zFar) 
{
    fovy_ = fovyRadians; aspect_ = aspect; zNear_ = zNear; zFar_ = zFar;
    proj_ = glm::perspective(fovy_, aspect_, zNear_, zFar_);
}

void Camera::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) 
{
    view_ = glm::lookAt(eye, center, up);
}