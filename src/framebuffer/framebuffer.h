#pragma once

#include <bgfx/bgfx.h>

bgfx::Encoder* render_framebuffer(bgfx::FrameBufferHandle framebuffer, 
    bgfx::UniformHandle handle, uint8_t stage = 0, uint8_t attachment = 0, 
    bgfx::Encoder* encoder = nullptr);
