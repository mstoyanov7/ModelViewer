#include "app/ModelViewerApp.hpp"

void ModelViewerApp::lazyInitIfNeeded() 
{
    if (m_Scene) return;
    m_Camera = std::make_unique<Camera>();

    m_Camera->setPerspective(glm::radians(60.0f), 1280.0f/720.0f, 0.1f, 100.0f);
    m_Camera->lookAt({2.5f,2.0f,2.5f}, {0,0,0}, {0,1,0});

    m_Scene = std::make_unique<CubeScene>();
    int fbw=0, fbh=0; m_Window->GetFramebufferSize(fbw, fbh);
    m_Scene->init(fbw, fbh);

    glEnable(GL_DEPTH_TEST);
}

void ModelViewerApp::OnUpdate(double dt) 
{
    m_Accum += dt; m_Frames++;
    if (m_Accum >= 0.3) 
    {
        char buf[128];
        const double fps = m_Frames / m_Accum;
        snprintf(buf, sizeof(buf), "OpenGL — Cube | %.1f FPS", fps);
        m_Window->SetTitle(buf);
        m_Accum = 0.0; m_Frames = 0;
    }

    lazyInitIfNeeded();

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
    if (w == 0 || h == 0) return;

    glViewport(0, 0, w, h);
    if (m_Camera) 
    {
        m_Camera->setAspect(static_cast<float>(w) / static_cast<float>(h));
    }
}
