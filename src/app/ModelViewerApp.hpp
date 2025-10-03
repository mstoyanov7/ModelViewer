#pragma once

#include "core/Application.hpp"
#include "gfx/Renderer.hpp"
#include "core/Camera.hpp"
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

    std::unique_ptr<CubeScene> m_Scene;
    std::unique_ptr<Camera> m_Camera;
    double m_Accum = 0.0;
    int m_Frames = 0;
    bool m_IsSceneReady = false;
};
