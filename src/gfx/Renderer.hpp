#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer {
public:
    static void Init() 
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_DEPTH_CLAMP);
        glDepthFunc(GL_LEQUAL);          
        glClearDepth(1.0);              // default, but be explicit
    }

    static void Clear(float r, float g, float b, float a) 
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    static void SetWireframe(bool on) 
    {
        glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL);
    }

    static void SetCull(bool on) 
    {
        if (on) 
        { 
            glEnable(GL_CULL_FACE); 
            glCullFace(GL_BACK); 
            glFrontFace(GL_CCW); 
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }    
    }
};