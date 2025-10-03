#include "app/ModelViewerApp.hpp"
#include "scenes/TriangleScene.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void ModelViewerApp::lazyInitIfNeeded() 
{
    if (sceneReady_) return;

    scene_ = std::make_unique<TriangleScene>();
    scene_->init();
    sceneReady_ = true;
    glEnable(GL_DEPTH_TEST);
}

void ModelViewerApp::OnUpdate(double dt) 
{
    accum_ += dt; frames_++;
    if (accum_ >= 0.3) 
    {
        char buf[128];
        const double fps = frames_ / accum_;
        snprintf(buf, sizeof(buf), "OpenGL — Triangle | %.1f FPS", fps);
        m_Window->SetTitle(buf);
        accum_ = 0.0; frames_ = 0;
    }
    lazyInitIfNeeded();
}

void ModelViewerApp::OnRender() 
{
    if (scene_) scene_->render();
}

void ModelViewerApp::OnResize(int w, int h) 
{
    (void)w; (void)h; // later: update projection matrices
}
