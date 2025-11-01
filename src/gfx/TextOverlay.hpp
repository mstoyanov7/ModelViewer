#pragma once
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class TextOverlay {
public:
    void init();
    void shutdown();

    // Render a translucent panel with white text lines at top-left.
    void renderHelp(int fbWidth, int fbHeight, bool modelMode) const;

    // Low-level: draw a string at pixel position with given color.
    void drawString(int fbWidth, int fbHeight, float x, float y, float scale, const std::string& text, float r, float g, float b, float a) const;

    // Draw a solid rect at pixel coordinates.
    void drawRect(int fbWidth, int fbHeight, float x, float y, float w, float h, float r, float g, float b, float a) const;

private:
    unsigned int vao_ = 0, vbo_ = 0;
    unsigned int prog_ = 0;

    void ensureGL() const;
    static void buildGlyph(char c, std::vector<float>& outXY, float x, float y, float px, float py, float scale);
};

