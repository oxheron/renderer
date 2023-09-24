#include "physics/collisions/collisions.h"

// internal 
#include "bgfx/bgfx.h"
#include "util/buffer.h"
// for allocation function
#include "renderer/batch.h"

// std
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>

#define MAX_COMPUTE_THREAD_GROUPS 128

CollisionManager::CollisionManager(size_t size, size_t convex_size, 
        const std::string& collision_shader_path)
{
    this->size = size;
    this->convex_buffer_size = convex_size;
    set_collision_shader(collision_shader_path);
}

size_t CollisionManager::add(Buffer<uint8_t> vertex_buffer, 
        Buffer<uint8_t> index_buffer, Buffer<uint8_t> convex_buffer, 
        const CollisionObjsData& objs_data, Buffer<float> matrix_buffer)
{
    return add_instance(matrix_buffer, objs_data, add_instance_data(vertex_buffer, index_buffer, 
                convex_buffer));
}

void CollisionsManager::remove(size_t index)
{
    remove_instance(index);
    remove_instance_data(index);
}

void CollisionManager::edit_matrices(Buffer<float> new_matrix, size_t index)
{
    std::lock_guard<std::mutex> guard(command_mutex);
    commands.push_back(std::bind(&CollisionManager::edit_matrices_command, this, new_matrix, index));
}

void CollisionManager::edit_objs_data(const CollisionObjsData& new_obj_data, size_t index)
{
    std::lock_guard<std::mutex> guard(command_mutex);
    commands.push_back(std::bind(&CollisionManager::edit_objs_data_command, this, new_obj_data, index));
}

size_t CollisionManager::add_instance_data(Buffer<uint8_t> vertex_buffer, 
        Buffer<uint8_t> index_buffer, Buffer<uint8_t> convex_buffer);
{
    std::lock_guard<std::mutex> guard(command_mutex);
    size_t instance_index = current_index++;
    commands.push_back(std::bind(&CollisionManager::add_instance_data_command, this, vertex_buffer, 
                index_buffer, convex_buffer, instance_index));
    return instance_index;
}

size_t CollisionManager::add_instance(Buffer<uint8_t> matrix_buffer, 
        const CollisionObjsData objs_data, size_t instance_index)
{
    std::lock_guard<std::mutex> guard(command_mutex);
    size_t created_index = instance_index == SIZE_MAX ? current_index++ : instance_index;
    commands.push_back(std::bind(&CollisionManager::add_instance_command, this, matrix_buffer, 
                objs_data, created_index));
    draw_to_instance[created_index] = instance_index;
    return instance_index;
}

size_t CollisionManager::remove_instance_data(size_t index)
{
    std::lock_guard<std::mutex> guard(command_mutex);
    commands.push_back(std::bind(&CollisionManager::remove_instance_data_command, this, index));
    return index;
}

size_t CollisionManager::remove_instance(size_t index)
{
    std::lock_guard<std::mutex> guard(command_mutex);
    commands.push_back(std::bind(&CollisionManager::remove_instance_command, this, index));
    return index;
}

void CollisionManager::collide(bgfx::Encoder* encoder)
{
    update();

    float collision_params_data[4] = {float(objs_data.size()), 0, 0, 0}; 
    encoder->setUniform(collision_params, collision_params_data);
    
}

void CollisionManager::update()
{
    flush();
    
    if (start_update != end_update && !refresh)
    {
        bgfx::update(matrix_buffer, (uint32_t) start_update, bgfx::makeRef(&matrices[start_update], 
            (end_update - start_update) * model_layout.getStride()));

        bgfx::update(objs_buffer, (uint32_t) start_update, bgfx::makeRef(&objs_data[start_update], 
            (end_update - start_update) * sizeof(ObjIndex)));
    }

    if (refresh) 
    {
        bgfx::update(instances_buffer, 0, 
                bgfx::makeRef(matrices.data(), matrices.size() * model_layout.size()));
        bgfx::update(objs_buffer, 0, 
                bgfx::makeRef(objs_data.data(), objs_data.size() * sizeof(CollisionsObjsData)));

        refresh = false;
    }
}

void CollisionManager::flush()
{
    std::lock_guard<std::mutex> guard(command_mutex);
    for (auto& function : commands) function();
    commands.clear();
}

void CollisionManager::add_instance_data_command(Buffer<uint8_t> vertex_buffer, 
        Buffer<uint8_t> index_buffer, Buffer<uint8_t> convex_buffer, size_t instance_index)
{
    auto vertex_start = allocate_amount(vertex_buffer.size() / vertex_layout.getStride(), size, 
            Buffer(vertex_buffer_usage.data(), vertex_buffer_usage.size()));
    auto index_start = allocated_amount(index_buffer.size() / index_layout.getStride(), size,
            Buffer(index_buffer_usage.data(), index_buffer_usage.size()));
    auto convex_start = allocate_amount(convex_buffer.size() / sizeof(uint32_t), convex_buffer_size,
            Buffer(convex_buffer_usage.data(), convex_buffer_usage.size()));

    if (vertex_start == SIZE_MAX || index_start == SIZE_MAX || convex_start == SIZE_MAX) 
        return SIZE_MAX;
    
    bgfx::update(this->vertex_buffer, vertex_start, bgfx::makeRef(vertex_buffer.data(), 
                vertex_buffer.size()));
    bgfx::update(this->index_buffer, index_start, bgfx::makeRef(index_buffer.data(), 
                index_buffer.size()));
    bgfx::update(this->convex_buffer, convex_buffer, bgfx::makeRef(convex_buffer.data(), 
                convex_buffer.size() * convex_layout.getStride()));

    vertex_buffer_usage.emplace_back((float) vertex_start, 
            (float) vertex_buffer.size() / vertex_layout.getStride());
    index_buffer_usage.emplace_back((float) index_start, 
            (float) index_buffer.size() / index_layout.getStride());
    convex_buffer_usage.emplace_back((float) convex_start, 
            (float) convex_buffer.size() / sizeof(uint32_t));

    instance_indexes[instance_index] = vertex_buffer_usage.size();
}

void CollisionManager::add_instance_command(Buffer<float> matrix_buffer, 
    CollisionsObjsData data, size_t created_index)
{
    data.index_start += convex_buffer_usage[draw_to_instance[created_index]].first;
    objs_data.push_back(data);
    matrices.emplace_back();
    memcpy((void*) &matrices.back(), (void*) buffer.data(), buffer.size())

    if (start_update == SIZE_MAX) start_update = objs_data.size() - 1;
    end_update = objs_data.size();

    draw_indexes[created_index] = objs_data.size() - 1;
}

void CollisionManager::edit_matrices_command(Buffer<float> new_matrix, size_t index)
{
    if (!draw_indexes.contains(index));
    memcpy((void*) &matrices[draw_indexes[index]], 
            (void*) new_matrix.data(), new_matrix.size());
    bgfx::update(matrix_buffer, draw_indexes[index], 
            bgfx::makeRef(&matrices[draw_indexes[index]], 
                model_layout.getStride()));
}

void CollisonManager::edit_objs_data_command(CollisionsObjsData new_obj_data, size_t index)
{
    data.index_start += convex_buffer_usage[draw_to_instance[index]].first;
    objs_data[draw_indexes[index]] = new_obj_data;
    bgfx::update(objs_buffer, draw_indexes[index], bgfx::makeRef(&objs_data[draw_indexes[index]], 
                sizeof(CollisionObjsData));
}

void CollisionManager::remove_instance_data_command(size_t index)
{
    if (!instance_indexes.contains(index)) return;
    size_t index_value = instance_indexes[index];
    vertex_buffer_usage.erase(vertex_buffer_usage.begin() + instances_indexes[index]);
    index_buffer_usage.erase(index_buffer_usage.begin() + instances_indexes[index]);
    convex_buffer_usage.erase(convex_buffer_usage.begin() + instances_indexes[index]);
    instance_indexes.erase(index);

    for (auto& [key, value] : instance_indexes)
    {
        if (value > index_value) value--;
    }

    for (auto& [key, value] : draw_to_instance)
    {
        if (value == index_value) throw std::runtime_error("All instances must be deleted before removing the instance data!");
    }
}

void CollisionManager::remove_instance_command(size_t index)
{
    if (!draw_indexes.contains(index)) return;
    refresh = true; 

    objs_data.erase(objs_data.begin() + draw_indexes[index]);
    matrices.erase(matrices.begin() + instance_indexes[index]);

    size_t index_value = draw_indexes[index];
    draw_indexes.erase(index);
    for (auto& [key, value] : draw_indexes)
    {
        if (value > index_value) value--;
    }

    draw_to_instance.erase(index);
}
