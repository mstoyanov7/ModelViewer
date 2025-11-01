#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "gfx/Shader.hpp"
#include "core/Camera.hpp"

using GLuint = unsigned int;

class Model {
public:
    Model() = default;
    ~Model();

    // Load a .gltf or .glb file (geometry only; colors if present). Returns false on error.
    bool loadGLTF(const std::string& path);

    // Auto-detect by file extension: .gltf, .glb
    bool load(const std::string& path);

    // Draw with Phong shader (provided by caller or owned here)
    void render(const Camera& cam, const glm::mat4& model, Shader& shader) const;

    // Simple bounds for framing the camera
    void getBounds(glm::vec3& minOut, glm::vec3& maxOut) const { minOut = bmin_; maxOut = bmax_; }

    const std::string& lastError() const { return err_; }

    // GL resources
    void shutdown();

private:
    struct Vertex { glm::vec3 pos; glm::vec3 nrm; glm::vec3 col; glm::vec2 uv; };
    struct Draw {
        int first = 0;
        int count = 0;
        unsigned int tex = 0;
        bool blend = false;              // glTF material alphaMode == BLEND
        glm::vec4 baseColorFactor{1.0f}; // glTF baseColorFactor
    };

    GLuint vao_ = 0, vbo_ = 0;
    int vertexCount_ = 0; // non-indexed triangles
    glm::vec3 bmin_{0}, bmax_{0}; // AABB in object space
    std::string err_;

    std::vector<Draw> draws_;
    std::vector<unsigned int> textures_; // owned GL textures

    static void computeFlatNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, glm::vec3& n);
};
