#include "mesh.h"

// std
#include <stdexcept>

StandardMesh::StandardMesh()
{
    // Potentially allow customizable layouts
}

StandardMesh::~StandardMesh()
{
    delete[] vertices;
    cgltf_free(data);
}

void StandardMesh::load_model(const std::string& path)
{
    // Load buffers
    cgltf_options options = cgltf_options();
    cgltf_result result = cgltf_parse_file(&options, "resources/models/cube.gltf", &data);
    options = cgltf_options();
    cgltf_load_buffers(&options, data, "resources/models/cube.gltf");

    // Parse buffers into mesh
    vertices_size = (data->buffer_views[0].size + data->buffer_views[1].size + data->buffer_views[2].size) / (sizeof(float) * 8);
    indices_size = data->buffer_views[3].size / sizeof(uint16_t);

    // Dangerous hack, assume order of buffer data, and that it has positions, normals, and uv data 
    vertices = new Vertex[vertices_size];
    
    // Data freed in destructor, indices are just a pointer to the data in the buffer
    indices = (uint16_t*) ((char*) data->buffers->data + data->buffer_views[3].offset);

    // Copy vertex data
    glm::vec3* position_pointer = (glm::vec3*) ((char*) data->buffers->data + data->buffer_views[0].offset);
    glm::vec2* uv_pointer = (glm::vec2*) ((char*) data->buffers->data + data->buffer_views[1].offset);
    glm::vec3* normal_pointer = (glm::vec3*) ((char*) data->buffers->data + data->buffer_views[2].offset);

    if (data->buffer_views[0].size / sizeof(glm::vec3) != data->buffer_views[1].size / sizeof(glm::vec2) || data->buffer_views[0].size / sizeof(glm::vec3) != data->buffer_views[2].size / sizeof(glm::vec3)) throw std::runtime_error("Bad buffer data in parsing gltf file");
    
    for (size_t i = 0; i < data->buffer_views[0].size / sizeof(glm::vec3); i++)
    {
        vertices[i].position = position_pointer[i];
        vertices[i].uv = uv_pointer[i];
        vertices[i].normal = normal_pointer[i];
    }
}

void StandardMesh::load_texture(const std::string& path)
{
    this->texture_id = br->load_texture(path);
    std::cout << this->texture_id << std::endl;
}

void StandardMesh::set_batchmanager(BatchManager* br)
{
    this->br = br;
}

// Set the model matrix
// This is used to transform the mesh
// For particle meshes this could be used to set the overall position but there may be multiple
// Also edit the value in the batch
void StandardMesh::set_modelmat(const glm::mat4& mat)
{
    modelmat = mat;
    if (batch) batch->edit_model_data(this, index);
}

void StandardMesh::upload()
{
    // Load to batch renderer
    auto [batch, index] = br->add(this);
    this->batch = batch;
    this->index = index;
}

Buffer<uint8_t> StandardMesh::get_model_buffer()
{
    // Make sure the stride is a multiple of 16 (20 in this case)
    if (model_buffer.size() == 0) model_buffer.resize(16 * sizeof(float) + sizeof(float) * 4);
    memcpy((void*) model_buffer.data(), (void*) glm::value_ptr(modelmat), 16 * sizeof(float));
    float* tex_id = (float*) (model_buffer.data() + 16 * sizeof(float));
    *tex_id = (float) texture_id;
    std::cout << *tex_id << std::endl;
    return Buffer(model_buffer.data(), model_buffer.size());
}

Buffer<uint8_t> StandardMesh::get_vertex_buffer()
{
    return Buffer((uint8_t*) vertices, vertices_size * sizeof(Vertex));
}

Buffer<uint8_t> StandardMesh::get_index_buffer()
{
    return Buffer((uint8_t*) indices, indices_size * sizeof(uint16_t));
}
