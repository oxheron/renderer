#include "framebuffer.h"

// A shader that can be used is
/* $output v_texcoord

void main()
{
    vec2 vertices[3] = vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));
    gl_Position = vec4(vertices[gl_VertexID], 0, 1);
    v_texcoord = 0.5 * gl_Position.xy + vec2(0.5);
} */

// Then you can use the texture bound here

bgfx::Encoder* render_framebuffer(bgfx::FrameBufferHandle framebuffer, 
    bgfx::UniformHandle handle, uint8_t stage, uint8_t attachment, 
    bgfx::Encoder* encoder)
{
    if (!encoder) encoder = bgfx::begin();

    encoder->setTexture(stage, handle, getTexture(framebuffer, attachment));
    encoder->setVertexCount(3);

    return encoder; 
}
