#pragma once
#include <unordered_map>

class Input 
{
public:
    static bool IsKeyPressed(int key);
    static void SetKeyState(int key, bool pressed);

private:
    static std::unordered_map<int, bool> s_KeyState;
};