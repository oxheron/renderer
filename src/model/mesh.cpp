#include "mesh.h"

// external
#include <nlohmann/json.hpp>

// std
#include <stdexcept>
#include <tuple>
#include <fstream>

using json = nlohmann::json;

void Mesh::load_data(const std::string& path)
{   
    vertices.clear();
    indices.clear();

    std::fstream file(path);
    json data = json::parse(file); 

    if (!data.contains("texture")) throw std::runtime_error("Invalid json file loaded");
    this->texture_path = data["texture"].get<std::string>();

    // Load the actual animation data
    json frame_paths = data["animation_frames"];
    for (auto& [key, value] : frame_paths.items())
    {
        load_animation(key, value);
    }
}

void Mesh::load_animation(const std::string& identifier, const std::string& path)
{
    cgltf_data* data;
    cgltf_options options = cgltf_options();
    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
    options = cgltf_options();
    cgltf_load_buffers(&options, data, path.c_str());

    // Validate buffer
    int do_more_error_checking = 0;

    if (data->buffer_views[0].size / sizeof(glm::vec3) != data->buffer_views[1].size / sizeof(glm::vec2) 
            || data->buffer_views[0].size / sizeof(glm::vec3) != data->buffer_views[2].size / sizeof(glm::vec3)) 
        throw std::runtime_error("Bad buffer data in gltf file");

    // Indices can either be 16 bit or 32 bit
    uint32_t sizeof_index = data->accessors[3].component_type == cgltf_component_type_r_16u ? sizeof(uint16_t) : sizeof(uint32_t);

    // Parse buffers into model
    size_t vertices_size = (data->buffer_views[0].size + data->buffer_views[1].size + data->buffer_views[2].size) / sizeof(Vertex);
    size_t indices_size = data->buffer_views[3].size / sizeof_index;

    vertices.resize(vertices_size + vertices.size());
    indices.resize(indices_size + indices.size());

    // Copy vertex data
    glm::vec3* position_pointer = (glm::vec3*) ((char*) data->buffers->data + data->buffer_views[0].offset);
    glm::vec2* uv_pointer = (glm::vec2*) ((char*) data->buffers->data + data->buffer_views[1].offset);
    glm::vec3* normal_pointer = (glm::vec3*) ((char*) data->buffers->data + data->buffer_views[2].offset);
    
    for (size_t i = 0; i < data->buffer_views[0].size / sizeof(glm::vec3); i++)
    {
        vertices[i].position = position_pointer[i];
        vertices[i].uv = uv_pointer[i];
        vertices[i].normal = normal_pointer[i];
    }

    for (size_t i = 0; i < indices_size; i++)
    {
        if (sizeof_index == sizeof(uint16_t)) indices[i] = ((uint16_t*) ((char*) data->buffers->data + data->buffer_views[3].offset))[i];
        else indices[i] = ((uint32_t*) ((char*) data->buffers->data + data->buffer_views[3].offset))[i];
    }

    animation_frames[identifier] = {indices.size() - indices_size - 1, indices_size};

    cgltf_free(data);
}

StandardModel::StandardModel()
{
    // Potentially allow customizable layouts
}

StandardModel::~StandardModel()
{
}

void StandardModel::load_mesh(const std::string& path)
{
    mesh.load_data(path);
    load_texture(mesh.get_texture());
}

void StandardModel::load_texture(const std::string& path)
{
    if (!br) return;
    this->texture_id = br->load_texture(path);
}

void StandardModel::set_batchmanager(BatchManager* br)
{
    this->br = br;
}

// Set the model matrix
// This is used to transform the model
// For particle modeles this could be used to set the overall position but there may be multiple
// Also edit the value in the batch
void StandardModel::set_modelmat(const glm::mat4& mat)
{
    modelmat = mat;
    if (batch) batch->edit_model_data(this, index);
}

void StandardModel::upload()
{
    // Load to batch renderer
    auto [batch, index] = br->add(this);
    this->batch = batch;
    this->index = index;
}

Buffer<uint8_t> StandardModel::get_model_buffer()
{
    // Make sure the stride is a multiple of 16 (20 in this case)
    if (model_buffer.size() == 0) model_buffer.resize(16 * sizeof(float) + sizeof(float) * 4);
    memcpy((void*) model_buffer.data(), (void*) glm::value_ptr(modelmat), 16 * sizeof(float));
    float* tex_id = (float*) (model_buffer.data() + 16 * sizeof(float));
    *tex_id = (float) texture_id;
    return Buffer(model_buffer.data(), model_buffer.size());
}

BaseInstance::BaseInstance()
{

}

BaseInstance::~BaseInstance()
{
    if (batch) batch->remove_instance_data(instance_index);
}

void BaseInstance::load_mesh(const std::string& path)
{
    mesh.load_data(path);
    load_texture(mesh.get_texture());
}

void BaseInstance::load_texture(const std::string& path)
{
    if (!br) return;
    this->texture_id = br->load_texture(path);
}

void BaseInstance::upload()
{
    if (this->mesh.get_vertices().size() == 0) return;
    auto [batch, index] = br->add_instance_data(this->get_vertex_buffer(), this->get_index_buffer());
    this->batch = batch;
    this->instance_index = index;
}

InstancedModel::InstancedModel(BaseInstance* base)
{
    this->base = base;
    this->index = 0;
}

InstancedModel::~InstancedModel()
{
    base->get_batch()->remove_instance(index);
}

void InstancedModel::upload()
{
    if (base->get_batch() == nullptr) base->upload();

    index = base->get_batch()->add_instance(this, base->get_index()); 
}

void InstancedModel::set_modelmat(const glm::mat4& mat)
{
    modelmat = mat;
    if (base->get_batch()) base->get_batch()->edit_model_data(this, index);
}

Buffer<uint8_t> InstancedModel::get_model_buffer()
{
    if (model_buffer.size() == 0) model_buffer.resize(16 * sizeof(float) + sizeof(float) * 4);
    memcpy((void*) model_buffer.data(), (void*) glm::value_ptr(modelmat), 16 * sizeof(float));
    float* tex_id = (float*) (model_buffer.data() + 16 * sizeof(float));
    *tex_id = (float) base->get_texture_id();
    return Buffer(model_buffer.data(), model_buffer.size());
}

Buffer<uint8_t> InstancedModel::get_vertex_buffer()
{
    return base->get_vertex_buffer();
}

Buffer<uint8_t> InstancedModel::get_index_buffer()
{
    return base->get_index_buffer();
}
