#include "scenes/ModelScene.hpp"
#include "gfx/Model.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

static const char* kVS = R"(#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec3 aCol;

out vec3 vCol;
out vec3 vNormal;
out vec3 vWorldPos;

uniform mat4 uModel, uView, uProj;
uniform mat3 uNormalMat;

void main(){
  vec4 wp = uModel * vec4(aPos,1.0);
  vWorldPos = wp.xyz;
  vNormal = normalize(uNormalMat * aNormal);
  vCol = aCol;
  gl_Position = uProj * uView * wp;
})";

static const char* kFS = R"(#version 330 core
in vec3 vCol;
in vec3 vNormal;
in vec3 vWorldPos;
out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uViewPos;
uniform bool uUseLighting;

void main(){
  vec3 base = vCol;
  if (!uUseLighting) { FragColor = vec4(base,1.0); return; }

  vec3 N = normalize(vNormal);
  vec3 L = normalize(uLightDir);
  vec3 V = normalize(uViewPos - vWorldPos);
  vec3 R = reflect(-L, N);

  float diff = max(dot(N,L), 0.0);
  float spec = pow(max(dot(R,V),0.0), 32.0);

  vec3 amb = 0.12 * base;
  vec3 dif = 0.88 * diff * base;
  vec3 spc = 0.25 * spec * vec3(1.0);
  FragColor = vec4(amb+dif+spc, 1.0);
})";

ModelScene::~ModelScene() { shutdown(); }

bool ModelScene::init(const std::string& objPath) {
    if (initialized_) return true;

    shader_ = std::make_unique<Shader>(kVS, kFS);
    model_  = std::make_unique<Model>();
    if (!model_->loadOBJ(objPath)) {
        err_ = model_->lastError();
        shader_.reset(); model_.reset();
        return false;
    }

    // center and scale to a reasonable size (optional)
    glm::vec3 mn, mx; model_->getBounds(mn, mx);
    glm::vec3 center = 0.5f * (mn + mx);
    glm::vec3 size   = (mx - mn);
    float maxSide = std::max(size.x, std::max(size.y, size.z));

    float s = (maxSide > 1e-6f) ? (1.0f / maxSide) : 1.0f;

    modelM_ = glm::scale(glm::mat4(1.0f), glm::vec3(s));
    modelM_ = glm::translate(modelM_, -center);

    initialized_ = true;
    return true;
}

void ModelScene::update(float dt) {
    (void)dt; 
}

void ModelScene::render(const Camera& cam) {
    if (!initialized_) return;

    shader_->use();

    // set light and eye
    const float len = sqrtf(1.f*1.f + 1.f*1.f + 0.6f*0.6f);
    glUniform3f(shader_->loc("uLightDir"), -1.0f/len, 1.0f/len, -0.6f/len);
    glUniform3f(shader_->loc("uViewPos"), 4.0f, 3.0f, 4.0f); // TODO: supply real camera eye

    glUniform1i(shader_->loc("uUseLighting"), lighting_ ? 1 : 0);

    model_->render(cam, modelM_, *shader_);
}

void ModelScene::shutdown() {
    if (model_)  model_->shutdown();
    model_.reset();
    shader_.reset();
    initialized_ = false;
}
