#include "texture.h"

// internal
#include "global.h"
#include "util/util.h"

// external
#include <bimg/decode.h>

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
    image_container = bimg::imageParse(global.allocator, raw_data->data, raw_data->size);
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
    bx::free(global.allocator, raw_data);
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