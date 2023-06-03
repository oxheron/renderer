#include "mesh.h"

// std
#include <stdexcept>

StandardMesh::StandardMesh()
{
    // Potentially allow customizable layouts
    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();
}

StandardMesh::~StandardMesh()
{
    bgfx::destroy(vbh);
    bgfx::destroy(ibh);
    texture.destroy();
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
    texture.load_image(path);
    texture.create_handle();
    texture.create_sampler();
}

void StandardMesh::set_handles()
{
    vbh = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(Vertex) * vertices_size), layout);
    ibh = bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(uint16_t) * indices_size));
}

// For now, all this does is draw the buffer
// In the future, this will be used to handle model matrix and more
void StandardMesh::draw(bgfx::ProgramHandle& prg_handle)
{
    bgfx::setTransform(glm::value_ptr(modelmat));
    bgfx::setVertexBuffer(0, vbh);
    bgfx::setIndexBuffer(ibh);
    bgfx::setTexture(0, texture.get_sampler(), texture.get_handle());
    bgfx::setState(BGFX_STATE_DEFAULT);
    bgfx::submit(0, prg_handle);
}

// Set the model matrix
// This is used to transform the mesh
// For particle meshes this could be used to set the overall position but there may be multiple
void StandardMesh::set_modelmat(const glm::mat4& mat)
{
    modelmat = mat;
}