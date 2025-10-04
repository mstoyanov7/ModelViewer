#pragma once
#include <unordered_map>

class Input 
{
public:
    // keyboard
    static void SetKeyState(int key, bool pressed);
    static bool IsKeyPressed(int key);

    // mouse buttons
    static void SetMouseButton(int button, bool pressed);
    static bool IsMousePressed(int button);

    // mouse position (pixels, framebuffer coordinates)
    static void SetMousePos(double x, double y);
    static void GetMousePos(double& x, double& y);

    // scroll (delta since last poll)
    static void AddScroll(double dx, double dy);
    static void ConsumeScroll(double& dx, double& dy); // returns and clears

private:
    static inline std::unordered_map<int, bool> s_Key{};
    static inline std::unordered_map<int, bool> s_Mouse{};
    static inline double s_MouseX = 0.0, s_MouseY = 0.0;
    static inline double s_ScrollX = 0.0, s_ScrollY = 0.0;
};