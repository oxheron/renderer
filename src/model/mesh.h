#pragma once

// internal
#include "texture/texture.h"

// external
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <cgltf/cgltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Vertex
{
public:
    glm::vec3 position; 
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh
{
    virtual void load_model(const std::string& path) = 0;
    virtual void load_texture(const std::string& path) = 0;
    virtual void set_handles() = 0;
    virtual void draw(bgfx::ProgramHandle& prg_handle) = 0;
    virtual void set_modelmat(const glm::mat4& mat) = 0;
};

// One mesh to be rendered (has one texture and one set of buffers)
class StandardMesh : public Mesh
{
private:
    // Buffer data
    cgltf_data* data;
    Vertex* vertices;
    uint16_t* indices;

    // Buffer size
    size_t vertices_size;
    size_t indices_size;

    // Buffer handles
    bgfx::VertexLayout layout;
    bgfx::VertexBufferHandle vbh;
    bgfx::IndexBufferHandle ibh;

    // Texture
    Texture texture;

    // Model matrix 
    glm::mat4 modelmat = glm::mat4(1.0f);
public:
    StandardMesh();
    ~StandardMesh();

    virtual void load_model(const std::string& path);
    virtual void load_texture(const std::string& path);
    virtual void set_handles();
    virtual void set_modelmat(const glm::mat4& mat);

    // Simple draw method for now, eventually will pass on to batch renderer (per shader)
    virtual void draw(bgfx::ProgramHandle& prg_handle);
};

