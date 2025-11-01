#pragma once

#include "core/Application.hpp"
#include "gfx/Renderer.hpp"
#include "gfx/GridAxes.hpp"
#include "core/Camera.hpp"
#include "core/OrbitCamera.hpp"
#include "core/Input.hpp"
#include "scenes/CubeScene.hpp"
#include "scenes/ModelScene.hpp"

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
    void handleToggles();

    std::unique_ptr<CubeScene>   scene_;
    std::unique_ptr<GridAxes>    grid_;
    std::unique_ptr<OrbitCamera> camera_;
    double accum_ = 0.0; int frames_ = 0;
    bool wireframe_ = false, cull_ = false;
    bool lighting_ = true;

    std::unique_ptr<ModelScene> modelScene_;
    bool showModel_ = false;
};
