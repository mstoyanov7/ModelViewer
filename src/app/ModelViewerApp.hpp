#pragma once
#include "core/Application.hpp"
#include "scenes/TriangleScene.hpp" 

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

    std::unique_ptr<TriangleScene> scene_;
    double accum_ = 0.0;
    int    frames_ = 0;
    bool   sceneReady_ = false;
};
