#include "batchmanager.h"

// internal
#include "core/shader.h"
#include "util/util.h"
#include "global.h"

// external
#include <bimg/bimg.h>
#include <bimg/decode.h>

BatchManager::BatchManager(bgfx::VertexLayout layout, 
    bgfx::VertexLayout model_layout, const std::string& compute_path, 
    size_t size)
{
    this->layout = layout;
    this->model_layout = model_layout;
    this->batch_size = size;
    this->batches.resize(0);
}

BatchManager::~BatchManager()
{
     
}

std::pair<Batch*, size_t> BatchManager::add(Model* model)
{
    for (auto& batch : batches)
    {
        size_t rval = batch.add(model);
        if (rval == SIZE_MAX) continue;
        return {&batch, rval};
    }

    batches.emplace_back(batch_size, compute_path, layout, model_layout);
    return {&batches.back(), batches.back().add(model)};
}

std::pair<Batch*, size_t> BatchManager::add_instance_data(
    Buffer<uint8_t> vertex_buffer, Buffer<uint8_t> index_buffer)
{
    for (auto& batch : batches)
    {
        size_t rval = batch.add_instance_data(vertex_buffer, index_buffer);
        if (rval == SIZE_MAX) continue;
        return {&batch, rval};
    }

    batches.emplace_back(batch_size, compute_path, layout, model_layout);
    return {&batches.back(), 
        batches.back().add_instance_data(vertex_buffer, index_buffer)};
}

void BatchManager::draw(bgfx::Encoder* encoder, bgfx::ProgramHandle program)
{
    // Bind textures and set render state
    // Multithreaded drawing
    // Potentially evolve onto more complex structure?
    if (!encoder) encoder = bgfx::begin();

    for (auto& batch : batches)
    {
        batch.draw(program, encoder);
    }

    bgfx::end(encoder);
}
