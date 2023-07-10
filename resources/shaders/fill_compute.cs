#include "bgfx_compute.sh"

BUFFER_RO(in_draws, vec4, 0);

BUFFER_WR(out_draws, uvec4, 1);

uniform vec4 draw_params; 

NUM_THREADS(64, 1, 1)

void main()
{
    int thread_id = int(gl_GlobalInvocationID.x);
    int per_thread_draw = int(draw_params.x) / 64 + 1;
    int idx_start = thread_id * per_thread_draw;
    int idx_end = min(int(draw_params.x), (thread_id + 1) * per_thread_draw);

    for (int i = idx_start; i < idx_end; i++)
    {
        drawIndexedIndirect(out_draws, i, in_draws[i].w, 1u, in_draws[i].z, in_draws[i].x, i);
    }
}
