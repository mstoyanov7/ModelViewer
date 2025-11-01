#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer {
public:
    static void Init();
    static void Clear(float r, float g, float b, float a);
    static void SetWireframe(bool on);
    static void SetCull(bool on);
};