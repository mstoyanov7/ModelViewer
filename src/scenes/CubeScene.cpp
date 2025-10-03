#include "scenes/CubeScene.hpp"

static const char* kVert = R"(#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aCol;
out vec3 vCol;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
void main(){
    vCol = aCol;
    gl_Position = uProj * uView * uModel * vec4(aPos,1.0);
})";

static const char* kFrag = R"(#version 330 core
in vec3 vCol;
out vec4 FragColor;
void main(){
    FragColor = vec4(vCol, 1.0);
})";

CubeScene::CubeScene() = default;
CubeScene::~CubeScene() { shutdown(); }

void CubeScene::init(int /*fbWidth*/, int /*fbHeight*/) {
    if (initialized_) return;

    const float vertices[] = {
        // pos               // color
        -0.5f,-0.5f,-0.5f,   1,0,0,
         0.5f,-0.5f,-0.5f,   0,1,0,
         0.5f, 0.5f,-0.5f,   0,0,1,
        -0.5f, 0.5f,-0.5f,   1,1,0,
        -0.5f,-0.5f, 0.5f,   1,0,1,
         0.5f,-0.5f, 0.5f,   0,1,1,
         0.5f, 0.5f, 0.5f,   1,1,1,
        -0.5f, 0.5f, 0.5f,   0.2f,0.7f,0.9f
    };

    const unsigned indices[] = {
        // back face
        0,1,2, 2,3,0,
        // front
        4,5,6, 6,7,4,
        // left
        0,3,7, 7,4,0,
        // right
        1,5,6, 6,2,1,
        // bottom
        0,1,5, 5,4,0,
        // top
        3,2,6, 6,7,3
    };

    glGenVertexArrays(1,&vao_);
    glGenBuffers(1,&vbo_);
    glGenBuffers(1,&ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

    glBindVertexArray(0);

    shader_ = std::make_unique<Shader>(kVert, kFrag);
    initialized_ = true;
}

void CubeScene::update(float dt) {
    angle_ += dt * 0.8f; // rad/s
    model_ = glm::rotate(glm::mat4(1.0f), angle_, glm::vec3(0.3f,1.0f,0.2f));
}

void CubeScene::render(const Camera& cam) {
    if (!initialized_) return;
    shader_->use();
    // uniforms
    GLint locM = shader_->loc("uModel");
    GLint locV = shader_->loc("uView");
    GLint locP = shader_->loc("uProj");
    glUniformMatrix4fv(locM, 1, GL_FALSE, &model_[0][0]);
    glUniformMatrix4fv(locV, 1, GL_FALSE, &cam.view()[0][0]);
    glUniformMatrix4fv(locP, 1, GL_FALSE, &cam.proj()[0][0]);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CubeScene::shutdown() {
    if (ebo_) { glDeleteBuffers(1,&ebo_); ebo_=0; }
    if (vbo_) { glDeleteBuffers(1,&vbo_); vbo_=0; }
    if (vao_) { glDeleteVertexArrays(1,&vao_); vao_=0; }
    shader_.reset();
    initialized_ = false;
}
