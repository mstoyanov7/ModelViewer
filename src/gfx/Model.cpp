#include "gfx/Model.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <cctype>
#include <algorithm>
#include <unordered_map>

// tinygltf: header-only glTF 2.0 loader (enable STB image for textures)
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

Model::~Model() 
{ 
    shutdown(); 
}

void Model::shutdown() 
{
    if (vbo_) 
    { 
        glDeleteBuffers(1, &vbo_); 
        vbo_ = 0; 
    }
    if (vao_) 
    { 
        glDeleteVertexArrays(1, &vao_); 
        vao_ = 0; 
    }
    if (!textures_.empty())
    {
        glDeleteTextures((GLsizei)textures_.size(), textures_.data());
        textures_.clear();
    }
    vertexCount_ = 0;
    draws_.clear();
}


static std::string toLowerExt(const std::string& path)
{
    auto dot = path.find_last_of('.');
    if (dot == std::string::npos) return {};
    std::string ext = path.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return char(std::tolower(c)); });
    return ext;
}

bool Model::load(const std::string& path)
{
    const std::string ext = toLowerExt(path);
    if (ext == ".gltf" || ext == ".glb")
        return loadGLTF(path);
    err_ = "Unsupported file extension: " + ext + "; only .gltf/.glb supported.";
    return false;
}

static bool readAccessorFloatVecN(const tinygltf::Model& m,
                                  const tinygltf::Accessor& acc,
                                  int N,
                                  std::vector<float>& out)
{
    const tinygltf::BufferView& bv = m.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = m.buffers[bv.buffer];
    const size_t count = acc.count;

    size_t componentSize = 0;
    bool isFloat = false;
    float normScale = 1.0f;
    const bool normalized = acc.normalized;
    switch (acc.componentType)
    {
        case TINYGLTF_COMPONENT_TYPE_FLOAT: componentSize = sizeof(float); isFloat = true; break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: componentSize = sizeof(uint16_t); normScale = normalized ? (1.0f / 65535.0f) : 1.0f; break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: componentSize = sizeof(uint8_t); normScale = normalized ? (1.0f / 255.0f) : 1.0f; break;
        default: return false;
    }

    size_t stride = N * componentSize;
    if (bv.byteStride) stride = bv.byteStride;

    const size_t start = bv.byteOffset + acc.byteOffset;
    const unsigned char* base = buf.data.data() + start;

    out.resize(count * N);
    for (size_t i = 0; i < count; ++i)
    {
        const unsigned char* src = base + i * stride;
        if (isFloat)
        {
            const float* fsrc = reinterpret_cast<const float*>(src);
            for (int k = 0; k < N; ++k) out[i * N + k] = fsrc[k];
        }
        else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            const uint16_t* p = reinterpret_cast<const uint16_t*>(src);
            for (int k = 0; k < N; ++k) out[i * N + k] = normalized ? (p[k] * normScale) : float(p[k]);
        }
        else // UNSIGNED_BYTE
        {
            const uint8_t* p = reinterpret_cast<const uint8_t*>(src);
            for (int k = 0; k < N; ++k) out[i * N + k] = normalized ? (p[k] * normScale) : float(p[k]);
        }
    }
    return true;
}

static bool readIndices(const tinygltf::Model& m, const tinygltf::Accessor& acc, std::vector<uint32_t>& out)
{
    const tinygltf::BufferView& bv = m.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = m.buffers[bv.buffer];
    const unsigned char* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
    out.resize(acc.count);
    switch (acc.componentType)
    {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        {
            const uint16_t* p = reinterpret_cast<const uint16_t*>(base);
            for (size_t i=0;i<acc.count;++i) out[i] = p[i];
            return true;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        {
            const uint8_t* p = reinterpret_cast<const uint8_t*>(base);
            for (size_t i=0;i<acc.count;++i) out[i] = p[i];
            return true;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        {
            const uint32_t* p = reinterpret_cast<const uint32_t*>(base);
            for (size_t i=0;i<acc.count;++i) out[i] = p[i];
            return true;
        }
        default: return false;
    }
}

static glm::mat4 nodeLocalMatrix(const tinygltf::Node& nd)
{
    glm::mat4 M(1.0f);
    if (!nd.matrix.empty() && nd.matrix.size() == 16)
    {
        // glTF stores column-major; glm is column-major
        M = glm::make_mat4(nd.matrix.data());
    }
    else
    {
        glm::mat4 T(1.0f), R(1.0f), S(1.0f);
        if (!nd.translation.empty())
            T = glm::translate(glm::mat4(1.0f), glm::vec3(nd.translation[0], nd.translation[1], nd.translation[2]));
        if (!nd.scale.empty())
            S = glm::scale(glm::mat4(1.0f), glm::vec3(nd.scale[0], nd.scale[1], nd.scale[2]));
        if (!nd.rotation.empty())
        {
            glm::quat q((float)nd.rotation[3], (float)nd.rotation[0], (float)nd.rotation[1], (float)nd.rotation[2]);
            R = glm::mat4_cast(q);
        }
        M = T * R * S;
    }
    return M;
}

bool Model::loadGLTF(const std::string& path)
{
    shutdown();
    err_.clear();

    tinygltf::Model gltf;
    tinygltf::TinyGLTF loader;
    std::string warn, err;

    const std::string ext = toLowerExt(path);
    bool ok = false;
    if (ext == ".glb") ok = loader.LoadBinaryFromFile(&gltf, &err, &warn, path);
    else ok = loader.LoadASCIIFromFile(&gltf, &err, &warn, path);
    if (!warn.empty()) {
        // ignore warnings silently
    }
    if (!ok) {
        err_ = err.empty() ? "Failed to load glTF" : err;
        return false;
    }

    std::vector<Vertex> verts;
    verts.reserve(50000);

    glm::vec3 bmin{ std::numeric_limits<float>::max() };
    glm::vec3 bmax{ std::numeric_limits<float>::lowest() };

    std::unordered_map<int, unsigned int> texCache; // gltf texture index -> GL id

    auto getOrCreateTexture = [&](int texIndex) -> unsigned int {
        if (texIndex < 0) return 0U;
        auto it = texCache.find(texIndex);
        if (it != texCache.end()) return it->second;
        const tinygltf::Texture& tex = gltf.textures[texIndex];
        if (tex.source < 0) { texCache[texIndex] = 0U; return 0U; }
        const tinygltf::Image& img = gltf.images[tex.source];
        GLenum fmt = GL_RGBA; int comp = img.component;
        if (comp == 3) fmt = GL_RGB; else fmt = GL_RGBA;
        unsigned int gltex = 0;
        glGenTextures(1, &gltex);
        glBindTexture(GL_TEXTURE_2D, gltex);
        // Sampler settings if present
        GLint minF = GL_LINEAR_MIPMAP_LINEAR, magF = GL_LINEAR;
        GLint wrapS = GL_REPEAT, wrapT = GL_REPEAT;
        if (tex.sampler >= 0 && tex.sampler < (int)gltf.samplers.size())
        {
            const tinygltf::Sampler& smp = gltf.samplers[tex.sampler];
            if (smp.minFilter != 0) minF = smp.minFilter;
            if (smp.magFilter != 0) magF = smp.magFilter;
            if (smp.wrapS != 0) wrapS = smp.wrapS;
            if (smp.wrapT != 0) wrapT = smp.wrapT;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        GLint internal = (fmt == GL_RGB) ? GL_SRGB8 : GL_SRGB8_ALPHA8;
        glTexImage2D(GL_TEXTURE_2D, 0, internal, img.width, img.height, 0, fmt, GL_UNSIGNED_BYTE, img.image.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        texCache[texIndex] = gltex;
        textures_.push_back(gltex);
        return gltex;
    };

    auto appendPrimitive = [&](const tinygltf::Primitive& prim, const glm::mat4& M)
    {
        if (prim.mode != TINYGLTF_MODE_TRIANGLES) return; // skip non-triangles
        auto itPos = prim.attributes.find("POSITION");
        if (itPos == prim.attributes.end()) return;
        const tinygltf::Accessor& accPos = gltf.accessors[itPos->second];
        if (accPos.type != TINYGLTF_TYPE_VEC3) return;
        std::vector<float> pos;
        if (!readAccessorFloatVecN(gltf, accPos, 3, pos)) return;

        // normals (optional)
        std::vector<float> nrm;
        auto itN = prim.attributes.find("NORMAL");
        if (itN != prim.attributes.end())
        {
            const tinygltf::Accessor& accN = gltf.accessors[itN->second];
            if (accN.type == TINYGLTF_TYPE_VEC3)
                readAccessorFloatVecN(gltf, accN, 3, nrm);
        }

        // color0 (optional, VEC3 or VEC4 float); default gray
        std::vector<float> col;
        int colN = 0;
        auto itC = prim.attributes.find("COLOR_0");
        if (itC != prim.attributes.end())
        {
            const tinygltf::Accessor& accC = gltf.accessors[itC->second];
            if (accC.type == TINYGLTF_TYPE_VEC3) { readAccessorFloatVecN(gltf, accC, 3, col); colN = 3; }
            else if (accC.type == TINYGLTF_TYPE_VEC4) { readAccessorFloatVecN(gltf, accC, 4, col); colN = 4; }
        }

        // UV sets
        std::vector<float> uv0, uv1;
        auto itUV0 = prim.attributes.find("TEXCOORD_0");
        if (itUV0 != prim.attributes.end())
        {
            const tinygltf::Accessor& accUV0 = gltf.accessors[itUV0->second];
            if (accUV0.type == TINYGLTF_TYPE_VEC2)
                readAccessorFloatVecN(gltf, accUV0, 2, uv0);
        }
        auto itUV1 = prim.attributes.find("TEXCOORD_1");
        if (itUV1 != prim.attributes.end())
        {
            const tinygltf::Accessor& accUV1 = gltf.accessors[itUV1->second];
            if (accUV1.type == TINYGLTF_TYPE_VEC2)
                readAccessorFloatVecN(gltf, accUV1, 2, uv1);
        }

        // indices optional
        std::vector<uint32_t> indices;
        bool hasIndices = prim.indices >= 0;
        if (hasIndices)
        {
            const tinygltf::Accessor& accI = gltf.accessors[prim.indices];
            readIndices(gltf, accI, indices);
        }

        auto getCol = [&](size_t i) {
            if (col.empty()) return glm::vec3(0.75f);
            if (colN == 3) return glm::vec3(col[3*i+0], col[3*i+1], col[3*i+2]);
            return glm::vec3(col[4*i+0], col[4*i+1], col[4*i+2]);
        };

        glm::mat3 Nmat = glm::transpose(glm::inverse(glm::mat3(M)));
        auto getN = [&](size_t i, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c){
            glm::vec3 n;
            if (!nrm.empty()) n = glm::vec3(nrm[3*i+0], nrm[3*i+1], nrm[3*i+2]);
            else {
                glm::vec3 e1 = b - a, e2 = c - a;
                glm::vec3 nn = glm::cross(e1, e2);
                float len = glm::length(nn); n = (len>1e-10f)?(nn/len):glm::vec3(0,1,0);
            }
            n = glm::normalize(Nmat * n);
            return n;
        };

        // Determine texture, texCoord set, and KHR_texture_transform if present
        unsigned int gltex = 0;
        int uvSet = 0; // which TEXCOORD_n to use
        glm::vec2 uvScale(1.0f, 1.0f), uvOffset(0.0f, 0.0f);
        float uvRotate = 0.0f;
        bool doBlend = false;
        glm::vec4 baseColorFactor(1.0f);
        if (prim.material >= 0 && prim.material < (int)gltf.materials.size())
        {
            const tinygltf::Material& mat = gltf.materials[prim.material];
            int tindex = -1;
            const auto& bct = mat.pbrMetallicRoughness.baseColorTexture;
            if (bct.index >= 0) tindex = bct.index;
            auto itv = mat.values.find("baseColorTexture");
            if (tindex < 0 && itv != mat.values.end() && itv->second.TextureIndex() >= 0)
                tindex = itv->second.TextureIndex();
            // baseColorFactor
            if (!mat.pbrMetallicRoughness.baseColorFactor.empty() && mat.pbrMetallicRoughness.baseColorFactor.size() >= 4)
            {
                const auto& f = mat.pbrMetallicRoughness.baseColorFactor;
                baseColorFactor = glm::vec4((float)f[0], (float)f[1], (float)f[2], (float)f[3]);
            }
            // alpha mode
            if (mat.alphaMode == "BLEND") doBlend = true;
            // texCoord set
            if (bct.texCoord >= 0) uvSet = bct.texCoord;
            // KHR_texture_transform
            auto extIt = bct.extensions.find("KHR_texture_transform");
            if (extIt != bct.extensions.end())
            {
                const tinygltf::Value& ext = extIt->second;
                if (ext.Has("scale"))
                {
                    const auto& a = ext.Get("scale");
                    if (a.IsArray() && a.ArrayLen() >= 2)
                        uvScale = glm::vec2((float)a.Get(0).GetNumberAsDouble(), (float)a.Get(1).GetNumberAsDouble());
                }
                if (ext.Has("offset"))
                {
                    const auto& a = ext.Get("offset");
                    if (a.IsArray() && a.ArrayLen() >= 2)
                        uvOffset = glm::vec2((float)a.Get(0).GetNumberAsDouble(), (float)a.Get(1).GetNumberAsDouble());
                }
                if (ext.Has("rotation"))
                {
                    uvRotate = (float)ext.Get("rotation").GetNumberAsDouble();
                }
                if (ext.Has("texCoord"))
                {
                    uvSet = (int)ext.Get("texCoord").GetNumberAsInt();
                }
            }
            gltex = getOrCreateTexture(tindex);
        }

        auto applyUVXform = [&](glm::vec2 uv){
            uv *= uvScale;
            if (uvRotate != 0.0f)
            {
                float c = cosf(uvRotate), s = sinf(uvRotate);
                uv = glm::vec2(c*uv.x - s*uv.y, s*uv.x + c*uv.y);
            }
            uv += uvOffset;
            return uv;
        };

        auto emitTri = [&](uint32_t i0, uint32_t i1, uint32_t i2){
            glm::vec3 lp0{ pos[3*i0+0], pos[3*i0+1], pos[3*i0+2] };
            glm::vec3 lp1{ pos[3*i1+0], pos[3*i1+1], pos[3*i1+2] };
            glm::vec3 lp2{ pos[3*i2+0], pos[3*i2+1], pos[3*i2+2] };
            glm::vec3 p0 = glm::vec3(M * glm::vec4(lp0, 1.0f));
            glm::vec3 p1 = glm::vec3(M * glm::vec4(lp1, 1.0f));
            glm::vec3 p2 = glm::vec3(M * glm::vec4(lp2, 1.0f));
            glm::vec3 n0 = getN(i0, p0, p1, p2);
            glm::vec3 n1 = getN(i1, p0, p1, p2);
            glm::vec3 n2 = getN(i2, p0, p1, p2);
            glm::vec3 c0 = getCol(i0);
            glm::vec3 c1 = getCol(i1);
            glm::vec3 c2 = getCol(i2);
            auto getUV = [&](size_t i){
                const std::vector<float>& src = (uvSet == 1 && !uv1.empty()) ? uv1 : uv0;
                if (src.empty()) return glm::vec2(0.0f);
                return glm::vec2(src[2*i+0], src[2*i+1]);
            };
            glm::vec2 t0 = applyUVXform(getUV(i0));
            glm::vec2 t1 = applyUVXform(getUV(i1));
            glm::vec2 t2 = applyUVXform(getUV(i2));
            verts.push_back({p0,n0,c0,t0});
            verts.push_back({p1,n1,c1,t1});
            verts.push_back({p2,n2,c2,t2});
            // bounds
            const glm::vec3 pp[3] = {p0,p1,p2};
            for (int j=0;j<3;++j) {
                bmin.x = std::min(bmin.x, pp[j].x); bmax.x = std::max(bmax.x, pp[j].x);
                bmin.y = std::min(bmin.y, pp[j].y); bmax.y = std::max(bmax.y, pp[j].y);
                bmin.z = std::min(bmin.z, pp[j].z); bmax.z = std::max(bmax.z, pp[j].z);
            }
        };

        int vertStart = (int)verts.size();
        if (hasIndices)
        {
            for (size_t i=0;i+2<indices.size(); i+=3)
                emitTri(indices[i+0], indices[i+1], indices[i+2]);
        }
        else
        {
            const size_t vcount = pos.size()/3;
            for (size_t i=0;i+2<vcount; i+=3)
                emitTri((uint32_t)i, (uint32_t)i+1, (uint32_t)i+2);
        }

        int added = (int)verts.size() - vertStart;
        if (added > 0)
            draws_.push_back({vertStart, added, gltex, doBlend, baseColorFactor});
    };

    // Iterate default scene nodes and gather all primitives
    auto traverse = [&](auto&& self, int nodeIndex, const glm::mat4& parentM) -> void {
        const tinygltf::Node& nd = gltf.nodes[nodeIndex];
        glm::mat4 local = nodeLocalMatrix(nd);
        glm::mat4 M = parentM * local;
        if (nd.mesh >= 0)
        {
            const tinygltf::Mesh& mesh = gltf.meshes[nd.mesh];
            for (const auto& prim : mesh.primitives) appendPrimitive(prim, M);
        }
        for (int c : nd.children) self(self, c, M);
    };

    if (gltf.scenes.empty())
    {
        // Fallback: iterate all meshes without transforms
        for (const auto& mesh : gltf.meshes)
            for (const auto& prim : mesh.primitives) appendPrimitive(prim, glm::mat4(1.0f));
    }
    else
    {
        int sceneIndex = gltf.defaultScene >= 0 ? gltf.defaultScene : 0;
        const tinygltf::Scene& sc = gltf.scenes[sceneIndex];
        for (int nodeIndex : sc.nodes)
            traverse(traverse, nodeIndex, glm::mat4(1.0f));
    }

    if (verts.empty()) { err_ = "No triangles found in glTF."; return false; }

    // upload to GPU (same as OBJ path)
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    const GLsizei stride = sizeof(Vertex);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, stride, (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, nrm));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, col));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);

    vertexCount_ = static_cast<int>(verts.size());
    bmin_ = bmin; bmax_ = bmax;
    return true;
}

void Model::computeFlatNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, glm::vec3& n) 
{
    glm::vec3 e1 = b - a, e2 = c - a;
    glm::vec3 nn = glm::cross(e1, e2);
    float len = glm::length(nn);
    n = (len > 1e-10f) ? (nn / len) : glm::vec3(0, 1, 0);
}

void Model::render(const Camera& cam, const glm::mat4& model, Shader& shader) const 
{
    if (!vao_ || vertexCount_ <= 0) return;

    // You already set uniforms in your scenes; we set them here for convenience:
    // expect shader "uModel/uView/uProj/uNormalMat/uLightDir/uViewPos/uUseLighting"
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
    glUniformMatrix4fv(shader.loc("uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(shader.loc("uView"), 1, GL_FALSE, glm::value_ptr(cam.view()));
    glUniformMatrix4fv(shader.loc("uProj"), 1, GL_FALSE, glm::value_ptr(cam.proj()));
    glUniformMatrix3fv(shader.loc("uNormalMat"), 1, GL_FALSE, glm::value_ptr(normalMat));

    glBindVertexArray(vao_);
    if (draws_.empty())
    {
        glUniform1i(shader.loc("uHasBaseColorTex"), 0);
        glUniform4f(shader.loc("uBaseColorFactor"), 1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    }
    else
    {
        // Pass 1: opaque (no blending, depth writes on)
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        for (const auto& d : draws_)
        {
            if (d.blend) continue;
            if (d.tex)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, d.tex);
                glUniform1i(shader.loc("uBaseColorTex"), 0);
                glUniform1i(shader.loc("uHasBaseColorTex"), 1);
            }
            else
            {
                glUniform1i(shader.loc("uHasBaseColorTex"), 0);
            }
            glUniform4f(shader.loc("uBaseColorFactor"), d.baseColorFactor.r, d.baseColorFactor.g, d.baseColorFactor.b, d.baseColorFactor.a);
            glDrawArrays(GL_TRIANGLES, d.first, d.count);
        }

        // Pass 2: transparent (enable blending, depth writes off)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for (const auto& d : draws_)
        {
            if (!d.blend) continue;
            if (d.tex)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, d.tex);
                glUniform1i(shader.loc("uBaseColorTex"), 0);
                glUniform1i(shader.loc("uHasBaseColorTex"), 1);
            }
            else
            {
                glUniform1i(shader.loc("uHasBaseColorTex"), 0);
            }
            glUniform4f(shader.loc("uBaseColorFactor"), d.baseColorFactor.r, d.baseColorFactor.g, d.baseColorFactor.b, d.baseColorFactor.a);
            glDrawArrays(GL_TRIANGLES, d.first, d.count);
        }
        // Restore state
        glBindTexture(GL_TEXTURE_2D, 0);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    glBindVertexArray(0);
}
