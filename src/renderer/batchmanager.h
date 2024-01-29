#pragma once

// internal
#include "util/buffer.h"
#include "batch.h"

// external
#include <bgfx/bgfx.h>

// std
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <stdexcept>

class BatchManager 
{
private:
    std::vector<Batch> batches;

    // All of the texture stuff
    // Bgfx stuff handled in the batch manager for now, maybe seperate texture class at some point
    bgfx::TextureHandle texture_handle;
    bgfx::UniformHandle texture_sampler;
    std::unordered_map<std::string, uint32_t> mapped_paths;

    // Image widths and heights
    uint16_t width;
    uint16_t height;

    // Number of images
    uint16_t num_images;
    uint16_t num_images_used = 0;

    // The compute program used for the batches (a file path)
    std::string compute_path;

    // The program used to render this set of batches
    // Should be set before drawing
    bgfx::ProgramHandle rendering_program = BGFX_INVALID_HANDLE;

    // The layouts used in each batch
    bgfx::VertexLayout vertex_layout;
    bgfx::VertexLayout model_layout;

    // The size of each batch (number of vertices)
    size_t batch_size;
public: 
    BatchManager();

    // Image width and height
    // Maybe at some point layout will be specific to modeles, and the batch will be selected from that? 
    explicit BatchManager(uint16_t width, uint16_t height, 
        bgfx::VertexLayout layout, bgfx::VertexLayout model_layout, 
        const std::string& compute_path, uint16_t num_images = 100, 
        size_t size = 100000);

    ~BatchManager();
    
    // Delete copy constructors
    BatchManager(const BatchManager& other) = delete;
    BatchManager& operator=(const BatchManager& other) = delete;

    // Set the program for this batch manager
    void set_program(const std::string& vertex_path, 
        const std::string& fragement_path);

    // Load a texture from a path
    uint32_t load_texture(const std::string& path);

    // Add a model to a batch
    std::pair<Batch*, size_t> add(Model* model); 

    // Add data for a new instance to a batch
    std::pair<Batch*, size_t> add_instance_data(Buffer<uint8_t> vertex_buffer, 
        Buffer<uint8_t> index_buffer);
    
    // Draw all of the batches
    void draw();
};

