#include "scenes/ModelScene.hpp"
#include "gfx/Model.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

ModelScene::~ModelScene() 
{ 
    shutdown(); 
}

bool ModelScene::init(const std::string& objPath) 
{
    if (initialized_) return true;

    shader_ = Shader::FromFiles("assets/shaders/phong.vert", "assets/shaders/phong.frag");
    model_  = std::make_unique<Model>();
    if (!model_->load(objPath)) 
    {
        err_ = model_->lastError();
        shader_.reset();
        model_.reset();

        return false;
    }

    // center and scale to a reasonable size (optional)
    glm::vec3 mn, mx; model_->getBounds(mn, mx);
    glm::vec3 center = 0.5f * (mn + mx);
    glm::vec3 size = (mx - mn);
    float maxSide = std::max(size.x, std::max(size.y, size.z));

    float s = (maxSide > 1e-6f) ? (1.0f / maxSide) : 1.0f;

    modelM_ = glm::scale(glm::mat4(1.0f), glm::vec3(s));
    modelM_ = glm::translate(modelM_, -center);

    initialized_ = true;
    return true;
}

void ModelScene::update(float dt) 
{
    (void)dt; 
}

void ModelScene::render(const Camera& cam) 
{
    if (!initialized_) return;

    shader_->use();

    // set light and eye
    const float len = sqrtf(1.f * 1.f + 1.f * 1.f + 0.6f * 0.6f);
    glUniform3f(shader_->loc("uLightDir"), -1.0f / len, 1.0f / len, -0.6f / len);
    glUniform3f(shader_->loc("uViewPos"), 4.0f, 3.0f, 4.0f); // TODO: supply real camera eye
    glUniform1i(shader_->loc("uUseLighting"), lighting_ ? 1 : 0);

    // Environment lighting to brighten and add reflections
    glUniform1i(shader_->loc("uUseEnv"), 1);
    glUniform3f(shader_->loc("uEnvSkyColor"),   0.60f, 0.70f, 0.90f);
    glUniform3f(shader_->loc("uEnvGroundColor"),0.35f, 0.35f, 0.35f);
    glUniform1f(shader_->loc("uEnvIntensity"),  0.6f);

    model_->render(cam, modelM_, *shader_);
}

void ModelScene::shutdown() 
{
    if (model_) model_->shutdown();
    model_.reset();
    shader_.reset();
    initialized_ = false;
}
