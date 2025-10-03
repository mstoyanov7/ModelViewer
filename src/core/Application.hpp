#pragma once
#include "core/Window.hpp"
#include "core/Timer.hpp"
#include <memory>


class Application 
{
public:
    explicit Application(std::unique_ptr<IWindow> window);
    virtual ~Application() = default;


    int Run();

protected:
    // Hooks for your app/game logic — override in derived class later if needed
    virtual void OnUpdate(double /*dt*/) {}
    virtual void OnRender() {}
    virtual void OnResize(int /*w*/, int /*h*/) {}

protected:
    std::unique_ptr<IWindow> m_Window;
};