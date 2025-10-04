#pragma once

#include "core/Application.hpp"
#include "gfx/Renderer.hpp"
#include "core/Camera.hpp"
#include "core/OrbitCamera.hpp"
#include "core/Input.hpp"
#include "scenes/CubeScene.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/trigonometric.hpp>  

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

class ModelViewerApp : public Application 
{
public:
    using Application::Application;
    ~ModelViewerApp() = default;

protected:
    void OnUpdate(double dt) override;
    void OnRender() override;
    void OnResize(int w, int h) override;

private:
    void lazyInitIfNeeded();
    void handleCameraInput(float dt);

    std::unique_ptr<CubeScene> m_Scene;
    std::unique_ptr<OrbitCamera> m_Camera;
    double accum_ = 0.0;
    int    frames_ = 0;
};
