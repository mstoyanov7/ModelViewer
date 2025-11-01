#include "app/ModelViewerApp.hpp"

void ModelViewerApp::lazyInitIfNeeded() 
{
    if (scene_) return;

    Renderer::Init();

    camera_ = std::make_unique<OrbitCamera>();
    camera_->setPerspective(glm::radians(60.0f), 1280.0f/720.0f, 0.2f, 200.0f);
    camera_->setTarget({0,0,0}); 
    camera_->setRadius(4.0f); 
    camera_->setYawPitch(0.7f, 0.5f);

    grid_ = std::make_unique<GridAxes>(); 
    grid_->init(20, 1.0f);

    scene_ = std::make_unique<CubeScene>(); 
    scene_->init();

    // Prepare ModelScene but don't activate until user toggles
    if (!modelScene_) 
    {
        modelScene_ = std::make_unique<ModelScene>();
        // Change path if different:
        if (!modelScene_->init("assets/c63v2/scene.gltf")) 
        {
            // optional: printf error, keep cube as fallback
            printf("OBJ load error: %s\n", modelScene_->lastError().c_str());
            modelScene_.reset();
        } 
        else 
        {
            modelScene_->setLighting(true);
        }
    }
}

void ModelViewerApp::handleCameraInput(float) 
{
    // Orbit drag = Right Mouse
    static bool orbiting = false; 
    static double ox = 0, oy = 0;

    double x, y; 
    Input::GetMousePos(x, y);

    const bool rmb = Input::IsMousePressed(1); // GLFW_MOUSE_BUTT ON_RIGHT

    if (rmb && !orbiting) 
    { 
        orbiting=true; 
        ox=x; 
        oy=y; 
    }

    if (!rmb && orbiting)  
    { 
        orbiting = false; 
    }
    if (orbiting && camera_) 
    {
        float dx = float(x - ox), 
        dy = float(y - oy); 
        ox = x; 
        oy = y;
        // Make horizontal (yaw) drag direction intuitive: left drag -> rotate left
        camera_->addYawPitch(dx * 0.005f, -dy * 0.005f);
    }

    // Pan drag = Middle Mouse
    static bool panning = false; 
    static double px = 0, py = 0;

    const bool mmb = Input::IsMousePressed(2); // GLFW_MOUSE_BUTTON_MIDDLE

    if (mmb && !panning) 
    { 
        panning = true;
        px = x;
        py = y; 
    }

    if (!mmb && panning)  
    { 
        panning = false; 
    }

    if (panning && camera_) 
    {
        float dx = float(x - px); 
        float dy = float(y - py); 
        px=x; 
        py=y;
        camera_->pan(-dx, -dy);
    }

    // Scroll to zoom
    double sx = 0, sy = 0; 
    Input::ConsumeScroll(sx, sy);

    if (camera_ && sy != 0.0) 
    {
        camera_->addRadius(float(-sy * 0.25f));
    }
}

void ModelViewerApp::handleToggles() 
{
    // F = wireframe toggle
    static bool fPrev = false; 
    bool fNow = Input::IsKeyPressed(/*GLFW_KEY_F*/ 70);
    if (fNow && !fPrev) 
    { 
        wireframe_ = !wireframe_;
        Renderer::SetWireframe(wireframe_); 
    }
    fPrev = fNow;

    // C = culling toggle
    static bool cPrev = false; 
    bool cNow = Input::IsKeyPressed(/*GLFW_KEY_C*/ 67);
    if (cNow && !cPrev) 
    { 
        cull_ = !cull_; 
        Renderer::SetCull(cull_); 
    }
    cPrev = cNow;

    // R = reset camera
    static bool rPrev = false; 
    bool rNow = Input::IsKeyPressed(/*GLFW_KEY_R*/ 82);
    if (rNow && !rPrev && camera_) 
    {
        camera_->setTarget({0, 0, 0}); 
        camera_->setRadius(4.0f); 
        camera_->setYawPitch(0.7f, 0.5f);
    } 

    // L = lighting toggle
    static bool lPrev = false; 
    bool lNow = Input::IsKeyPressed(/*GLFW_KEY_L*/ 76);
    if (lNow && !lPrev) 
    {
        lighting_ = !lighting_;
        if (scene_)
        { 
            scene_->setLighting(lighting_);
        }
    }

    // M = toggle ModelScene/CubeScene
    static bool mPrev = false; 
    bool mNow = Input::IsKeyPressed(/*GLFW_KEY_M*/ 77);
    if (mNow && !mPrev) 
    {
        showModel_ = !showModel_;
        if (camera_) 
        {
            camera_->setTarget({0, 0, 0});
        }
    }
    rPrev = rNow;
}

void ModelViewerApp::OnUpdate(double dt) 
{
    accum_ += dt; 
    frames_++;

    if (accum_ >= 0.3) 
    {
        char buf[128]; const double fps = frames_ / accum_;

        snprintf(buf, sizeof(buf),
            "OpenGL — Cube (Orbit) | %.1f FPS  [%s%s%s]",
            fps,
            wireframe_ ? "WF " : "",
            cull_      ? "Cull " : "",
            lighting_  ? "Light" : "NoLight");

        m_Window->SetTitle(buf); 
        accum_ = 0.0; 
        frames_ = 0;
    }

    lazyInitIfNeeded();
    handleCameraInput((float)dt);
    handleToggles();

    if (scene_) 
    {
        scene_->update((float)dt);
    }
}

void ModelViewerApp::OnRender() 
{
    Renderer::Clear(0.07f, 0.08f, 0.10f, 1.0f);
    if (grid_ && camera_) 
    {
        grid_->render(*camera_);
    }

    if (showModel_) 
    {
        if (modelScene_ && camera_) 
        {
            modelScene_->render(*camera_);
        }
    } 
    else 
    {
        if (scene_ && camera_) 
        {
            scene_->render(*camera_);
        }
    }
}

void ModelViewerApp::OnResize(int w, int h) 
{
    if (w <= 0 || h <= 0) return;
    
    glViewport(0, 0, w, h);
    if (camera_) 
    {
        camera_->setAspect(float(w) / float(h));
    } 
}
