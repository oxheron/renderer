#include "batchmanager.h"

// internal
#include "core/shader.h"
#include "util/util.h"
#include "global.h"

// external
#include <bimg/bimg.h>
#include <bimg/decode.h>

BatchManager::BatchManager()
{

}

BatchManager::BatchManager(uint16_t width, uint16_t height, 
    bgfx::VertexLayout layout, bgfx::VertexLayout model_layout, 
    const std::string& compute_path, uint16_t num_images, size_t batch_size)
{
    this->width = width;
    this->height = height;
    this->vertex_layout = layout;
    this->model_layout = model_layout;
    this->num_images = num_images;
    this->batch_size = batch_size;
    this->compute_path = compute_path;
    
    this->texture_handle = 
        bgfx::createTexture2D(width, height, 0, num_images, 
        bgfx::TextureFormat::Enum::RGB8);
    this->texture_sampler = 
        bgfx::createUniform("textures", bgfx::UniformType::Sampler, num_images);
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

void BatchManager::set_program(const std::string& vertex_path, 
    const std::string& fragment_path)
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

std::pair<Batch*, size_t> BatchManager::add(Model* model)
{
    for (auto& batch : batches)
    {
        size_t rval = batch.add(model);
        if (rval == SIZE_MAX) continue;
        return {&batch, rval};
    }

    batches.emplace_back(batch_size, compute_path, vertex_layout, model_layout);
    return {&batches.back(), batches.back().add(model)};
}

std::pair<Batch*, size_t> BatchManager::add_instance_data(Buffer<uint8_t> vertex_buffer, Buffer<uint8_t> index_buffer)
{
    for (auto& batch : batches)
    {
        size_t rval = batch.add_instance_data(vertex_buffer, index_buffer);
        if (rval == SIZE_MAX) continue;
        return {&batch, rval};
    }

    batches.emplace_back(batch_size, compute_path, vertex_layout, model_layout);
    return {&batches.back(), batches.back().add_instance_data(vertex_buffer, index_buffer)};
}

void BatchManager::draw()
{
    // Bind textures and set render state
    // Multithreaded drawing
    // Potentially evolve onto more complex structure?
    bgfx::Encoder* encoder = bgfx::begin();
    encoder->setTexture(0, this->texture_sampler, this->texture_handle);
    encoder->setState(BGFX_STATE_DEFAULT);
    for (auto& batch : batches)
    {
        batch.draw(this->rendering_program, encoder);
    }
    bgfx::end(encoder);
}
