#pragma once

// exernal 
#include "bgfx/bgfx.h"

// std
#include <array> 
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>
#include <mutex>

struct CollisionsObjsData
{
    // Indexes into convex buffer which then indexes into index buffer which indexes into vertex buffer 
    size_t index_start; // Have to have the index start from the instance data added
    size_t index_length;

    // Rectangle height for fast throwout of collisions
    size_t rec_height;
    size_t rec_width;
};

class CollisionManager
{
private:
    // Vertex buffers for the data that must be sent to the gpu 
    // Data buffer is the vertex data of the collision meshes 
    bgfx::DynamicVertexBufferHandle data_buffer;    

    // Index buffer is the indices into the data buffer that describe an object
    bgfx::DynamicVertexBufferHandle index_buffer;
    
    // Matrix buffer is the model matrix of each object
    bgfx::DyanmicVertexBufferHandle matrix_buffer;

    // Stores all other "per object" data
    // This includes the rectangle specification, the current animation, the indices and their length for this object, etc.
    bgfx::DynamicVertexBufferHandle objs_buffer;

    // Start and end indexes for convex shapes
    // Ex. the objs data stores a start and end location here, which then stores a list of start and end locations in the index buffer
    // The size of that list is also specified in the objs buffer
    bgfx::DynamicVertexBufferHandle convex_buffer;

    // Data of how the large buffers among these are used in gpu memory
    std::vector<std::pair<size_t, size_t>> vertex_buffer_usage;
    std::vector<std::pair<size_t, size_t>> index_buffer_usage;
    std::vector<std::pair<size_t, size_t>> convex_buffer_usage;

    // All of the layouts for the above vertex buffers
    bgfx::VertexLayout data_layout;
    bgfx::VertexLayout index_layout;
    bgfx::VertexLayout matrix_layout;
    bgfx::VertexLayout objs_layout;
    bgfx::VertexLayout convex_layout;

    // The raw data for the buffers
    // Note that the data is not stored here as it will not be editable or needed for any processes on the cpu
    std::vector<std::array<float, 16>> matrices;

    // The data for each object
    std::vector<CollisionObjsData> objs_data;

    // Size of the batch as the number of vertices in the batch 
    size_t size;
    size_t convex_buffer_size;

    // Uniform to input parameters into the compute shader
    bgfx::UniformHandle collision_params;

    // The compute shader that runs the collision detection
    bgfx::ProgramHandle collision_program;

    // The output values of the collisions after the compute shader has been called
    // These are stored in a texture because it is the only gpu writable and cpu readable data
    // Even though this is hacky
    // But... whatever
    bgfx::TextureHandle collision_output;

    // A uniform to send the draw parameters to the compute shader
    bgfx::UniformHandle draw_params;

    // Instance indexes contain the indexes into the large data storage buffers, mostly for allocation purposes
    // Draw indexes are used for everything else
    // A map that takes a draw index and turns it into an instance index
    std::unordered_map<size_t, size_t> instance_indexes;
    std::unordered_map<size_t, size_t> draw_indexes;
    std::unordered_map<size_t, size_t> draw_to_instance;
    size_t current_index = 0;

    // Some parameters to tell the update function how the buffers should be updated
    size_t start_update;
    size_t end_update;
    bool refresh;

    std::mutex command_mutex;
    std::vector<std::function<void()>> commands;
public:
    CollisionManager();
    explicit CollisionManager(size_t size, size_t convex_size, 
            const std::string& collision_shader_path);
    CollisionManager(const CollisionManager& other) = delete;
    CollisionManager& operator=(const CollisionManager& other) = delete;
    CollisionManager(CollisionManager&& other) noexcept;
    CollisionManager& operator=(CollisionManager&& other) noexcept;
    ~CollisionManager();

    // Manipulate the data in a batch
    size_t add(Buffer<uint8_t> vertex_buffer, Buffer<uint8_t> index_buffer, 
            Buffer<uint8_t> convex_buffer, const CollisionObjsData& obj_data, Buffer<float> matrix);
    void edit_matrices(Buffer<float> new_matrix, size_t index);
    void edit_objs_data(const CollisionObjsData& new_obj_data, size_t index);
    void remove(size_t index);

    // Instance adding
    size_t add_instance_data(Buffer<uint8_t> vertex_buffer, Buffer<uint8_t> index_buffer, 
            Buffer<uint8_t> convex_buffer); 
    size_t add_instance(Buffer<float> matrix_buffer, CollisionObjsData data, 
            size_t instance_index = SIZE_MAX);
    void remove_instance_data(size_t index);
    void remove_instance(size_t index);

    // Flush the commands and do other resource writing 
    void update();

    // Do the collisions
    void collide();

    // Change or add the collision shader
    void update_collision_shader(const std::string& shader_path);
private:
    // Run all of the commands and clear the command buffer
    void flush();

    // All of the commands that get run
    void edit_matrices_command(Buffer<float> new_matrix, size_t index);
    void edit_objs_data_command(const CollisionObjsData& new_obj_data, size_t index);
    void add_instance_data_command(Buffer<uint8_t> vertex_buffer, Buffer<uint8_t> index_buffer, 
            Buffer<uint8_t> convex_buffer, size_t instance_index); 
    void add_instance_command(Buffer<float> matrix_buffer, const CollisionObjsData& data, 
            size_t created_index);
    void remove_instance_data_command(size_t index);
    void remove_instance_command(size_t index);
};
