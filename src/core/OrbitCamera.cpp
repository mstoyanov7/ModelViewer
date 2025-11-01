#include "core/OrbitCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

void OrbitCamera::updateView() {
    const float cy = glm::cos(yaw_), sy = glm::sin(yaw_);
    const float cp = glm::cos(pitch_), sp = glm::sin(pitch_);
    glm::vec3 dir(cp*cy, sp, cp*sy);           // target -> eye (negated below)
    glm::vec3 eye = target_ + (-dir) * radius_;
    lookAt(eye, target_, {0,1,0});
}

void OrbitCamera::pan(float dx, float dy) {
    // build camera basis from current yaw/pitch
    const float cy = glm::cos(yaw_), sy = glm::sin(yaw_);
    const float cp = glm::cos(pitch_), sp = glm::sin(pitch_);

    glm::vec3 forward = glm::normalize(glm::vec3(-cp*cy, -sp, -cp*sy)); // eye->target
    glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
    glm::vec3 up      = glm::normalize(glm::cross(right, forward));

    // scale pan with distance so it feels consistent
    float s = radius_ * 0.0015f;
    target_ += (-dx * s) * right + (dy * s) * up;
    updateView();
}