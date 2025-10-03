#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

Shader::Shader(const char* vertSrc, const char* fragSrc) 
{
    GLuint vs = compile(GL_VERTEX_SHADER,   vertSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragSrc);

    prog_ = glCreateProgram();
    glAttachShader(prog_, vs);
    glAttachShader(prog_, fs);
    glLinkProgram(prog_);

    GLint ok = 0;
    glGetProgramiv(prog_, GL_LINK_STATUS, &ok);
    if (!ok) 
    {
        char log[2048];
        glGetProgramInfoLog(prog_, 2048, nullptr, log);
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(prog_);
        throw std::runtime_error(std::string("Program link failed: ") + log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() 
{
    if (prog_) glDeleteProgram(prog_);
}

Shader::Shader(Shader&& o) noexcept 
{ 
    prog_ = o.prog_;
    o.prog_ = 0; 
}

Shader& Shader::operator=(Shader&& o) noexcept 
{
    if (this != &o) 
    { 
        if (prog_) glDeleteProgram(prog_);
        prog_ = o.prog_;
        o.prog_ = 0; 
    }

    return *this;
}

GLuint Shader::compile(GLenum type, const char* src) 
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) 
    {
        char log[2048];
        glGetShaderInfoLog(s, 2048, nullptr, log);
        glDeleteShader(s);
        const char* kind = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        throw std::runtime_error(std::string("Compile failed (") + kind + "): " + log);
    }

    return s;
}

void Shader::use() const 
{ 
    glUseProgram(prog_); 
}

GLint Shader::loc(const char* name) const 
{ 
    return glGetUniformLocation(prog_, name); 
}

void Shader::setMat4(const char* name, const float* m16) const 
{ 
    glUniformMatrix4fv(loc(name), 1, GL_FALSE, m16); 
}

void Shader::setVec3(const char* name, float x,float y,float z) const 
{ 
    glUniform3f(loc(name), x,y,z); 
}
