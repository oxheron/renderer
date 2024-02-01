#include "texture.h"

// internal
#include "global.h"
#include "util/util.h"

// external
#include <bimg/decode.h>
#include <bgfx/bgfx.h>

Texture::Texture()
{
    // empty
}

Texture::Texture(const std::string& filepath)
{
    load_image(filepath);
    create_handle();
}

void Texture::load_image(const std::string& filepath)
{
    raw_data = read_file_raw(filepath);
    image_container = bimg::imageParse(global->allocator, raw_data->data, raw_data->size);
}

static void img_free(void*, void* img_container)
{
    bimg::imageFree((bimg::ImageContainer*) img_container);
}

void Texture::create_handle(uint64_t flags)
{
    if(!bgfx::isTextureValid(0, false, image_container->m_numLayers, bgfx::TextureFormat::Enum(image_container->m_format), flags)) return;
    texture_handle = bgfx::createTexture2D(image_container->m_width, image_container->m_height, 1 < image_container->m_numMips, image_container->m_numLayers, bgfx::TextureFormat::Enum(image_container->m_format), flags, bgfx::makeRef(
					  image_container->m_data, image_container->m_size, img_free, image_container));
    bx::free(global->allocator, raw_data);
    valid_handle = true;
}

void Texture::create_sampler()
{
    if (!valid_handle) return;
    sampler_uniform = bgfx::createUniform(uniform_name.c_str(), bgfx::UniformType::Sampler);
    valid_sampler = true;
}

void Texture::destroy()
{
    if (valid_handle) bgfx::destroy(texture_handle);
    if (valid_sampler) bgfx::destroy(sampler_uniform);
    valid_handle = false;
    valid_sampler = false;
}

TextureAtlas::TextureAtlas() 
{
      
}

TextureAtlas::TextureAtlas(uint16_t width, uint16_t height, 
    uint16_t num_images, const std::string& uniform_name, uint16_t stage)
{
    this->width = width;
    this->height = height;
    this->num_images = num_images;
    this->stage = stage;
    
    this->texture_handle = 
        bgfx::createTexture2D(width, height, 0, num_images, 
        bgfx::TextureFormat::Enum::RGB8);
    this->texture_sampler = 
        bgfx::createUniform(
            uniform_name.c_str(), bgfx::UniformType::Sampler, num_images);
}

TextureAtlas::~TextureAtlas()
{
    if (bgfx::isValid(texture_handle)) bgfx::destroy(texture_handle);
    if (bgfx::isValid(texture_sampler)) bgfx::destroy(texture_sampler);
}

uint16_t TextureAtlas::load_texture(const std::string& path)
{
    if (mapped_paths.contains(path)) return mapped_paths[path];
    mapped_paths[path] = num_images_used;
    
    auto* raw_data = read_file_raw(path);  
    auto image_container = bimg::imageParse(global->allocator, 
        raw_data->data, raw_data->size);

    if (image_container->m_width != width || image_container->m_height != height) 
        throw std::runtime_error("Bad image being added to batch renderer");

    bgfx::updateTexture2D(this->texture_handle, num_images_used, 0, 0, 0, 
        image_container->m_width, image_container->m_height, 
        bgfx::makeRef(image_container->m_data, image_container->m_size, 
        img_free, image_container));
    bx::free(global->allocator, raw_data);

    return num_images_used++;
}

bgfx::Encoder* TextureAtlas::bind(bgfx::Encoder* encoder)
{
    if (!encoder) encoder = bgfx::begin();
    encoder->setTexture(stage, texture_sampler, texture_handle);
    return encoder;
}
