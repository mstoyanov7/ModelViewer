#include "core/OrbitCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

void OrbitCamera::updateView() 
{
    const float cy = glm::cos(yaw_), sy = glm::sin(yaw_);
    const float cp = glm::cos(pitch_), sp = glm::sin(pitch_);
    // spherical to Cartesian (right-handed)
    glm::vec3 dir(cp*cy, sp, cp*sy); // from target to eye (normalized)
    glm::vec3 eye = target_ + (-dir) * radius_;
    lookAt(eye, target_, {0,1,0});
}
