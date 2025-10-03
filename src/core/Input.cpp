#include "Input.hpp"
#include <GLFW/glfw3.h>
#include "GlfwWindow.hpp"

extern GLFWwindow* g_MainWindow;

bool Input::IsKeyPressed(int keycode) 
{
    auto state = glfwGetKey(g_MainWindow, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}