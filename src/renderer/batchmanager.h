#pragma once

// internal
#include "util/buffer.h"
#include "batch.h"

// external
#include <bgfx/bgfx.h>

// std
#include <string>
#include <vector>

class BatchManager
{   
private:
    // List of batches 
    std::vector<Batch> batches;

    // Size of each batch (number of vertices / indices allocated)
    size_t batch_size;
    
    // Computer shader path (file path)
    std::string compute_path;

    // Vertex layouts
    bgfx::VertexLayout layout;
    bgfx::VertexLayout model_layout;

public:
    // Constructor
    BatchManager(bgfx::VertexLayout layout, bgfx::VertexLayout model_layout, 
        const std::string& compute_path, size_t size = 100000);
    
    ~BatchManager();

    // Delete copy constructors
    BatchManager(const BatchManager& other) = delete;
    BatchManager& operator=(const BatchManager& other) = delete;

    // Add a model to a batch 
    std::pair<Batch*, size_t> add(Model* model);

    // Add data for a new instance to a batch (so you can make instances out of it)
    std::pair<Batch*, size_t> add_instance_data(Buffer<uint8_t> vertex_buffer, 
        Buffer<uint8_t> index_buffer);

    // Draw all of the batches, with other info added to the encoder
    void draw(bgfx::ViewId view, bgfx::ProgramHandle rendering_program, 
        bgfx::Encoder* encoder);
};
