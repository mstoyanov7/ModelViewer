#include "platform/glfw/GlfwWindow.hpp"
#include "app/ModelViewerApp.hpp"

int main() 
{
    WindowProps props;
    props.title  = "OpenGL — Rotating Cube";
    props.width  = 1280;
    props.height = 720;
    props.vsync  = true;

    auto window = std::make_unique<GlfwWindow>(props);
    ModelViewerApp app(std::move(window));
    return app.Run();
}