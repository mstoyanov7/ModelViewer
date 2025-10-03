#include "core/Application.hpp"
#include "platform/glfw/GlfwWindow.hpp"
#include <memory>


class MyApp : public Application 
{
public:
    using Application::Application; // inherit ctor

protected:
    void OnUpdate(double dt) override 
    {
        // Example: animate window title with FPS every ~0.3s
        m_accum += dt; m_frames++;
        if (m_accum >= 0.3) 
        {
            double fps = m_frames / m_accum;
            char buf[128];
            snprintf(buf, sizeof(buf), "ModelViewer | %.1f FPS", fps);
            m_Window->SetTitle(buf);
            m_accum = 0.0; m_frames = 0;
        }
    }


    void OnResize(int w, int h) override 
    {
        // Here we could update a projection matrix later
        (void)w; (void)h;
    }

private:
    double m_accum = 0.0;
    int m_frames = 0;
};


int main() 
{
    WindowProps props;
    props.title = "ModelViewer";
    props.width = 1280; 
    props.height = 720;
    props.vsync = false;

    auto window = std::make_unique<GlfwWindow>(props);
    MyApp app(std::move(window));
    return app.Run();
}