#pragma once

// internal 
#include "util/buffer.h"

// external
#include <bgfx/bgfx.h>

// std
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <cstddef>

// Forward declaration of Model
class Model;

// Struct & layout for storing indirect draw call data on the cpu
struct ObjIndex 
{
    float vertex_start;
	float vertex_count;
	float index_start;
	float index_count;  

    ObjIndex(float vs, float vc, float is, float ic) 
        : vertex_start(vs), vertex_count(vc), index_start(is), index_count(ic) 
    {}

    static bgfx::VertexLayout layout()
    {
        static bgfx::VertexLayout layout;
        if (layout.getStride() != 0) return layout;

        layout.begin()
            .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
            .end();

        return layout;
    }
};

class Batch 
{
private:
    // Layout of the vertex buffer
    bgfx::VertexLayout vertex_layout;

    // The vertex buffer that gets added to
    bgfx::DynamicVertexBufferHandle vbh;

    // The index buffer that gets added to
    bgfx::DynamicIndexBufferHandle ibh;

    // Information about each model in the batch
    // Contains vertex & index offsets and counts
    std::vector<ObjIndex> objs_data;

    // Allocation data for the vertex and index buffers
    // This is the way the data is layed out on the gpu
    std::vector<std::pair<size_t, size_t>> vertex_buffer_usage;
    std::vector<std::pair<size_t, size_t>> index_buffer_usage;

    // The model instance data, such as matrices and textures
    // Only used for removal of models, since adding models doesn't require a whole buffer rewrite
    std::vector<uint8_t> model_data;
    
    // Objs buffer
    bgfx::DynamicVertexBufferHandle objs_buffer;

    // Layout of the model data
    bgfx::VertexLayout model_layout;

    // Instances buffer (cpu populated)
    bgfx::DynamicVertexBufferHandle instances_buffer;
 
    // Indirect buffer
    bgfx::IndirectBufferHandle indirect_buffer;

    // The compute shader that loads the indirect buffer
    bgfx::ProgramHandle compute_program;

    // Update compute
    bool update_compute = false;

    // Size of batch (in number of vertices)
    size_t size;

    // Instance indexes contain the indexes into the buffers (such as start vertex etc.)
    // Draw indexes contain the indexes into pretty much everything else
    // Also the current last index into the map
    std::unordered_map<size_t, size_t> instance_indexes;
    std::unordered_map<size_t, size_t> draw_indexes;
    std::unordered_map<size_t, size_t> draw_to_instance;
    size_t current_index = 0;

    // A uniform to send the draw parameters to the compute shader
    bgfx::UniformHandle draw_params;

    // Some parameters to tell the update function how the buffers should be updated
    size_t start_update;
    size_t end_update;
    bool refresh;
public:
    Batch();
    explicit Batch(size_t size, const std::string& compute_path, 
        const bgfx::VertexLayout& vertex_layout, 
        const bgfx::VertexLayout& model_layout);
    Batch(const Batch& other) = delete;
    Batch& operator=(const Batch& other) = delete;
    Batch(Batch&& other) noexcept;
    Batch& operator=(Batch&& other) noexcept;
    ~Batch();

    // Manipulate the data in a batch
    size_t add(Model* model);
    void edit(Model* model, size_t index);
    void edit_model_data(Model* model, size_t index);
    void edit_indirect(Model*, size_t model_index);
    void remove(size_t index);

    // Instance adding
    size_t add_instance_data(Buffer<uint8_t> vertex_buffer, 
        Buffer<uint8_t> index_buffer);
    size_t add_instance(Model* model, size_t instance_index, 
        bool new_index = true);
    void remove_instance_data(size_t index);
    void remove_instance(size_t index);

    // Texture list should have been bound before
    void draw(bgfx::ViewId view, bgfx::ProgramHandle rendering_program, 
        bgfx::Encoder* encoder);

    // Change/add a compute progam 
    void set_compute_program(const std::string& compute_path);
private:
    // Do all updates to the objs data and model data
    // Update the batch renderer
    // Run the compute shader (if needed)
    void update(bgfx::Encoder* encoder);

    // Get the start of the vertex and index buffers for a new model being added
    std::pair<size_t, size_t> get_start_in_buffers(size_t num_vertices, 
        size_t num_indices);
};

size_t allocate_amount(size_t amount, size_t memory_size, 
        Buffer<std::pair<size_t, size_t>> allocated_ranges);
