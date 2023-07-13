#pragma once

// external
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>

// std
#include <string>

class Texture
{
private:
    bgfx::TextureHandle texture_handle;
    bimg::ImageContainer* image_container;
    bgfx::Memory* raw_data;
    bgfx::UniformHandle sampler_uniform;

    bool valid_handle = false;
    bool valid_sampler = false;
public:
    std::string uniform_name = "s_texColor";
public:
    Texture();
    Texture(const std::string& filepath);

    void load_image(const std::string& filepath);
    void create_handle(uint64_t flags = BGFX_TEXTURE_NONE|BGFX_SAMPLER_NONE);
    void create_sampler();
    void destroy();
    bgfx::TextureHandle& get_handle() { return texture_handle; }
    bgfx::UniformHandle& get_sampler() { return sampler_uniform; }
};
