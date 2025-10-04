#include "core/Input.hpp"

void Input::SetKeyState(int key, bool pressed) 
{ 
    s_Key[key] = pressed; 
}

bool Input::IsKeyPressed(int key) 
{
    auto it = s_Key.find(key);
    return it != s_Key.end() && it->second;
}

void Input::SetMouseButton(int button, bool pressed) 
{ 
    s_Mouse[button] = pressed; 
}

bool Input::IsMousePressed(int button) 
{
    auto it = s_Mouse.find(button); 
    return it != s_Mouse.end() && it->second;
}

void Input::SetMousePos(double x, double y) 
{ 
    s_MouseX = x; s_MouseY = y; 
}

void Input::GetMousePos(double& x, double& y) 
{ 
    x = s_MouseX; y = s_MouseY; 
}

void Input::AddScroll(double dx, double dy) 
{ 
    s_ScrollX += dx; s_ScrollY += dy; 
}

void Input::ConsumeScroll(double& dx, double& dy) 
{ 
    dx = s_ScrollX; 
    dy = s_ScrollY; 
    s_ScrollX = s_ScrollY = 0.0; 
}
