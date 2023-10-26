#pragma once

// internal
#include "texture/texture.h"
#include "util/buffer.h"
#include "global.h"
#include "renderer/batchmanager.h"

// external
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <cgltf/cgltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// std
#include <string> 
#include <vector>

class Vertex
{
public:
    glm::vec3 position; 
    glm::vec2 uv;
    glm::vec3 normal;
};

class Mesh
{
private:
    // Buffer data
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Texture path 
    std::string texture_path;

    // Animation data
    std::unordered_map<std::string, std::pair<size_t, size_t>> animation_frames;
public:
    // Loads the model from a json file
    void load_data(const std::string& path);

    // Get the texture path to load into the batch manager
    std::string get_texture() { return texture_path; }

    // Get the vertices and indices
    Buffer<uint8_t> get_vertices() { return Buffer<uint8_t>((uint8_t*) vertices.data(), vertices.size() * sizeof(Vertex)); }
    Buffer<uint8_t> get_indices() { return Buffer<uint8_t>((uint8_t*) indices.data(), indices.size() * sizeof(uint32_t)); }
private:
    // Adds one animation frame to the model
    void load_animation(const std::string& identifier, const std::string& path);
};

class Model
{
public:
    // Puts this model into a batch
    virtual void upload() = 0;

    // Setters  
    virtual void set_modelmat(const glm::mat4& mat) = 0;

    // Getters 
    virtual Buffer<uint8_t> get_model_buffer() = 0;
    virtual Buffer<uint8_t> get_vertex_buffer() = 0;
    virtual Buffer<uint8_t> get_index_buffer() = 0;

    // Animation, done by changing the index buffer
    virtual size_t animation_start() = 0;
    virtual size_t animation_length() = 0;
};

// One model to be rendered (has one texture and one set of buffers)
class StandardModel : public Model
{
private:
    // The batch renderer for this model, when we do a draw should we load it to the renderer
    BatchManager* br;

    // The batch that this model is in, as well as the index pointer for that batch
    Batch* batch = nullptr;
    size_t index = SIZE_MAX;
    
    // The mesh that holds all of the data (vertices, indices, etc) for this model
    Mesh mesh;

    // Texture id  
    uint32_t texture_id;

    // Model matrix 
    glm::mat4 modelmat = glm::mat4(1.0f);

    // A buffer that combines all of the overall data for this model
    // ie model matrix, texture id, etc.
    std::vector<uint8_t> model_buffer;
public:
    StandardModel();
    ~StandardModel();

    virtual void load_mesh(const std::string& path);
    virtual void load_texture(const std::string& path);

    virtual void set_modelmat(const glm::mat4& mat);
    virtual void set_batchmanager(BatchManager* br);

    virtual void upload();

    virtual Buffer<uint8_t> get_model_buffer();
    virtual Buffer<uint8_t> get_vertex_buffer() { return mesh.get_vertices(); }
    virtual Buffer<uint8_t> get_index_buffer() { return mesh.get_indices(); }

    virtual size_t animation_start() { return 0; }
    virtual size_t animation_length() { return mesh.get_indices().size() / sizeof(uint32_t); }

    Batch* get_batch() { return batch; }
    size_t get_index() { return index; }
};

class BaseInstance
{
private:
    // The batch and batchmanager for this instance, but not the index
    Batch* batch = nullptr;
    BatchManager* br = nullptr;

    // An index into the allocation data of the batch 
    size_t instance_index;

    // All of the mesh data
    Mesh mesh;

    // Texture id
    uint32_t texture_id;
 public:  
    BaseInstance();
    ~BaseInstance();
    
    // Loads the model for this instance
    void load_mesh(const std::string& path);
    
    // Loads the texture for this instance
    void load_texture(const std::string& path);

    // Uploads the instance data to the batch
    void upload();
    
    // Set the batchmanager for this instance
    void set_batchmanager(BatchManager* br) { this->br = br; }

    // Getters 
    BatchManager* get_batchmanager() { return br; }
    Batch* get_batch() { return batch; }
    uint32_t get_texture_id() { return texture_id; }
    Buffer<uint8_t> get_vertex_buffer() { return mesh.get_vertices(); } 
    Buffer<uint8_t> get_index_buffer() { return mesh.get_indices(); } 
    size_t get_index() { return instance_index; }
};

class InstancedModel : public Model
{
private:
    BaseInstance* base;

    // The model index into the batch for this model
    size_t index;

    // Model matrix 
    glm::mat4 modelmat = glm::mat4(1.0f);

    // A buffer that combines all of the overall data for this model
    // ie model matrix, texture id, etc.
    std::vector<uint8_t> model_buffer;
public:
    InstancedModel(BaseInstance* base);
    ~InstancedModel();

    // Puts this model into a batch
    virtual void upload();

    // Setters  
    virtual void set_modelmat(const glm::mat4& mat);

    // Getters 
    virtual Buffer<uint8_t> get_model_buffer();
    virtual Buffer<uint8_t> get_vertex_buffer();
    virtual Buffer<uint8_t> get_index_buffer();

    // Animation, done by changing the index buffer
    virtual size_t animation_start() { return 0; }
    virtual size_t animation_length() { return get_index_buffer().size() / sizeof(uint32_t); }
};
