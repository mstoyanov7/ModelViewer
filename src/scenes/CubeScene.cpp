#include "scenes/CubeScene.hpp"

// Vertex shader: positions, normals, colors; Phong in fragment
static const char* kVert = R"(#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec3 aCol;

out vec3 vCol;
out vec3 vNormal;
out vec3 vWorldPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat3 uNormalMat;

void main(){
    vec4 worldPos = uModel * vec4(aPos,1.0);
    vWorldPos = worldPos.xyz;
    vNormal   = normalize(uNormalMat * aNormal);
    vCol      = aCol;
    gl_Position = uProj * uView * worldPos;
})";

static const char* kFrag = R"(#version 330 core
in vec3 vCol;
in vec3 vNormal;
in vec3 vWorldPos;

out vec4 FragColor;

uniform vec3 uLightDir;    // normalized, world space (direction *towards* surface)
uniform vec3 uViewPos;     // camera position, world space
uniform bool uUseLighting;

void main(){
    vec3 baseColor = vCol;

    if(!uUseLighting){
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    // Phong lighting
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(R, V), 0.0), 32.0);

    vec3 ambient  = 0.12 * baseColor;
    vec3 diffuse  = 0.88 * diff * baseColor;
    vec3 specular = 0.25 * spec * vec3(1.0);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
})";

CubeScene::~CubeScene() { shutdown(); }

void CubeScene::init() {
    if (initialized_) return;

    // Non-indexed cube with per-face normals (6 faces * 2 triangles * 3 verts = 36)
    // Each vertex: position (3), normal (3), color (3) -> stride = 9 floats
    const float n = 0.5f;
    struct V { float px,py,pz, nx,ny,nz, r,g,b; };

    // helper to push a face (p0,p1,p2,p3 CCW), with a shared face normal & color
    auto pushFace = [](std::vector<V>& out,
                       glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                       glm::vec3 faceNormal, glm::vec3 col)
    {
        // tri1: p0 p1 p2; tri2: p2 p3 p0
        out.push_back({p0.x,p0.y,p0.z, faceNormal.x,faceNormal.y,faceNormal.z, col.r,col.g,col.b});
        out.push_back({p1.x,p1.y,p1.z, faceNormal.x,faceNormal.y,faceNormal.z, col.r,col.g,col.b});
        out.push_back({p2.x,p2.y,p2.z, faceNormal.x,faceNormal.y,faceNormal.z, col.r,col.g,col.b});
        out.push_back({p2.x,p2.y,p2.z, faceNormal.x,faceNormal.y,faceNormal.z, col.r,col.g,col.b});
        out.push_back({p3.x,p3.y,p3.z, faceNormal.x,faceNormal.y,faceNormal.z, col.r,col.g,col.b});
        out.push_back({p0.x,p0.y,p0.z, faceNormal.x,faceNormal.y,faceNormal.z, col.r,col.g,col.b});
    };

    std::vector<V> verts; verts.reserve(36);

    // Define cube corners
    glm::vec3 p000 = {-n,-n,-n};
    glm::vec3 p100 = { n,-n,-n};
    glm::vec3 p110 = { n, n,-n};
    glm::vec3 p010 = {-n, n,-n};
    glm::vec3 p001 = {-n,-n, n};
    glm::vec3 p101 = { n,-n, n};
    glm::vec3 p111 = { n, n, n};
    glm::vec3 p011 = {-n, n, n};

    // Colors per face
    glm::vec3 Cx  = {1,0.2f,0.2f};
    glm::vec3 Cnx = {0.9f,0.4f,0.4f};
    glm::vec3 Cy  = {0.2f,1,0.2f};
    glm::vec3 Cny = {0.4f,0.9f,0.4f};
    glm::vec3 Cz  = {0.2f,0.5f,1};
    glm::vec3 Cnz = {0.4f,0.7f,0.9f};

    // +X face (normal +X)
    pushFace(verts, p100,p110,p111,p101, {1,0,0}, Cx);
    // -X face
    pushFace(verts, p010,p000,p001,p011, {-1,0,0}, Cnx);
    // +Y face
    pushFace(verts, p110,p010,p011,p111, {0,1,0}, Cy);
    // -Y face
    pushFace(verts, p000,p100,p101,p001, {0,-1,0}, Cny);
    // +Z face
    pushFace(verts, p101,p111,p011,p001, {0,0,1}, Cz);
    // -Z face
    pushFace(verts, p000,p010,p110,p100, {0,0,-1}, Cnz);

    vertexCount_ = static_cast<int>(verts.size());

    glGenVertexArrays(1,&vao_);
    glGenBuffers(1,&vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(V), verts.data(), GL_STATIC_DRAW);

    const GLsizei stride = sizeof(V);
    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(V, px));
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(V, nx));
    glEnableVertexAttribArray(2); // color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(V, r));

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

    // normal matrix = transpose(inverse(mat3(model)))
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model_)));
    glUniformMatrix4fv(shader_->loc("uModel"), 1, GL_FALSE, &model_[0][0]);
    glUniformMatrix4fv(shader_->loc("uView"),  1, GL_FALSE, &cam.view()[0][0]);
    glUniformMatrix4fv(shader_->loc("uProj"),  1, GL_FALSE, &cam.proj()[0][0]);
    glUniformMatrix3fv(shader_->loc("uNormalMat"), 1, GL_FALSE, &normalMat[0][0]);

    // lighting uniforms
    // Light coming from (−1,+1,−0.6) direction toward the origin
    const float len = sqrtf(1.f*1.f + 1.f*1.f + 0.6f*0.6f);
    const float lx = -1.0f/len, ly = 1.0f/len, lz = -0.6f/len;
    glUniform3f(shader_->loc("uLightDir"), lx, ly, lz);

    // camera position: derive from inverse of view if you don't store it
    // Here we reconstruct from view matrix (approx): inverse(view)*[0,0,0,1]
    // Simpler: pass it from app via a uniform; but we'll keep it here as (0,0,0) in view space mapped to world.
    // If your Camera doesn’t expose eye(), set your OrbitCamera to keep target & radius and compute eye similarly.
    // For now, set a reasonable eye for specular highlight:
    // (Optional TODO: expose Camera::eye())
    glUniform3f(shader_->loc("uViewPos"), 4.0f, 3.0f, 4.0f);

    glUniform1i(shader_->loc("uUseLighting"), lighting_ ? 1 : 0);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    glBindVertexArray(0);
}

void CubeScene::shutdown() {
    if (vbo_) { glDeleteBuffers(1,&vbo_); vbo_=0; }
    if (vao_) { glDeleteVertexArrays(1,&vao_); vao_=0; }
    shader_.reset();
    initialized_ = false;
}

