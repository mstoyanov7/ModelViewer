#include "app/ModelViewerApp.hpp"
#include <filesystem>
#include <cctype>

void ModelViewerApp::lazyInitIfNeeded() 
{
    if (scene_) return;

    Renderer::Init();

    camera_ = std::make_unique<OrbitCamera>();
    camera_->setPerspective(glm::radians(60.0f), 1280.0f/720.0f, 0.2f, 200.0f);
    camera_->setTarget({0,0,0}); 
    camera_->setRadius(4.0f); 
    camera_->setYawPitch(0.7f, -0.5f);

    grid_ = std::make_unique<GridAxes>(); 
    grid_->init(20, 1.0f);

    overlay_.init();

    scene_ = std::make_unique<CubeScene>(); 
    scene_->init();

    // Prepare ModelScene and discover available models
    if (!modelScene_) 
    {
        modelScene_ = std::make_unique<ModelScene>();
        modelScene_->setLighting(true);
    }
    scanModels();
    if (!modelPaths_.empty())
    {
        currentModelIndex_ = 0;
        if (!modelScene_->init(modelPaths_[currentModelIndex_]))
        {
            printf("Model load error: %s\n", modelScene_->lastError().c_str());
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
        if (scene_) { scene_->setLighting(lighting_); }
        if (modelScene_) { modelScene_->setLighting(lighting_); }
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

    // H = help toggle
    static bool hPrev = false;
    bool hNow = Input::IsKeyPressed(/*GLFW_KEY_H*/ 72);
    if (hNow && !hPrev) { showHelp_ = !showHelp_; }
    hPrev = hNow;

    // Arrow keys: Left/Right to cycle models when in Model mode
    static bool leftPrev = false, rightPrev = false;
    bool leftNow = Input::IsKeyPressed(263);  // GLFW_KEY_LEFT
    bool rightNow = Input::IsKeyPressed(262); // GLFW_KEY_RIGHT
    if (showModel_ && !modelPaths_.empty() && modelScene_)
    {
        if (leftNow && !leftPrev) { switchModel(-1); }
        if (rightNow && !rightPrev) { switchModel(+1); }
    }
    leftPrev = leftNow;
    rightPrev = rightNow;
}

void ModelViewerApp::OnUpdate(double dt) 
{
    accum_ += dt; 
    frames_++;

    if (accum_ >= 0.3) 
    {
        char buf[128]; const double fps = frames_ / accum_;

        if (showModel_ && currentModelIndex_ >= 0 && currentModelIndex_ < (int)modelPaths_.size())
        {
            // Show current model file name
            const char* file = modelPaths_[currentModelIndex_].c_str();
            snprintf(buf, sizeof(buf),
                "OpenGL — Model | %.1f FPS [%s%s%s] — %s",
                fps,
                wireframe_ ? "WF " : "",
                cull_      ? "Cull " : "",
                lighting_  ? "Light" : "NoLight",
                file);
        }
        else
        {
            snprintf(buf, sizeof(buf),
                "OpenGL — Cube (Orbit) | %.1f FPS  [%s%s%s]",
                fps,
                wireframe_ ? "WF " : "",
                cull_      ? "Cull " : "",
                lighting_  ? "Light" : "NoLight");
        }

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
    // Lighter background (soft light gray)
    Renderer::Clear(0.92f, 0.93f, 0.95f, 1.0f);
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

    // Help overlay
    if (showHelp_)
    {
        int fbw=0, fbh=0; m_Window->GetFramebufferSize(fbw, fbh);
        overlay_.renderHelp(fbw, fbh, showModel_);
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

void ModelViewerApp::scanModels()
{
    using std::filesystem::recursive_directory_iterator;
    using std::filesystem::path;
    modelPaths_.clear();
    try
    {
        for (const auto& entry : recursive_directory_iterator("assets"))
        {
            if (!entry.is_regular_file()) continue;
            path p = entry.path();
            auto ext = p.extension().string();
            for (auto& c : ext) c = (char)tolower((unsigned char)c);
            if (ext == ".gltf" || ext == ".glb")
            {
                modelPaths_.push_back(p.string());
            }
        }
    }
    catch (...) { /* ignore scan errors; leave list empty */ }
}

void ModelViewerApp::switchModel(int dir)
{
    if (modelPaths_.empty() || !modelScene_) return;
    if (currentModelIndex_ < 0) currentModelIndex_ = 0;
    const int n = (int)modelPaths_.size();
    currentModelIndex_ = (currentModelIndex_ + dir) % n;
    if (currentModelIndex_ < 0) currentModelIndex_ += n;
    const std::string& path = modelPaths_[currentModelIndex_];
    if (!modelScene_->load(path))
    {
        printf("Model load error: %s\n", modelScene_->lastError().c_str());
    }
}
