#include "batch.h"

// internal
#include "model/mesh.h"
#include "util/util.h"
#include "global.h"
#include "core/shader.h"

// std
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <utility>
#include <unordered_map>
#include <string>
#include <iostream>
#include <algorithm>

Batch::Batch()
{
    vbh = BGFX_INVALID_HANDLE;
    ibh = BGFX_INVALID_HANDLE;
    objs_buffer = BGFX_INVALID_HANDLE;
    instances_buffer = BGFX_INVALID_HANDLE;
    indirect_buffer = BGFX_INVALID_HANDLE;
    compute_program = BGFX_INVALID_HANDLE;
    size = 0;
}

Batch::Batch(size_t size, const std::string& compute_path, 
    const bgfx::VertexLayout& vertex_layout, 
    const bgfx::VertexLayout& model_layout)
{
    compute_program = BGFX_INVALID_HANDLE;
    this->size = size;
    set_compute_program(compute_path);
    this->vertex_layout = vertex_layout;
    this->model_layout = model_layout;
    vbh = bgfx::createDynamicVertexBuffer(size, vertex_layout, 
        BGFX_BUFFER_ALLOW_RESIZE);
    ibh = bgfx::createDynamicIndexBuffer(size, 
        BGFX_BUFFER_ALLOW_RESIZE | BGFX_BUFFER_INDEX32);
    objs_buffer = bgfx::createDynamicVertexBuffer((uint32_t) 0, 
        ObjIndex::layout(), 
        BGFX_BUFFER_ALLOW_RESIZE | BGFX_BUFFER_COMPUTE_READ);
    instances_buffer = bgfx::createDynamicVertexBuffer((uint32_t) 0, 
        model_layout, BGFX_BUFFER_ALLOW_RESIZE);
    draw_params = bgfx::createUniform("draw_params", bgfx::UniformType::Vec4);
    indirect_buffer = BGFX_INVALID_HANDLE;
    start_update = end_update = SIZE_MAX;
    refresh = false;
}

Batch::Batch(Batch&& other) noexcept
{
    this->vertex_layout = other.vertex_layout;
    this->model_layout = other.model_layout;
    this->vbh = other.vbh;
    this->ibh = other.ibh;
    this->objs_buffer = other.objs_buffer;
    this->instances_buffer = other.instances_buffer;
    this->indirect_buffer = other.indirect_buffer;
    this->compute_program = other.compute_program;
    this->draw_params = other.draw_params;
    this->size = other.size;
    this->model_data = std::move(other.model_data);
    this->objs_data = std::move(other.objs_data);
    this->vertex_buffer_usage = std::move(other.vertex_buffer_usage);
    this->index_buffer_usage = std::move(other.index_buffer_usage);
    this->start_update = other.start_update;
    this->end_update = other.end_update;
    this->refresh = other.refresh;
    this->current_index = other.current_index;
    this->draw_indexes = std::move(other.draw_indexes);
    this->instance_indexes = std::move(other.instance_indexes);
    this->draw_to_instance = std::move(other.draw_to_instance);
    other.vbh = BGFX_INVALID_HANDLE;
    other.ibh = BGFX_INVALID_HANDLE;
    other.objs_buffer = BGFX_INVALID_HANDLE;
    other.instances_buffer = BGFX_INVALID_HANDLE;
    other.indirect_buffer = BGFX_INVALID_HANDLE;
    other.compute_program = BGFX_INVALID_HANDLE;
    other.draw_params = BGFX_INVALID_HANDLE;
    other.size = 0;
}

Batch& Batch::operator=(Batch&& other) noexcept
{
    this->vertex_layout = other.vertex_layout;
    this->model_layout = other.model_layout;
    this->vbh = other.vbh;
    this->ibh = other.ibh;
    this->objs_buffer = other.objs_buffer;
    this->instances_buffer = other.instances_buffer;
    this->indirect_buffer = other.indirect_buffer;
    this->compute_program = other.compute_program;
    this->draw_params = other.draw_params;
    this->size = other.size;
    this->model_data = std::move(other.model_data);
    this->objs_data = std::move(other.objs_data);
    this->vertex_buffer_usage = std::move(other.vertex_buffer_usage);
    this->index_buffer_usage = std::move(other.index_buffer_usage);
    this->start_update = other.start_update;
    this->end_update = other.end_update;
    this->refresh = other.refresh;
    this->current_index = other.current_index;
    this->draw_indexes = std::move(other.draw_indexes);
    this->instance_indexes = std::move(other.instance_indexes);
    this->draw_to_instance = std::move(other.draw_to_instance);
    other.vbh = BGFX_INVALID_HANDLE;
    other.ibh = BGFX_INVALID_HANDLE;
    other.objs_buffer = BGFX_INVALID_HANDLE;
    other.instances_buffer = BGFX_INVALID_HANDLE;
    other.indirect_buffer = BGFX_INVALID_HANDLE;
    other.compute_program = BGFX_INVALID_HANDLE;
    other.draw_params = BGFX_INVALID_HANDLE;
    other.size = 0;
    return *this;
}

Batch::~Batch()
{
    if (bgfx::isValid(vbh)) bgfx::destroy(vbh);
    if (bgfx::isValid(ibh)) bgfx::destroy(ibh);
    if (bgfx::isValid(objs_buffer)) bgfx::destroy(objs_buffer);
    if (bgfx::isValid(instances_buffer)) bgfx::destroy(instances_buffer);
    if (bgfx::isValid(indirect_buffer)) bgfx::destroy(indirect_buffer);
    if (bgfx::isValid(compute_program)) bgfx::destroy(compute_program);
    if (bgfx::isValid(draw_params)) bgfx::destroy(draw_params);
}

size_t Batch::add(Model* model)
{
    size_t instance_index = add_instance_data(
        model->get_vertex_buffer(), model->get_index_buffer());
    add_instance(model, instance_index, false);
    return instance_index;
}

void Batch::edit_model_data(Model* model, size_t index)
{
    if (!draw_indexes.contains(index)) return;
    Buffer<uint8_t> model_buffer = model->get_model_buffer();
    for (size_t i = 0; i < model_layout.getStride(); i++)
    {
        model_data[draw_indexes[index] * model_layout.getStride() + i] = model_buffer[i];
    }

    bgfx::update(instances_buffer, draw_indexes[index], 
        bgfx::makeRef(
        &model_data[draw_indexes[index] * model_layout.getStride()], 
        model_layout.getStride()));
}
 
void Batch::edit_indirect(Model* model, size_t index)
{
    if (!draw_indexes.contains(index) || !draw_to_instance.contains(index) 
        || !instance_indexes.contains(draw_to_instance[index])) return;
     
    objs_data[draw_indexes[index]].index_start = 
        index_buffer_usage[instance_indexes[draw_to_instance[index]]].first + 
        model->animation_start();
    objs_data[draw_indexes[index]].index_count = model->animation_length();

    update_compute = true;

    bgfx::update(objs_buffer, draw_indexes[index], 
            bgfx::makeRef(&objs_data[draw_indexes[index]], sizeof(ObjIndex)));
}

void Batch::edit(Model* model, size_t index)
{
    // TODO
}

void Batch::remove(size_t index)
{
    remove_instance(index);
    remove_instance_data(index);
}

size_t Batch::add_instance_data(Buffer<uint8_t> vertex_buffer, 
    Buffer<uint8_t> index_buffer)
{
    auto vertex_start = allocate_amount(
        vertex_buffer.size() / vertex_layout.getStride(), size, 
        Buffer<std::pair<size_t, size_t>>(vertex_buffer_usage.data(), 
        vertex_buffer_usage.size()));
    auto index_start = allocate_amount(index_buffer.size() / sizeof(uint32_t), 
        size, Buffer<std::pair<size_t, size_t>>(index_buffer_usage.data(), 
        index_buffer_usage.size()));
    if (vertex_start == SIZE_MAX || index_start == SIZE_MAX) return SIZE_MAX;

    bgfx::update(vbh, vertex_start, bgfx::makeRef(vertex_buffer.data(), 
        vertex_buffer.size())); 
    bgfx::update(ibh, index_start, bgfx::makeRef(index_buffer.data(), 
        index_buffer.size())); 

    vertex_buffer_usage.emplace_back(vertex_start, 
        vertex_buffer.size() / vertex_layout.getStride());

    index_buffer_usage.emplace_back(index_start, 
        index_buffer.size() / sizeof(uint32_t));

    size_t instance_index = current_index++;
    instance_indexes[instance_index] = vertex_buffer_usage.size() - 1;
    return instance_index;
}

size_t Batch::add_instance(Model* model, size_t instance_index, bool new_index)
{
    if (instance_index == SIZE_MAX) return SIZE_MAX;
    objs_data.emplace_back(
        (float) vertex_buffer_usage[instance_indexes[instance_index]].first, 
        (float) vertex_buffer_usage[instance_indexes[instance_index]].second, 
        (float) model->animation_start() + 
        index_buffer_usage[instance_indexes[instance_index]].first, 
        (float) model->animation_length());

    Buffer<uint8_t> model_buffer = model->get_model_buffer();
    for (size_t i = 0; i < model_buffer.size(); i++)
    {
        model_data.push_back(model_buffer[i]);
    }

    if (start_update == SIZE_MAX) start_update = objs_data.size() - 1;
    end_update = objs_data.size();
    
    size_t created_index = new_index ? current_index++ : instance_index;
    draw_indexes[created_index] = objs_data.size() - 1;
    draw_to_instance[created_index] = instance_index;
    update_compute = true;
    return created_index;
}

void Batch::remove_instance_data(size_t index)
{
    if (!instance_indexes.contains(index)) return;
    size_t index_value = instance_indexes[index];
    vertex_buffer_usage.erase(
        vertex_buffer_usage.begin() + instance_indexes[index]);
    index_buffer_usage.erase(
        index_buffer_usage.begin() + instance_indexes[index]);
    instance_indexes.erase(index);
    
    for (auto& [key, value] : instance_indexes)
    {
        if (value > index_value) value--;
    }

    for (auto& [key, value] : draw_to_instance)
    {
        if (value == index_value) 
            throw std::runtime_error(
            "All instances must be deleted before removing the instance data!");
    }
}

void Batch::remove_instance(size_t index)
{   
    if (!draw_indexes.contains(index)) return;
    update_compute = true;
    refresh = true; 

    objs_data.erase(objs_data.begin() + draw_indexes[index]);

    for (size_t i = 0; i < model_layout.getStride(); i++)
    {
        model_data.erase(
            model_data.begin() + draw_indexes[index] * model_layout.getStride());
    }

    size_t index_value = draw_indexes[index];
    draw_indexes.erase(index);
    for (auto& [key, value] : draw_indexes)
    {
        if (value > index_value) value--;
    }

    draw_to_instance.erase(index);
}

void Batch::draw(bgfx::ProgramHandle rendering_program, bgfx::Encoder* encoder)
{
    update(encoder);
    if (!isValid(indirect_buffer)) return;

    encoder->setVertexBuffer(0, vbh);
    encoder->setIndexBuffer(ibh);
    encoder->setInstanceDataBuffer(instances_buffer, 0, objs_data.size());
    encoder->submit(0, rendering_program, indirect_buffer, 0, objs_data.size());
}

void Batch::set_compute_program(const std::string& compute_path)
{
    if (isValid(compute_program)) bgfx::destroy(compute_program);
    compute_program = bgfx::createProgram(load_shader(compute_path), true); 
}

void Batch::update(bgfx::Encoder* encoder)
{
    if (!isValid(compute_program)) return;

    if (start_update != end_update && !refresh) 
    { 
        bgfx::update(instances_buffer, (uint32_t) start_update, bgfx::makeRef(&model_data[start_update * model_layout.getStride()], (end_update - start_update) * model_layout.getStride()));
        bgfx::update(objs_buffer, (uint32_t) start_update, bgfx::makeRef(&objs_data[start_update], (end_update - start_update) * sizeof(ObjIndex)));
    }

    if (refresh)
    {
        bgfx::update(instances_buffer, 0, bgfx::makeRef(model_data.data(), model_data.size()));
        bgfx::update(objs_buffer, 0, bgfx::makeRef(objs_data.data(), objs_data.size() * sizeof(ObjIndex)));
        refresh = false;
    }

    if (update_compute)
    {
        if (isValid(indirect_buffer)) bgfx::destroy(indirect_buffer);
        indirect_buffer = bgfx::createIndirectBuffer(objs_data.size());
        float draw_data[4] = {float(objs_data.size()), 0, 0, 0};
        encoder->setUniform(draw_params, draw_data);
        encoder->setBuffer(0, objs_buffer, bgfx::Access::Read);
        encoder->setBuffer(1, indirect_buffer, bgfx::Access::Write);
        encoder->dispatch(0, compute_program, uint32_t(objs_data.size()/64 + 1), 1, 1);
        update_compute = false;
    }
    
    start_update = end_update = SIZE_MAX;
}

size_t allocate_amount(size_t amount, size_t memory_size, 
    Buffer<std::pair<size_t, size_t>> allocated_ranges)
{
    std::vector<std::pair<size_t, size_t>> free_ranges;
    free_ranges.emplace_back(0, memory_size);

    for (size_t i = 0; i < allocated_ranges.size(); i++)
    {
        for (size_t j = 0; j < free_ranges.size(); j++)
        {
            if (allocated_ranges[i].first <= free_ranges[j].first
                && allocated_ranges[i].first + allocated_ranges[i].second >= 
                free_ranges[j].first + free_ranges[j].second)  
            {
                free_ranges.erase(free_ranges.begin() + j);              
                continue;
            }

            if (allocated_ranges[i].first > free_ranges[j].first
                && allocated_ranges[i].first + allocated_ranges[i].second > 
                free_ranges[j].first + free_ranges[j].second)
            {
                free_ranges[j].second = allocated_ranges[i].first - 
                    free_ranges[j].first;   
                continue;
            }

            if (allocated_ranges[i].first <= free_ranges[j].first
                && allocated_ranges[i].first + allocated_ranges[i].second >=
                free_ranges[j].first)
            {
                free_ranges[j].first = 
                    allocated_ranges[i].first + allocated_ranges[i].second;
                continue;
            }

            if (allocated_ranges[i].first > free_ranges[j].first
                && allocated_ranges[i].first + allocated_ranges[i].second < 
                free_ranges[j].first + free_ranges[j].second)
            {
                free_ranges.emplace_back(
                    allocated_ranges[i].first + allocated_ranges[i].second, 
                    free_ranges[j].second - allocated_ranges[i].second);
                free_ranges[j].second = 
                    allocated_ranges[i].first - free_ranges[j].first; 
                continue;
            }
        }
    }

    size_t rval = SIZE_MAX;

    for (auto& range : free_ranges)
    {
        if (range.second >= amount && range.first < rval) rval = range.first;
    }

    return rval;
}
