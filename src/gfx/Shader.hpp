#pragma once
#include <string>
#include <cstdint>
#include <memory>

using GLuint = unsigned int;
using GLenum = unsigned int;
using GLint  = int;

class Shader {
public:
    Shader(const char* vertexSrc, const char* fragmentSrc);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) noexcept;
    Shader& operator=(Shader&&) noexcept;

    void use() const;
    GLuint id() const { return prog_; }

    GLint loc(const char* name) const;
    void  setMat4(const char* name, const float* m16) const;
    void  setVec3(const char* name, float x, float y, float z) const;

    // Load shader sources from files on disk located at the given paths.
    static std::unique_ptr<Shader> FromFiles(const char* vertexPath, const char* fragmentPath);

private:
    GLuint prog_ = 0;
    static GLuint compile(GLenum type, const char* src);
};
