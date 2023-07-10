#include "batch.h"

// internal
#include "model/mesh.h"
#include "util/util.h"
#include "global.h"
#include "core/shader.h"

// external 
#include <bimg/bimg.h>
#include <bimg/decode.h>
#include <bx/bx.h>

// std
#include <cstdio>
#include <utility>
#include <unordered_map>
#include <string>
#include <iostream>

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

Batch::Batch(size_t size, const std::string& compute_path, const bgfx::VertexLayout& vertex_layout, const bgfx::VertexLayout& model_layout)
{
    compute_program = BGFX_INVALID_HANDLE;
    this->size = size;
    set_compute_program(compute_path);
    this->vertex_layout = vertex_layout;
    this->model_layout = model_layout;
    vbh = bgfx::createDynamicVertexBuffer(size, vertex_layout, BGFX_BUFFER_ALLOW_RESIZE);
    ibh = bgfx::createDynamicIndexBuffer(size, BGFX_BUFFER_ALLOW_RESIZE);
    objs_buffer = bgfx::createDynamicVertexBuffer((uint32_t) 10, ObjIndex::layout(), BGFX_BUFFER_ALLOW_RESIZE | BGFX_BUFFER_COMPUTE_READ);
    instances_buffer = bgfx::createDynamicVertexBuffer((uint32_t) 10, model_layout, BGFX_BUFFER_ALLOW_RESIZE);
    draw_params = bgfx::createUniform("draw_params", bgfx::UniformType::Vec4);
    indirect_buffer = BGFX_INVALID_HANDLE;
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
    this->index_pointers = other.index_pointers;
    this->index_starts = other.index_starts;
    this->index_counts = other.index_counts;
    other.vbh = BGFX_INVALID_HANDLE;
    other.ibh = BGFX_INVALID_HANDLE;
    other.objs_buffer = BGFX_INVALID_HANDLE;
    other.instances_buffer = BGFX_INVALID_HANDLE;
    other.indirect_buffer = BGFX_INVALID_HANDLE;
    other.compute_program = BGFX_INVALID_HANDLE;
    other.draw_params = BGFX_INVALID_HANDLE;
    other.size = 0;
    other.index_pointers.clear();
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
    this->index_pointers = other.index_pointers;
    this->index_starts = other.index_starts;
    this->index_counts = other.index_counts;
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
    for (size_t* ptr : index_pointers)
    {
        delete ptr;
    }

    if (bgfx::isValid(vbh)) bgfx::destroy(vbh);
    if (bgfx::isValid(ibh)) bgfx::destroy(ibh);
    if (bgfx::isValid(objs_buffer)) bgfx::destroy(objs_buffer);
    if (bgfx::isValid(instances_buffer)) bgfx::destroy(instances_buffer);
    if (bgfx::isValid(indirect_buffer)) bgfx::destroy(indirect_buffer);
    if (bgfx::isValid(compute_program)) bgfx::destroy(compute_program);
    if (bgfx::isValid(draw_params)) bgfx::destroy(draw_params);
}

size_t* Batch::add(Mesh* mesh)
{
    auto [vertex_start, index_start] = get_start_in_buffers(mesh->get_vertex_buffer().size() / vertex_layout.getStride(), mesh->get_index_buffer().size() / sizeof(uint16_t));

    if (vertex_start == SIZE_MAX || index_start == SIZE_MAX) return nullptr;
 
    bgfx::update(vbh, vertex_start, bgfx::makeRef(mesh->get_vertex_buffer().data(), mesh->get_vertex_buffer().size())); 
    bgfx::update(ibh, index_start, bgfx::makeRef(mesh->get_index_buffer().data(), mesh->get_index_buffer().size())); 

    objs_data.push_back(ObjIndex{(float) vertex_start, (float) mesh->get_vertex_buffer().size() / vertex_layout.getStride(), 
        (float) index_start + mesh->animation_start(), (float) mesh->animation_length()});
    index_starts.push_back(index_start);
    index_counts.push_back(mesh->get_index_buffer().size() / sizeof(uint16_t));

    for (size_t i = 0; i < mesh->get_model_buffer().size(); i++)
    {
        model_data.push_back(mesh->get_model_buffer()[i]);
    }

    bgfx::update(objs_buffer, (uint32_t) (objs_data.size() - 1), bgfx::makeRef(&objs_data.back(), sizeof(ObjIndex)));
    bgfx::update(instances_buffer, (uint32_t) (objs_data.size() - 1), bgfx::makeRef(&model_data[(objs_data.size() - 1) * model_layout.getStride()], model_layout.getStride()));

    // No direct storage, update whole draw buffer & instance buffer every frame (will change method if this gets slow)
    update_compute = true;
    index_pointers.push_back(new size_t);
    *index_pointers.back() = objs_data.size() - 1;
    return index_pointers.back();
}

void Batch::edit_model_data(Mesh* mesh, size_t* index)
{
    for (size_t i = 0; i < model_layout.getStride(); i++)
    {
        model_data[*index * model_layout.getStride() + i] = mesh->get_model_buffer()[i];
    }
}

void Batch::edit_indirect(Mesh* mesh, size_t* index)
{
    objs_data[*index].index_start = index_starts[*index] + mesh->animation_start();
    objs_data[*index].index_count = mesh->animation_length();
    update_compute = true;
}

void Batch::edit(Mesh* mesh, size_t* index)
{
    // TODO
}

void Batch::remove(size_t* index)
{
    // TODO
}

void Batch::draw(bgfx::ProgramHandle rendering_program)
{
    update();
    if (!isValid(indirect_buffer)) return;

    // bgfx::update(instances_buffer, 0, bgfx::makeRef(model_data.data(), model_data.size()));
    bgfx::setVertexBuffer(0, vbh);
    bgfx::setIndexBuffer(ibh);
    bgfx::setInstanceDataBuffer(instances_buffer, 0, objs_data.size());
    bgfx::submit(0, rendering_program, indirect_buffer, 0, objs_data.size());
}

void Batch::set_compute_program(const std::string& compute_path)
{
    if (isValid(compute_program)) bgfx::destroy(compute_program);
    compute_program = bgfx::createProgram(load_shader(compute_path), true); 
}

void Batch::update()
{
    if (!isValid(compute_program)) return;

    if (update_compute)
    {
        if (isValid(indirect_buffer)) bgfx::destroy(indirect_buffer);
        indirect_buffer = bgfx::createIndirectBuffer(objs_data.size());
        bgfx::update(objs_buffer, 0, bgfx::makeRef(objs_data.data(), objs_data.size() * sizeof(ObjIndex)));
        float draw_data[4] = {float(objs_data.size()), 0, 0, 0};
        bgfx::setUniform(draw_params, draw_data);
        bgfx::setBuffer(0, objs_buffer, bgfx::Access::Read);
        bgfx::setBuffer(1, indirect_buffer, bgfx::Access::Write);
        bgfx::dispatch(0, compute_program, uint32_t(objs_data.size()/64 + 1), 1, 1);
        update_compute = false;
    }
}

std::pair<size_t, size_t> Batch::get_start_in_buffers(size_t num_vertices, size_t num_indices)
{
    std::vector<std::pair<size_t, size_t>> vertices_free_ranges;  
    std::vector<std::pair<size_t, size_t>> indices_free_ranges;  
    vertices_free_ranges.emplace_back(0, size);
    indices_free_ranges.emplace_back(0, size);

    for (size_t i = 0; i < objs_data.size(); i++)
    {
        for (size_t j = 0; j < vertices_free_ranges.size(); j++)
        {
            // Split or remove each range
            if (objs_data[i].vertex_start <= vertices_free_ranges[j].first 
                    && objs_data[i].vertex_start + objs_data[i].vertex_count >= vertices_free_ranges[j].first + vertices_free_ranges[j].second)
            {
                vertices_free_ranges.erase(vertices_free_ranges.begin() + j);              
                continue;
            }

            if (objs_data[i].vertex_start > vertices_free_ranges[j].first 
                    && objs_data[i].vertex_start + objs_data[i].vertex_count > vertices_free_ranges[j].first + vertices_free_ranges[j].second)
            {
                vertices_free_ranges[j].second = objs_data[i].vertex_start - vertices_free_ranges[j].first;
                continue;
            }

            if (objs_data[i].vertex_start <= vertices_free_ranges[j].first 
                    && objs_data[i].vertex_start + objs_data[i].vertex_count >= vertices_free_ranges[j].first) 
            {
                vertices_free_ranges[j].first = objs_data[i].vertex_start + objs_data[i].vertex_count;
                continue;
            }

            if (objs_data[i].vertex_start > vertices_free_ranges[j].first
                    && objs_data[i].vertex_start + objs_data[i].vertex_count < vertices_free_ranges[j].first + vertices_free_ranges[j].second)
            {
                vertices_free_ranges.emplace_back(objs_data[i].vertex_start + objs_data[i].vertex_count, vertices_free_ranges[j].second - objs_data[i].vertex_count);
                vertices_free_ranges[j].second = objs_data[i].vertex_start - vertices_free_ranges[j].first;
                continue;
            }
        }

        for (size_t j = 0; j < indices_free_ranges.size(); j++)
        {
            // Split or remove each range
            if (index_starts[i] <= indices_free_ranges[j].first 
                    && index_starts[i] + index_counts[i] >= indices_free_ranges[j].first + indices_free_ranges[j].second)
            {
                indices_free_ranges.erase(indices_free_ranges.begin() + j);              
                continue;
            }

            if  (index_starts[i] > indices_free_ranges[j].first 
                    && index_starts[i] + index_counts[i] > indices_free_ranges[j].first + indices_free_ranges[j].second)
            {
                indices_free_ranges[j].second = index_starts[i] - indices_free_ranges[j].first;
                continue;
            }

            if (index_starts[i] <= indices_free_ranges[j].first 
                    && index_starts[i] + index_counts[i] >= indices_free_ranges[j].first) 
            {
                indices_free_ranges[j].first = index_starts[i] + index_counts[i];
                continue;
            }

            if (index_starts[i] > vertices_free_ranges[j].first
                    && index_starts[i] + index_counts[i] < indices_free_ranges[j].first + indices_free_ranges[j].second)
            {
                indices_free_ranges.emplace_back(index_starts[i] + index_counts[i], indices_free_ranges[j].second - index_counts[i]);
                indices_free_ranges[j].second = index_starts[i] - indices_free_ranges[j].first;
                continue;
            }
        }
    }

    std::pair<size_t, size_t> rval = {SIZE_MAX, SIZE_MAX};

    for (auto range : vertices_free_ranges)
    {
        if (range.second >= num_vertices && range.first < rval.first) rval.first = range.first;
    }

    for (auto range : indices_free_ranges)
    {
        if (range.second >= num_indices && range.first < rval.second) rval.second = range.first;
    }

    return rval;
}

BatchManager::BatchManager()
{

}

BatchManager::BatchManager(uint16_t width, uint16_t height, bgfx::VertexLayout layout, bgfx::VertexLayout model_layout, const std::string& compute_path, uint16_t num_images, size_t batch_size)
{
    this->width = width;
    this->height = height;
    this->vertex_layout = layout;
    this->model_layout = model_layout;
    this->num_images = num_images;
    this->batch_size = batch_size;
    this->compute_path = compute_path;
    
    this->texture_handle = bgfx::createTexture2D(width, height, 0, num_images, bgfx::TextureFormat::Enum::RGB8);
    this->texture_sampler = bgfx::createUniform("textures", bgfx::UniformType::Sampler, num_images);
    this->batches.resize(0); 
}

BatchManager::~BatchManager()
{
    if (bgfx::isValid(texture_handle)) bgfx::destroy(texture_handle);
    if (bgfx::isValid(texture_sampler)) bgfx::destroy(texture_sampler);
    if (bgfx::isValid(rendering_program)) bgfx::destroy(rendering_program);
}

static void img_free(void*, void* img_container)
{
    bimg::imageFree((bimg::ImageContainer*) img_container);
}

void BatchManager::set_program(const std::string& vertex_path, const std::string& fragment_path)
{
    if (bgfx::isValid(rendering_program)) bgfx::destroy(rendering_program);
    bgfx::ShaderHandle vsh = load_shader(vertex_path);
    bgfx::ShaderHandle fsh = load_shader(fragment_path);
    rendering_program = bgfx::createProgram(vsh, fsh, true);
}

uint32_t BatchManager::load_texture(const std::string& path)
{
    if (mapped_paths.contains(path)) return mapped_paths[path];
    mapped_paths[path] = num_images_used;
    
    auto* raw_data = read_file_raw(path);  
    auto image_container = bimg::imageParse(global->allocator, raw_data->data, raw_data->size);
    if (image_container->m_width != width || image_container->m_height != height) throw std::runtime_error("Bad image being added to batch renderer");
    bgfx::updateTexture2D(this->texture_handle, num_images_used, 0, 0, 0, image_container->m_width, image_container->m_height, 
            bgfx::makeRef(image_container->m_data, image_container->m_size, img_free, image_container));
    bx::free(global->allocator, raw_data);
    return num_images_used++;
}

std::pair<Batch*, size_t*> BatchManager::add(Mesh* mesh)
{
    for (auto& batch : batches)
    {
        size_t* rval = batch.add(mesh);
        if (rval == nullptr) continue;
        return {&batch, rval};
    }

    batches.emplace_back(batch_size, compute_path, vertex_layout, model_layout);
    return {&batches.back(), batches.back().add(mesh)};
}

void BatchManager::draw()
{
    // Bind textures and set render state
    bgfx::setTexture(0, this->texture_sampler, this->texture_handle);
    bgfx::setState(BGFX_STATE_DEFAULT);
    for (auto& batch : batches)
    {
        batch.draw(this->rendering_program);
    }
}
