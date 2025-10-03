#include "core/Application.hpp"

#if defined(_WIN32)
// MSVC/Win32: gl.h depends on <windows.h> for WINGDIAPI/APIENTRY
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif


#include <cstdio>


Application::Application(std::unique_ptr<IWindow> window)
    : m_Window(std::move(window)) {}


int Application::Run() 
{
    Timer timer;

    int fbw = 0, fbh = 0;
    m_Window->GetFramebufferSize(fbw, fbh);
    OnResize(fbw, fbh);

    while (!m_Window->ShouldClose()) 
    {
        double dt = timer.Tick();
        (void)dt; // not used yet

        // Handle resize polling here (no event bus yet)
        int w = 0, h = 0;
        m_Window->GetFramebufferSize(w, h);
        if (w != fbw || h != fbh) 
        {
            fbw = w; fbh = h;
            glViewport(0, 0, fbw, fbh);
            OnResize(fbw, fbh);
        }

        // Update
        OnUpdate(dt);

        // Basic clear pass
        glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render hook (no GL resources yet)
        OnRender();

        m_Window->SwapBuffers();
        m_Window->PollEvents();
    }

    return 0;
}
