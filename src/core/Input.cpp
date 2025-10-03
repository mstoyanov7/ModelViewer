// Input.cpp
#include "core/Input.hpp"
#include <GLFW/glfw3.h>

std::unordered_map<int,bool> Input::s_KeyState;

bool Input::IsKeyPressed(int key) 
{
    auto it = s_KeyState.find(key);
    return it != s_KeyState.end() && it->second;
}

void Input::SetKeyState(int key, bool pressed) 
{
    s_KeyState[key] = pressed;
}