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
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>
#include <robin-hood/robin-hood.h>

// std
#include <optional>
#include <string> 
#include <vector>
#include <fstream>

struct Vertex
{
    glm::vec3 position; 
    glm::vec2 uv;
    glm::vec3 normal;
};

struct PosOnly
{
    glm::vec3 position;
};

template <typename T> 
class Mesh
{
private:
    // Buffer data
    std::vector<T> vertices;
    std::vector<uint32_t> indices;

    // Texture path 
    std::optional<std::string> texture_path;

    // Animation data
    robin_hood::unordered_map<std::string, std::pair<size_t, size_t>> 
        animation_frames;
public:
    Mesh() 
    {
        this->texture_path = std::nullopt;
    }
    
    // Loads the model from a json file
    void load_data(const std::string& path)
    {   
        vertices.clear();
        indices.clear();

        std::fstream file(path);
        nlohmann::json data = nlohmann::json::parse(file); 

        if (!data.contains("texture")) 
            throw std::runtime_error("Invalid json file loaded");
        this->texture_path = data["texture"].get<std::string>();

        // Load the actual animation data
        if (!data.contains("animation_frames")) 
            throw std::runtime_error("Invalid json file loaded (no animation_frames)");
        nlohmann::json frame_paths = data["animation_frames"];
        for (auto& [key, value] : frame_paths.items())
        {
            load_animation(key, value);
        }
    }

    // Get the texture path to load into the batch manager
    std::optional<std::string> get_texture() { return texture_path; }

    // Get the vertices and indices
    Buffer<uint8_t> get_vertices() { 
        return Buffer<uint8_t>((uint8_t*) vertices.data(), 
            vertices.size() * sizeof(Vertex)); }
    Buffer<uint8_t> get_indices() { 
        return Buffer<uint8_t>((uint8_t*) indices.data(), 
            indices.size() * sizeof(uint32_t)); }
private:
    // Adds one animation frame to the model
    void load_animation(const std::string& identifier, const std::string& path);
};

class Model
{
public:
    // Puts this model into a batch
    virtual void upload(BatchManager*) = 0;

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
    Batch* batch = nullptr;
    size_t index = SIZE_MAX;
    
    Mesh<Vertex> mesh;

    uint32_t texture_id;

    glm::mat4 modelmat = glm::mat4(1.0f);

    std::vector<uint8_t> model_buffer;
public:
    StandardModel();
    ~StandardModel();

    virtual void load_mesh(const std::string& path);
    virtual void load_texture(TextureAtlas* atlas);

    virtual void set_modelmat(const glm::mat4& mat);

    virtual void upload(BatchManager* batchmanager);

    virtual Buffer<uint8_t> get_model_buffer();
    virtual Buffer<uint8_t> get_vertex_buffer() { return mesh.get_vertices(); }
    virtual Buffer<uint8_t> get_index_buffer() { return mesh.get_indices(); }

    virtual size_t animation_start() { return 0; }
    virtual size_t animation_length() { return mesh.get_indices().size() / sizeof(uint32_t); }

    Batch* get_batch() { return batch; }
    size_t get_index() { return index; }
};

template <typename T>
class BaseInstance
{
protected:
    Batch* batch = nullptr;

    // Where vertices & indicies are in the batch
    size_t instance_index = SIZE_MAX;

    Mesh<T> mesh;
 public:  
    BaseInstance() = default;
    ~BaseInstance() 
    {
        if (batch) batch->remove_instance_data(instance_index);
    }
    
    void load_mesh(const std::string& path) { mesh.load_data(path); }

    void upload(BatchManager* batchmanager) 
    {
        if (this->mesh.get_vertices().size() == 0) return;
        auto [batch, index] = batchmanager->add_instance_data(this->get_vertex_buffer(), 
            this->get_index_buffer());
        this->batch = batch;
        this->instance_index = index;
    }

    Batch* get_batch() { return batch; }
    Buffer<uint8_t> get_vertex_buffer() { return mesh.get_vertices(); } 
    Buffer<uint8_t> get_index_buffer() { return mesh.get_indices(); } 
    size_t get_index() { return instance_index; }
};

class TextureInstance : public BaseInstance<Vertex>
{
private:
    uint32_t texture_id;

public:
    void load_texture(TextureAtlas* atlas);

    uint32_t get_texture_id() { return texture_id; }
};

class InstancedModel : public Model
{
private:
    TextureInstance* base;

    size_t obj_index = SIZE_MAX;

    // Model matrix 
    glm::mat4 modelmat = glm::mat4(1.0f);

    // A buffer that combines all of the overall data for this model
    // ie model matrix, texture id, etc.
    std::vector<uint8_t> model_buffer;
public:
    InstancedModel(TextureInstance* base);
    ~InstancedModel();

    // Puts this model into a batch
    virtual void upload(BatchManager* batchmanager = nullptr);

    // Setters  
    virtual void set_modelmat(const glm::mat4& mat);

    // Getters 
    virtual Buffer<uint8_t> get_model_buffer();
    virtual Buffer<uint8_t> get_vertex_buffer();
    virtual Buffer<uint8_t> get_index_buffer();

    // Animation, done by changing the index buffer
    virtual size_t animation_start() { return 0; }
    virtual size_t animation_length() { 
        return get_index_buffer().size() / sizeof(uint32_t); }
};
