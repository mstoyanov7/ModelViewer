#include "core/Application.hpp"

Application::Application(std::unique_ptr<IWindow> window)
    : m_Window(std::move(window)) {}

int Application::Run()
{
    Timer timer;

    glEnable(GL_DEPTH_TEST);

    int fbw = 0, fbh = 0;
    m_Window->GetFramebufferSize(fbw, fbh);
    OnResize(fbw, fbh);

    while (!m_Window->ShouldClose())
    {
        double dt = timer.Tick();

        int w = 0, h = 0;
        m_Window->GetFramebufferSize(w, h);
        
        if (w != fbw || h != fbh) 
        {
            fbw = w; fbh = h;
            glViewport(0, 0, fbw, fbh);
            OnResize(fbw, fbh);
        }

        OnUpdate(dt);

        OnRender();

        m_Window->SwapBuffers();
        m_Window->PollEvents();
    }

    return 0;
}