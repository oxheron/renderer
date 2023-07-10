$input v_texcoord, v_normal, v_texture

#include "common.sh"

SAMPLER2DARRAY(textures, 0);

void main()
{
    gl_FragColor = texture2DArray(textures, vec3(v_texcoord, v_texture.x)); 
}

