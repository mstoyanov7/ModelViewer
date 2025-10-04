#include "app/ModelViewerApp.hpp"

void ModelViewerApp::lazyInitIfNeeded() 
{
    if (m_Scene) return;

    m_Camera = std::make_unique<OrbitCamera>();
    m_Camera->setPerspective(glm::radians(60.0f), 1280.0f/720.0f, 0.1f, 100.0f);
    m_Camera->setTarget({0,0,0});
    m_Camera->setRadius(3.0f);
    m_Camera->setYawPitch(0.6f, 0.6f);

    m_Scene = std::make_unique<CubeScene>();
    m_Scene->init();

    glEnable(GL_DEPTH_TEST);
}

void ModelViewerApp::handleCameraInput(float dt) 
{
    (void)dt;
    // Drag to orbit (right mouse button)
    static bool dragging = false;
    static double lastX = 0, lastY = 0;
    double x, y; Input::GetMousePos(x, y);

    const bool rmb = Input::IsMousePressed(/*GLFW_MOUSE_BUTTON_RIGHT*/ 1);
    if (rmb && !dragging) { dragging = true; lastX = x; lastY = y; }
    if (!rmb && dragging) { dragging = false; }

    if (dragging && m_Camera) 
    {
        float dx = float(x - lastX);
        float dy = float(y - lastY);
        lastX = x; lastY = y;

        // sensitivity
        const float s = 0.005f;
        m_Camera->addYawPitch(-dx * s, -dy * s);
    }

    // Scroll to zoom
    double sx=0, sy=0; Input::ConsumeScroll(sx, sy);
    if (m_Camera && (sx != 0.0 || sy != 0.0)) 
    {
        m_Camera->addRadius(float(-sy * 0.2f)); // wheel up → zoom in
    }
}

void ModelViewerApp::OnUpdate(double dt) 
{
    accum_ += dt; frames_++;
    if (accum_ >= 0.3) 
    {
        char buf[128];
        const double fps = frames_ / accum_;
        snprintf(buf, sizeof(buf), "OpenGL — Cube (Orbit) | %.1f FPS", fps);
        m_Window->SetTitle(buf);
        accum_ = 0.0; frames_ = 0;
    }

    lazyInitIfNeeded();
    handleCameraInput(static_cast<float>(dt));

    if (m_Scene)
    {
        m_Scene->update(static_cast<float>(dt));
    } 
}

void ModelViewerApp::OnRender() 
{
    glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_Scene && m_Camera)
    {
        m_Scene->render(*m_Camera);
    } 
}

void ModelViewerApp::OnResize(int w, int h) 
{
    if (w <= 0 || h <= 0) return;
    glViewport(0, 0, w, h);
    if (m_Camera) 
    {
        m_Camera->setAspect(float(w) / float(h));
    } 
}