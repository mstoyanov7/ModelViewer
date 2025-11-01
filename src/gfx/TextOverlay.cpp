#include "gfx/TextOverlay.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstring>
#include <cctype>

static unsigned int compile(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0; 
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) 
    { 
        char log[1024]; 
        glGetShaderInfoLog(s, 1024, nullptr, log); 
        glDeleteShader(s); 
        return 0; 
    }
    return s;
}

static unsigned int linkProg(const char* vs, const char* fs)
{
    GLuint v = compile(GL_VERTEX_SHADER, vs);
    GLuint f = compile(GL_FRAGMENT_SHADER, fs);
    if (!v || !f) 
    { 
        if(v) glDeleteShader(v); 
        if(f) glDeleteShader(f); 
        return 0; 
    }
    GLuint p = glCreateProgram();
    glAttachShader(p, v); 
    glAttachShader(p, f); 
    glLinkProgram(p);
    glDeleteShader(v); 
    glDeleteShader(f);
    GLint ok = 0; 
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) 
    { char log[1024]; 
        glGetProgramInfoLog(p, 1024, nullptr, log); 
        glDeleteProgram(p); 
        return 0; 
    }
    return p;
}

void TextOverlay::init()
{
    if (!prog_)
    {
        // Minimal inline shader sources matching assets/shaders/overlay.* for portability
        const char* vs = "#version 330 core\nlayout(location=0) in vec2 aPos;\nvoid main(){ gl_Position = vec4(aPos,0.0,1.0); }\n";
        const char* fs = "#version 330 core\nout vec4 FragColor; uniform vec4 uColor; void main(){ FragColor = uColor; }\n";
        prog_ = linkProg(vs, fs);
    }
    if (!vao_)
    {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glBindVertexArray(0);
    }
}

void TextOverlay::shutdown()
{
    if (vbo_) { glDeleteBuffers(1, &vbo_); vbo_ = 0; }
    if (vao_) { glDeleteVertexArrays(1, &vao_); vao_ = 0; }
    if (prog_) { glDeleteProgram(prog_); prog_ = 0; }
}

void TextOverlay::ensureGL() const
{
    // Lazy safety; in this minimal helper we assume init() called
}

static float toNDCX(int fbw, float x){ return (x / (float)fbw) * 2.0f - 1.0f; }
static float toNDCY(int fbh, float y){ return 1.0f - (y / (float)fbh) * 2.0f; }

void TextOverlay::drawRect(int fbWidth, int fbHeight, float x, float y, float w, float h, float r, float g, float b, float a) const
{
    float x0 = toNDCX(fbWidth, x);
    float y0 = toNDCY(fbHeight, y);
    float x1 = toNDCX(fbWidth, x + w);
    float y1 = toNDCY(fbHeight, y + h);

    float verts[] = {
        x0,y0, x1,y0, x1,y1,
        x0,y0, x1,y1, x0,y1
    };

    glUseProgram(prog_);
    GLint loc = glGetUniformLocation(prog_, "uColor");
    glUniform4f(loc, r,g,b,a);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
    if (depthWasOn) glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
    if (depthWasOn) glEnable(GL_DEPTH_TEST);
}

// 5x7 font bitmaps for ASCII A-Z, 0-9 and a few symbols. 1 bit per pixel, 5 columns x 7 rows.
static bool glyphBits(char c, int row, int col)
{
    // Clamp grid
    if (row < 0 || row >= 7 || col < 0 || col >= 5) return false;
    unsigned char r = 0;
    switch (c)
    {
        case 'A': { static const unsigned char rows[7]={0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}; r=rows[row]; break; }
        case 'B': { static const unsigned char rows[7]={0x1E,0x11,0x1E,0x11,0x11,0x11,0x1E}; r=rows[row]; break; }
        case 'C': { static const unsigned char rows[7]={0x0F,0x10,0x10,0x10,0x10,0x10,0x0F}; r=rows[row]; break; }
        case 'D': { static const unsigned char rows[7]={0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}; r=rows[row]; break; }
        case 'E': { static const unsigned char rows[7]={0x1F,0x10,0x1E,0x10,0x10,0x10,0x1F}; r=rows[row]; break; }
        case 'F': { static const unsigned char rows[7]={0x1F,0x10,0x1E,0x10,0x10,0x10,0x10}; r=rows[row]; break; }
        case 'G': { static const unsigned char rows[7]={0x0F,0x10,0x10,0x17,0x11,0x11,0x0F}; r=rows[row]; break; }
        case 'H': { static const unsigned char rows[7]={0x11,0x11,0x1F,0x11,0x11,0x11,0x11}; r=rows[row]; break; }
        case 'I': { static const unsigned char rows[7]={0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}; r=rows[row]; break; }
        case 'J': { static const unsigned char rows[7]={0x01,0x01,0x01,0x01,0x11,0x11,0x0E}; r=rows[row]; break; }
        case 'K': { static const unsigned char rows[7]={0x11,0x12,0x1C,0x12,0x11,0x11,0x11}; r=rows[row]; break; }
        case 'L': { static const unsigned char rows[7]={0x10,0x10,0x10,0x10,0x10,0x10,0x1F}; r=rows[row]; break; }
        case 'M': { static const unsigned char rows[7]={0x11,0x1B,0x15,0x11,0x11,0x11,0x11}; r=rows[row]; break; }
        case 'N': { static const unsigned char rows[7]={0x11,0x19,0x15,0x13,0x11,0x11,0x11}; r=rows[row]; break; }
        case 'O': { static const unsigned char rows[7]={0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}; r=rows[row]; break; }
        case 'P': { static const unsigned char rows[7]={0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}; r=rows[row]; break; }
        case 'Q': { static const unsigned char rows[7]={0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}; r=rows[row]; break; }
        case 'R': { static const unsigned char rows[7]={0x1E,0x11,0x11,0x1E,0x12,0x11,0x11}; r=rows[row]; break; }
        case 'S': { static const unsigned char rows[7]={0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}; r=rows[row]; break; }
        case 'T': { static const unsigned char rows[7]={0x1F,0x04,0x04,0x04,0x04,0x04,0x04}; r=rows[row]; break; }
        case 'U': { static const unsigned char rows[7]={0x11,0x11,0x11,0x11,0x11,0x11,0x0E}; r=rows[row]; break; }
        case 'V': { static const unsigned char rows[7]={0x11,0x11,0x11,0x11,0x11,0x0A,0x04}; r=rows[row]; break; }
        case 'W': { static const unsigned char rows[7]={0x11,0x11,0x11,0x11,0x15,0x1B,0x11}; r=rows[row]; break; }
        case 'X': { static const unsigned char rows[7]={0x11,0x0A,0x04,0x04,0x0A,0x11,0x11}; r=rows[row]; break; }
        case 'Y': { static const unsigned char rows[7]={0x11,0x0A,0x04,0x04,0x04,0x04,0x04}; r=rows[row]; break; }
        case 'Z': { static const unsigned char rows[7]={0x1F,0x02,0x04,0x04,0x08,0x10,0x1F}; r=rows[row]; break; }
        case '0': { static const unsigned char rows[7]={0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}; r=rows[row]; break; }
        case '1': { static const unsigned char rows[7]={0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}; r=rows[row]; break; }
        case '2': { static const unsigned char rows[7]={0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}; r=rows[row]; break; }
        case '3': { static const unsigned char rows[7]={0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E}; r=rows[row]; break; }
        case '4': { static const unsigned char rows[7]={0x12,0x12,0x12,0x1F,0x02,0x02,0x02}; r=rows[row]; break; }
        case '5': { static const unsigned char rows[7]={0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}; r=rows[row]; break; }
        case '6': { static const unsigned char rows[7]={0x0E,0x10,0x1E,0x11,0x11,0x11,0x0E}; r=rows[row]; break; }
        case '7': { static const unsigned char rows[7]={0x1F,0x01,0x02,0x04,0x08,0x08,0x08}; r=rows[row]; break; }
        case '8': { static const unsigned char rows[7]={0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}; r=rows[row]; break; }
        case '9': { static const unsigned char rows[7]={0x0E,0x11,0x11,0x0F,0x01,0x01,0x0E}; r=rows[row]; break; }
        case '-': { static const unsigned char rows[7]={0x00,0x00,0x00,0x1F,0x00,0x00,0x00}; r=rows[row]; break; }
        case '/': { static const unsigned char rows[7]={0x01,0x02,0x04,0x08,0x10,0x00,0x00}; r=rows[row]; break; }
        case ':': { static const unsigned char rows[7]={0x00,0x04,0x04,0x00,0x04,0x04,0x00}; r=rows[row]; break; }
        case '.': { static const unsigned char rows[7]={0x00,0x00,0x00,0x00,0x00,0x06,0x06}; r=rows[row]; break; }
        case ',': { static const unsigned char rows[7]={0x00,0x00,0x00,0x00,0x00,0x06,0x04}; r=rows[row]; break; }
        case ' ': default: return false;
    }
    // columns: bit4..bit0 as left..right
    unsigned char mask = (unsigned char)(1 << (4 - col));
    return (r & mask) != 0;
}

void TextOverlay::buildGlyph(char c, std::vector<float>& outXY, float x, float y, float px, float py, float scale)
{
    // 5x7 pixels; each pixel -> rectangle px by py
    for (int row = 0; row < 7; ++row)
    {
        for (int col = 0; col < 5; ++col)
        {
            if (!glyphBits(c, row, col)) continue;
            float gx = x + col * px * scale;
            float gy = y + row * py * scale;
            float x0 = gx, y0 = gy;
            float x1 = gx + px*scale, y1 = gy + py*scale;
            // two triangles
            outXY.push_back(x0); outXY.push_back(y0);
            outXY.push_back(x1); outXY.push_back(y0);
            outXY.push_back(x1); outXY.push_back(y1);
            outXY.push_back(x0); outXY.push_back(y0);
            outXY.push_back(x1); outXY.push_back(y1);
            outXY.push_back(x0); outXY.push_back(y1);
        }
    }
}

void TextOverlay::drawString(int fbWidth, int fbHeight, float x, float y, float scale, const std::string& text, float r, float g, float b, float a) const
{
    // Build triangle list in pixel space then convert to NDC on upload
    std::vector<float> xy; xy.reserve(text.size() * 6 * 2 * 6);
    float cursor = x;
    for (char c : text)
    {
        if (c == '\n') { y += 9.0f * scale; cursor = x; continue; }
        buildGlyph((char)std::toupper((unsigned char)c), xy, cursor, y, 1.0f, 1.0f, scale);
        cursor += 6.0f * scale; // 5px glyph + 1px spacing
    }

    // Convert to NDC
    for (size_t i=0;i<xy.size(); i+=2)
    {
        xy[i+0] = toNDCX(fbWidth, xy[i+0]);
        xy[i+1] = toNDCY(fbHeight, xy[i+1]);
    }

    glUseProgram(prog_);
    GLint loc = glGetUniformLocation(prog_, "uColor");
    glUniform4f(loc, r,g,b,a);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, xy.size()*sizeof(float), xy.data(), GL_DYNAMIC_DRAW);
    GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
    if (depthWasOn) glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(xy.size()/2));
    glDisable(GL_BLEND);
    if (depthWasOn) glEnable(GL_DEPTH_TEST);
}

void TextOverlay::renderHelp(int fbWidth, int fbHeight, bool modelMode) const
{
    const float pad = 12.0f;
    float x = pad, y = pad + 18.0f; // top-left text baseline
    float scale = 2.0f; // 2x 5x7 -> ~10x14 per char

    // Panel size: compute conservative width by character count
    const char* title = "Help";
    const char* linesModel[] = {
        "M - Toggle Model/Cube",
        "F - Toggle Wireframe",
        "C - Toggle Face Culling",
        "R - Reset Camera",
        "RMB Drag - Orbit",
        "MMB Drag - Pan",
        "Scroll - Zoom",
        "H - Toggle Help"
    };
    const char* linesCube[] = {
        "M - Toggle Model/Cube",
        "L - Toggle Lighting",
        "F - Toggle Wireframe",
        "C - Toggle Face Culling",
        "R - Reset Camera",
        "RMB Drag - Orbit",
        "MMB Drag - Pan",
        "Scroll - Zoom",
        "H - Toggle Help"
    };

    const char** lines = modelMode ? linesModel : linesCube;
    int nlines = modelMode ? (int)(sizeof(linesModel)/sizeof(linesModel[0])) : (int)(sizeof(linesCube)/sizeof(linesCube[0]));

    int maxChars = 0;
    for (int i=0 ; i < nlines; ++i) maxChars = std::max<int>(maxChars, (int)std::strlen(lines[i]));
    int panelW = (int)(pad*2 + (maxChars * 6.0f * scale));
    int panelH = (int)(pad*2 + 22.0f + (nlines * (9.0f * scale)));

    // Background panel (semi-transparent)
    drawRect(fbWidth, fbHeight, 8.0f, 8.0f, (float)panelW, (float)panelH, 0.05f, 0.06f, 0.08f, 0.8f);

    // Title
    drawString(fbWidth, fbHeight, x, y, scale, title, 1,1,1,1);
    y += 18.0f;

    for (int i=0;i<nlines;++i)
    {
        drawString(fbWidth, fbHeight, x, y + i * (9.0f * scale), scale, lines[i], 0.9f,0.9f,0.9f,1.0f);
    }
}
