#include <bgfx/bgfx.h>

bgfx::VertexLayout pos_only_layout()
{
    static bgfx::VertexLayout layout;
    static bool initialized = false;
    if (!initialized)
    {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .end();
        initialized = true;
    }

    return layout;
}

bgfx::VertexLayout objs_info_layout(uint8_t count)
{
    static bgfx::Attrib::Enum texcoords[] = 
}

bgfx::VertexLayout pos_tex_norm()
{
    static bgfx::VertexLayout layout;
    static bool initialized = false;
    if (!initialized)
    {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .end();
        initialized = true;
    }

    return layout;
}

bgfx::VertexLayout objs_info_layout(uint8_t count)
{
    static bgfx::Attrib::Enum texcoords[] = 
    {
        bgfx::Attrib::TexCoord0,
        bgfx::Attrib::TexCoord1,
        bgfx::Attrib::TexCoord2,
        bgfx::Attrib::TexCoord3,
        bgfx::Attrib::TexCoord4,
        bgfx::Attrib::TexCoord5,
        bgfx::Attrib::TexCoord6,
    };

    bgfx::VertexLayout layout;
    auto current_layout = layout.begin();

    for (size_t i = 0; i < count; i++)
    {
        current_layout =
            current_layout.add(texcoords[i], 4, bgfx::AttribType::Float);
    }
    current_layout.end();

    return current_layout;
}
