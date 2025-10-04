#include "core/Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

void Camera::setPerspective(float fovyRad, float aspect, float zNear, float zFar) 
{
    fovy_ = fovyRad; aspect_ = aspect; zNear_ = zNear; zFar_ = zFar;
    proj_ = glm::perspective(fovy_, aspect_, zNear_, zFar_);
}

void Camera::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) 
{
    view_ = glm::lookAt(eye, center, up);
}

void Camera::setAspect(float aspect) 
{
    aspect_ = aspect;
    proj_ = glm::perspective(fovy_, aspect_, zNear_, zFar_);
}