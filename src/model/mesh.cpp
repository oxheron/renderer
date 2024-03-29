#include "mesh.h"

// std
#include <stdexcept>
#include <tuple>

template <> 
void Mesh<Vertex>::load_animation(const std::string& identifier, const std::string& path)
{
    cgltf_data* data;
    cgltf_options options = cgltf_options();
    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
    options = cgltf_options();
    cgltf_load_buffers(&options, data, path.c_str());

    // Validate buffer
    int do_more_error_checking = 0;

    if (data->buffer_views[0].size / sizeof(glm::vec3) != 
            data->buffer_views[1].size / sizeof(glm::vec2) 
            || data->buffer_views[0].size / sizeof(glm::vec3) != 
            data->buffer_views[2].size / sizeof(glm::vec3)) 
        throw std::runtime_error("Bad buffer data in gltf file");

    // Indices can either be 16 bit or 32 bit
    uint32_t sizeof_index = 
        data->accessors[3].component_type == cgltf_component_type_r_16u ? 
        sizeof(uint16_t) : sizeof(uint32_t);

    // Parse buffers into model
    size_t vertices_size = 
        (data->buffer_views[0].size + data->buffer_views[1].size + 
         data->buffer_views[2].size) / sizeof(Vertex);
    size_t indices_size = data->buffer_views[3].size / sizeof_index;

    vertices.resize(vertices_size + vertices.size());
    indices.resize(indices_size + indices.size());

    // Copy vertex data
    glm::vec3* position_pointer = 
        (glm::vec3*) ((char*) data->buffers->data + data->buffer_views[0].offset);
    glm::vec2* uv_pointer = 
        (glm::vec2*) ((char*) data->buffers->data + data->buffer_views[1].offset);
    glm::vec3* normal_pointer = 
        (glm::vec3*) ((char*) data->buffers->data + data->buffer_views[2].offset);
    
    for (size_t i = 0; i < data->buffer_views[0].size / sizeof(glm::vec3); i++)
    {
        vertices[i].position = position_pointer[i];
        vertices[i].uv = uv_pointer[i];
        vertices[i].normal = normal_pointer[i];
    }

    for (size_t i = 0; i < indices_size; i++)
    {
        if (sizeof_index == sizeof(uint16_t)) indices[i] = 
            ((uint16_t*) ((char*) data->buffers->data + 
                data->buffer_views[3].offset))[i];
        else indices[i] = 
            ((uint32_t*) ((char*) data->buffers->data + 
                data->buffer_views[3].offset))[i];
    }

    animation_frames[identifier] = 
        {indices.size() / sizeof(uint32_t) - indices_size / sizeof(uint32_t) - 1, 
            indices_size / sizeof(uint32_t)};

    cgltf_free(data);
}

template <>
void Mesh<PosOnly>::load_animation(const std::string& identifier, const std::string& path)
{
    cgltf_data* data;
    cgltf_options options = cgltf_options();
    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
    options = cgltf_options();
    cgltf_load_buffers(&options, data, path.c_str());

    // LOl
    int do_more_error_checking = 0;

    size_t size = data->buffer_views[0].size / sizeof(glm::vec3);

    uint32_t sizeof_index = 
        data->accessors[1].component_type == cgltf_component_type_r_16u ? 
        sizeof(uint16_t) : sizeof(uint32_t);

    size_t indices_size = data->buffer_views[1].size / sizeof_index;

    vertices.resize(size + vertices.size());
    indices.resize(indices_size + indices.size());

    glm::vec3* position_pointer = 
        (glm::vec3*) ((char*) data->buffers->data + data->buffer_views[0].offset);

    for (size_t i = 0; i < size; i++)
    {
        vertices[i].position = position_pointer[i];
    }

    for (size_t i = 0; i < indices_size; i++)
    {
        if (sizeof_index == sizeof(uint16_t)) indices[i] = 
            ((uint16_t*) ((char*) data->buffers->data + 
                data->buffer_views[1].offset))[i];
        else indices[i] = 
            ((uint32_t*) ((char*) data->buffers->data + 
                data->buffer_views[1].offset))[i];
    }

    animation_frames[identifier] = 
        {indices.size() / sizeof(uint32_t) - indices_size / sizeof(uint32_t) - 1, 
            indices_size / sizeof(uint32_t)};
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
}

void StandardModel::load_texture(TextureAtlas* atlas)
{
    if (!mesh.get_texture()) return;
    this->texture_id = atlas->load_texture(mesh.get_texture().value());
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

void StandardModel::upload(BatchManager* batchmanager)
{
    // Load to batch renderer
    auto [batch, index] = batchmanager->add(this);
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

void TextureInstance::load_texture(TextureAtlas* atlas)
{
    if (!mesh.get_texture()) return;
    this->texture_id = atlas->load_texture(this->mesh.get_texture().value());
}

InstancedModel::InstancedModel(TextureInstance* base)
{
    this->base = base;
}

InstancedModel::~InstancedModel()
{
    base->get_batch()->remove_instance(obj_index);
}

void InstancedModel::upload(BatchManager*)
{
    if (base->get_batch() == nullptr) return; 

    obj_index = base->get_batch()->add_instance(this, base->get_index()); 
}

void InstancedModel::set_modelmat(const glm::mat4& mat)
{
    modelmat = mat;
    if (base->get_batch()) base->get_batch()->edit_model_data(this, obj_index);
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
