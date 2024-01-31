#pragma once

// external
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>

// std
#include <string>
#include <unordered_map>

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

class TextureAtlas
{
private:
    // Bgfx texture stuff, for an atlas
    bgfx::TextureHandle texture_handle;
    bgfx::UniformHandle texture_sampler;
    std::unordered_map<std::string, uint32_t> mapped_paths;

    // Image widths and heights
    uint16_t width;
    uint16_t height;

    // Number of images
    uint16_t num_images;
    uint16_t num_images_used = 0;

    // Texture slot
    uint16_t stage;

public:
    TextureAtlas();
    explicit TextureAtlas(uint16_t width, uint16_t height, 
        uint16_t num_images = 100, const std::string& uniform_name = "texture", 
        uint16_t stage = 0);

    ~TextureAtlas();

    // Load a texture from a path
    uint16_t load_texture(const std::string& path);

    // Bind these textures to an encoder
    bgfx::Encoder* bind(bgfx::Encoder* encoder = nullptr);
};
