#include "gfx/Model.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tiny_obj_loader.h>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <cctype>
#include <algorithm>

// tinygltf: header-only glTF 2.0 loader (geometry only here)
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
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
    vertexCount_ = 0;
}

bool Model::loadOBJ(const std::string& path) 
{
    shutdown();
    err_.clear();

    tinyobj::ObjReaderConfig cfg;
    cfg.triangulate = true;
    cfg.vertex_color = true; // if present

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path, cfg)) 
    {
        err_ = reader.Error().empty() ? "Failed to read .obj" : reader.Error();
        return false;
    }
    if (!reader.Warning().empty()) 
    {
        
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    std::vector<Vertex> verts;
    verts.reserve(50000);

    // init AABB
    glm::vec3 bmin{ std::numeric_limits<float>::max() };
    glm::vec3 bmax{ std::numeric_limits<float>::lowest() };

    for (const auto& sh : shapes) 
    {
        size_t index_offset = 0;
        for (size_t f = 0; f < sh.mesh.num_face_vertices.size(); f++) 
        {
            const int fv = sh.mesh.num_face_vertices[f]; // should be 3 due to triangulate
            if (fv != 3) 
            { 
                index_offset += fv; 
                continue; 
            }

            glm::vec3 pos[3]; glm::vec3 nrm[3]; glm::vec3 col[3];
            bool hasN[3] = {false, false, false};
            bool hasC[3] = {false, false, false};

            for (int v = 0; v < 3; v++) 
            {
                tinyobj::index_t idx = sh.mesh.indices[index_offset + v];

                pos[v].x = attrib.vertices[3 * idx.vertex_index + 0];
                pos[v].y = attrib.vertices[3 * idx.vertex_index + 1];
                pos[v].z = attrib.vertices[3 * idx.vertex_index + 2];

                if (idx.normal_index >= 0) 
                {
                    hasN[v] = true;
                    nrm[v].x = attrib.normals[3 * idx.normal_index + 0];
                    nrm[v].y = attrib.normals[3 * idx.normal_index + 1];
                    nrm[v].z = attrib.normals[3 * idx.normal_index + 2];
                }

                if (!attrib.colors.empty() && idx.vertex_index >= 0) 
                {
                    hasC[v] = true;
                    col[v].r = attrib.colors[3 * idx.vertex_index + 0];
                    col[v].g = attrib.colors[3 * idx.vertex_index + 1];
                    col[v].b = attrib.colors[3 * idx.vertex_index + 2];
                } 
                else 
                {
                    col[v] = glm::vec3(0.75f); // default gray
                }
            }

            // if any normal missing → compute a flat face normal
            if (!(hasN[0] && hasN[1] && hasN[2])) 
            {
                glm::vec3 fn; computeFlatNormal(pos[0], pos[1], pos[2], fn);
                nrm[0] = nrm[1] = nrm[2] = fn;
            }

            // append 3 vertices
            for (int v = 0; v < 3; ++v) 
            {
                verts.push_back({pos[v], nrm[v], col[v]});
                // expand bounds
                bmin.x = std::min(bmin.x, pos[v].x); bmax.x = std::max(bmax.x, pos[v].x);
                bmin.y = std::min(bmin.y, pos[v].y); bmax.y = std::max(bmax.y, pos[v].y);
                bmin.z = std::min(bmin.z, pos[v].z); bmax.z = std::max(bmax.z, pos[v].z);
            }

            index_offset += fv;
        }
    }

    if (verts.empty()) 
    { 
        err_ = "No vertices parsed from OBJ."; 
        return false; 
    }

    // upload to GPU
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    const GLsizei stride = sizeof(Vertex);
    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, stride, (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, nrm));
    glEnableVertexAttribArray(2); // color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, col));
    glBindVertexArray(0);

    vertexCount_ = static_cast<int>(verts.size());
    bmin_ = bmin; 
    bmax_ = bmax;

    return true;
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
    if (ext == ".obj")
        return loadOBJ(path);
    if (ext == ".gltf" || ext == ".glb")
        return loadGLTF(path);
    err_ = "Unsupported file extension: " + ext;
    return false;
}

static bool readAccessorFloatVecN(const tinygltf::Model& m,
                                  const tinygltf::Accessor& acc,
                                  int N,
                                  std::vector<float>& out)
{
    const tinygltf::BufferView& bv = m.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = m.buffers[bv.buffer];

    if (acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) return false;
    const size_t count = acc.count;

    size_t stride = N * sizeof(float);
#ifdef TINYGLTF_ENABLE_DRACO
    // Not enabling DRACO; skip.
#endif
    if (bv.byteStride) stride = bv.byteStride;

    const size_t start = bv.byteOffset + acc.byteOffset;
    const unsigned char* base = buf.data.data() + start;

    out.resize(count * N);
    for (size_t i = 0; i < count; ++i)
    {
        const float* src = reinterpret_cast<const float*>(base + i * stride);
        for (int k = 0; k < N; ++k) out[i * N + k] = src[k];
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
            if (accC.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                if (accC.type == TINYGLTF_TYPE_VEC3) { readAccessorFloatVecN(gltf, accC, 3, col); colN = 3; }
                else if (accC.type == TINYGLTF_TYPE_VEC4) { readAccessorFloatVecN(gltf, accC, 4, col); colN = 4; }
            }
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
            verts.push_back({p0,n0,c0});
            verts.push_back({p1,n1,c1});
            verts.push_back({p2,n2,c2});
            // bounds
            const glm::vec3 pp[3] = {p0,p1,p2};
            for (int j=0;j<3;++j) {
                bmin.x = std::min(bmin.x, pp[j].x); bmax.x = std::max(bmax.x, pp[j].x);
                bmin.y = std::min(bmin.y, pp[j].y); bmax.y = std::max(bmax.y, pp[j].y);
                bmin.z = std::min(bmin.z, pp[j].z); bmax.z = std::max(bmax.z, pp[j].z);
            }
        };

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
    glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    glBindVertexArray(0);
}
