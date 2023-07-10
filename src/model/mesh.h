#pragma once

// internal
#include "texture/texture.h"
#include "util/buffer.h"
#include "global.h"

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
public:
    // Loads from a json file, which stores each frame of animation
    virtual void load_model(const std::string& path) = 0;

    // Loads the texture to the batch renderer if needed
    virtual void load_texture(const std::string& path) = 0;

    // Puts this model into a batch
    virtual void upload() = 0;

    // Setters  
    virtual void set_batchmanager(BatchManager* br) = 0;
    virtual void set_modelmat(const glm::mat4& mat) = 0;

    // Getters 
    virtual Buffer<uint8_t> get_model_buffer() = 0;
    virtual Buffer<uint8_t> get_vertex_buffer() = 0;
    virtual Buffer<uint8_t> get_index_buffer() = 0;

    // Animation, done by changing the index buffer
    virtual size_t animation_start() = 0;
    virtual size_t animation_length() = 0;
};

// One mesh to be rendered (has one texture and one set of buffers)
class StandardMesh : public Mesh
{
private:
    // The batch renderer for this mesh, when we do a draw should we load it to the renderer
    BatchManager* br;

    // The batch that this mesh is in, as well as the index pointer for that batch
    Batch* batch = nullptr;
    size_t* index = nullptr;

    // Buffer data
    cgltf_data* data;
    Vertex* vertices;
    uint16_t* indices;

    // Buffer size
    size_t vertices_size;
    size_t indices_size;

    // Texture id  
    uint32_t texture_id;

    // Model matrix 
    glm::mat4 modelmat = glm::mat4(1.0f);

    // A buffer that combines all of the overall data for this model
    // ie model matrix, texture id, etc.
    std::vector<uint8_t> model_buffer;
public:
    StandardMesh();
    ~StandardMesh();

    virtual void load_model(const std::string& path);
    virtual void load_texture(const std::string& path);

    virtual void set_modelmat(const glm::mat4& mat);
    virtual void set_batchmanager(BatchManager* br);

    virtual void upload();

    virtual Buffer<uint8_t> get_model_buffer();
    virtual Buffer<uint8_t> get_vertex_buffer();
    virtual Buffer<uint8_t> get_index_buffer();

    virtual size_t animation_start() { return 0; }
    virtual size_t animation_length() { return indices_size; }
};
