#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer {
public:
    static void Init() 
    {
        glEnable(GL_DEPTH_TEST);
    }

    static void Clear(float r, float g, float b, float a) 
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};