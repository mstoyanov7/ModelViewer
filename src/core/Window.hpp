#pragma once
#include <string>

struct WindowProps {
    int width = 1280;
    int height = 720;
    std::string title = "ModelViewer";
    bool vsync = true;
};

class IWindow {
public:
    virtual ~IWindow() = default;

    virtual void PollEvents() = 0;
    virtual void SwapBuffers() = 0;
    virtual bool ShouldClose() const = 0;

    virtual void SetTitle(const std::string& title) = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;

    virtual void GetFramebufferSize(int& w, int& h) const = 0;
};