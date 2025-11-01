#include "gfx/Model.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tiny_obj_loader.h>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

Model::~Model() { shutdown(); }

void Model::shutdown() {
    if (vbo_) { glDeleteBuffers(1,&vbo_); vbo_=0; }
    if (vao_) { glDeleteVertexArrays(1,&vao_); vao_=0; }
    vertexCount_ = 0;
}

bool Model::loadOBJ(const std::string& path) {
    shutdown();
    err_.clear();

    tinyobj::ObjReaderConfig cfg;
    cfg.triangulate = true;
    cfg.vertex_color = true; // if present

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path, cfg)) {
        err_ = reader.Error().empty() ? "Failed to read .obj" : reader.Error();
        return false;
    }
    if (!reader.Warning().empty()) {
        // not fatal; stash if you want
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    std::vector<Vertex> verts;
    verts.reserve(50000);

    // init AABB
    glm::vec3 bmin{ std::numeric_limits<float>::max() };
    glm::vec3 bmax{ std::numeric_limits<float>::lowest() };

    for (const auto& sh : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < sh.mesh.num_face_vertices.size(); f++) {
            const int fv = sh.mesh.num_face_vertices[f]; // should be 3 due to triangulate
            if (fv != 3) { index_offset += fv; continue; }

            glm::vec3 pos[3]; glm::vec3 nrm[3]; glm::vec3 col[3];
            bool hasN[3] = {false,false,false};
            bool hasC[3] = {false,false,false};

            for (int v = 0; v < 3; v++) {
                tinyobj::index_t idx = sh.mesh.indices[index_offset + v];

                pos[v].x = attrib.vertices[3*idx.vertex_index+0];
                pos[v].y = attrib.vertices[3*idx.vertex_index+1];
                pos[v].z = attrib.vertices[3*idx.vertex_index+2];

                if (idx.normal_index >= 0) {
                    hasN[v] = true;
                    nrm[v].x = attrib.normals[3*idx.normal_index+0];
                    nrm[v].y = attrib.normals[3*idx.normal_index+1];
                    nrm[v].z = attrib.normals[3*idx.normal_index+2];
                }

                if (!attrib.colors.empty() && idx.vertex_index >= 0) {
                    hasC[v] = true;
                    col[v].r = attrib.colors[3*idx.vertex_index+0];
                    col[v].g = attrib.colors[3*idx.vertex_index+1];
                    col[v].b = attrib.colors[3*idx.vertex_index+2];
                } else {
                    col[v] = glm::vec3(0.75f); // default gray
                }
            }

            // if any normal missing → compute a flat face normal
            if (!(hasN[0] && hasN[1] && hasN[2])) {
                glm::vec3 fn; computeFlatNormal(pos[0], pos[1], pos[2], fn);
                nrm[0] = nrm[1] = nrm[2] = fn;
            }

            // append 3 vertices
            for (int v=0; v<3; ++v) {
                verts.push_back({pos[v], nrm[v], col[v]});
                // expand bounds
                bmin.x = std::min(bmin.x, pos[v].x); bmax.x = std::max(bmax.x, pos[v].x);
                bmin.y = std::min(bmin.y, pos[v].y); bmax.y = std::max(bmax.y, pos[v].y);
                bmin.z = std::min(bmin.z, pos[v].z); bmax.z = std::max(bmax.z, pos[v].z);
            }

            index_offset += fv;
        }
    }

    if (verts.empty()) { err_ = "No vertices parsed from OBJ."; return false; }

    // upload to GPU
    glGenVertexArrays(1,&vao_);
    glGenBuffers(1,&vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    const GLsizei stride = sizeof(Vertex);
    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(Vertex,pos));
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(Vertex,nrm));
    glEnableVertexAttribArray(2); // color
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(Vertex,col));
    glBindVertexArray(0);

    vertexCount_ = static_cast<int>(verts.size());
    bmin_ = bmin; bmax_ = bmax;
    return true;
}

void Model::computeFlatNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, glm::vec3& n) {
    glm::vec3 e1 = b - a, e2 = c - a;
    glm::vec3 nn = glm::cross(e1, e2);
    float len = glm::length(nn);
    n = (len > 1e-10f) ? (nn / len) : glm::vec3(0,1,0);
}

void Model::render(const Camera& cam, const glm::mat4& model, Shader& shader) const {
    if (!vao_ || vertexCount_ <= 0) return;

    // You already set uniforms in your scenes; we set them here for convenience:
    // expect shader "uModel/uView/uProj/uNormalMat/uLightDir/uViewPos/uUseLighting"
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
    glUniformMatrix4fv(shader.loc("uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(shader.loc("uView"),  1, GL_FALSE, glm::value_ptr(cam.view()));
    glUniformMatrix4fv(shader.loc("uProj"),  1, GL_FALSE, glm::value_ptr(cam.proj()));
    glUniformMatrix3fv(shader.loc("uNormalMat"), 1, GL_FALSE, glm::value_ptr(normalMat));

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    glBindVertexArray(0);
}
