#pragma once
#include <memory>
#include <string>
#include <glm/mat4x4.hpp>

#include "gfx/Model.hpp"

class Shader;
class Camera;
class Model;

class ModelScene {
public:
    ModelScene() = default;
    ~ModelScene();

    // call before first render
    bool init(const std::string& objPath);

    // Load or reload a model file (.gltf/.glb). Keeps shader.
    bool load(const std::string& objPath);

    void update(float dt);
    void render(const Camera& cam);
    void shutdown();

    void setLighting(bool on) { lighting_ = on; }
    const std::string& lastError() const { return err_; }

private:
    bool initialized_ = false;
    bool lighting_    = true;
    std::unique_ptr<Shader> shader_;
    std::unique_ptr<Model>  model_;

    glm::mat4 modelM_{1.0f};
    float spin_ = 0.0f;
    std::string err_;
};
