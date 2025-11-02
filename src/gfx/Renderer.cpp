#include "gfx/Renderer.hpp"

void Renderer::Init() 
{
    glEnable(GL_DEPTH_TEST);
    // Depth clamp can produce artifacts; keep it off for better depth behavior
    glDisable(GL_DEPTH_CLAMP);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0);              // default, but be explicit
    // Enable multisampling (MSAA must be requested at window creation)
    glEnable(GL_MULTISAMPLE);
}

void Renderer::Clear(float r, float g, float b, float a) 
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SetWireframe(bool on) 
{
    glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL);
}

void Renderer::SetCull(bool on) 
{
    if (on) 
    { 
        glEnable(GL_CULL_FACE); 
        glCullFace(GL_FRONT); 
        glFrontFace(GL_CCW); 
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }    
}
