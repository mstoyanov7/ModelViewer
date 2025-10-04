#pragma once
#include "core/Camera.hpp"

class OrbitCamera : public Camera 
{
public:
    void setTarget(const glm::vec3& target) { target_ = target; }
    void setYawPitch(float yaw, float pitch) { yaw_ = yaw; pitch_ = pitch; clampPitch(); updateView(); }
    void setRadius(float r) { radius_ = r > 0.05f ? r : 0.05f; updateView(); }

    void addYawPitch(float dYaw, float dPitch) { yaw_ += dYaw; pitch_ += dPitch; clampPitch(); updateView(); }
    void addRadius(float d) { setRadius(radius_ + d); }

    const glm::vec3& target() const { return target_; }
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }
    float radius() const { return radius_; }

private:
    void clampPitch() { if (pitch_ >  1.55f) pitch_ =  1.55f; if (pitch_ < -1.55f) pitch_ = -1.55f; }
    void updateView();

    glm::vec3 target_{0.0f, 0.0f, 0.0f};
    float yaw_ = 0.6f;
    float pitch_ = 0.6f;
    float radius_ = 3.0f;
};
