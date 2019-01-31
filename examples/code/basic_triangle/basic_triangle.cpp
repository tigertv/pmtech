#include "console.h"
#include "file_system.h"
#include "memory.h"
#include "pen.h"
#include "pen_string.h"
#include "renderer.h"
#include "threads.h"
#include "timer.h"
#include "os.h"
#include "str\Str.h"
#include "loader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

pen::window_creation_params pen_window{
    1280,            // width
    720,             // height
    4,               // MSAA samples
    "basic_triangle" // window title / process name
};

struct vertex
{
    float x, y, z, w;
};

void read_complete(void* data, u32 row_pitch, u32 depth_pitch, u32 block_size)
{
#ifdef _WIN32
    Str reference_filename = "data/textures/";
    reference_filename.appendf("%s%s", pen_window.window_title, ".dds");

    void* file_data = nullptr;
    u32   file_data_size = 0;
    u32 pen_err = pen::filesystem_read_file_to_buffer(reference_filename.c_str(), &file_data, file_data_size);

    u32 diffs = 0;

    if (pen_err == PEN_ERR_OK)
    {
        // file exists do image compare
        u8* ref_image = (u8*)file_data + 124; // size of DDS header and we know its RGBA8
        u8* cmp_image = (u8*)data;

        for (u32 i = 0; i < depth_pitch; i+=4)
        {
            // ref is bgra
            if (ref_image[i+0] != cmp_image[i+2]) ++diffs;
            if (ref_image[i+1] != cmp_image[i+1]) ++diffs;
            if (ref_image[i+2] != cmp_image[i+0]) ++diffs;
            if (ref_image[i+3] != cmp_image[i+3]) ++diffs;
        }

        Str output_file = pen_window.window_title;
        output_file.append(".png");
        stbi_write_png(output_file.c_str(), pen_window.width, pen_window.height, 4, ref_image, row_pitch);

        free(file_data);
    }
    else
    {
        // save reference image
        Str output_file = pen_window.window_title;
        output_file.append(".png");
        stbi_write_png(output_file.c_str(), pen_window.width, pen_window.height, 4, data, row_pitch);
    }

    pen::os_terminate();
    PEN_CONSOLE("test complete %i diffs (%2.3f%%)\n", diffs, (f32)diffs / (f32)depth_pitch );
#endif
}

void test()
{
    // wait for the first swap.
    static u32 count = 0;
    if (count++ < 1)
        return;

    // run once, wait for result
    static bool ran = false;
    if (ran)
        return;

    PEN_CONSOLE("running test %s\n", pen_window.window_title);

    pen::resource_read_back_params rrbp;
    rrbp.block_size = 4;
    rrbp.format = PEN_TEX_FORMAT_RGBA8_UNORM;
    rrbp.resource_index = PEN_BACK_BUFFER_COLOUR;
    rrbp.depth_pitch = 1280 * 4;
    rrbp.data_size = 1280 * 720 * 4;
    rrbp.call_back_function = &read_complete;

    pen::renderer_read_back_resource(rrbp);

    ran = true;
}

PEN_TRV pen::user_entry(void* params)
{
    // unpack the params passed to the thread and signal to the engine it ok to proceed
    pen::job_thread_params* job_params = (pen::job_thread_params*)params;
    pen::job*               p_thread_info = job_params->job_info;
    pen::semaphore_post(p_thread_info->p_sem_continue, 1);

    // create clear state
    static pen::clear_state cs = {
        0.0f, 0.0, 0.5f, 1.0f, 1.0f, 0x00, PEN_CLEAR_COLOUR_BUFFER | PEN_CLEAR_DEPTH_BUFFER,
    };

    u32 clear_state = pen::renderer_create_clear_state(cs);

    // create raster state
    pen::rasteriser_state_creation_params rcp;
    pen::memory_zero(&rcp, sizeof(rasteriser_state_creation_params));
    rcp.fill_mode = PEN_FILL_SOLID;
    rcp.cull_mode = PEN_CULL_NONE;
    rcp.depth_bias_clamp = 0.0f;
    rcp.sloped_scale_depth_bias = 0.0f;

    u32 raster_state = pen::renderer_create_rasterizer_state(rcp);

    // create shaders
    pen::shader_load_params vs_slp;
    vs_slp.type = PEN_SHADER_TYPE_VS;

    pen::shader_load_params ps_slp;
    ps_slp.type = PEN_SHADER_TYPE_PS;

    c8 shader_file_buf[256];

    pen::string_format(shader_file_buf, 256, "data/pmfx/%s/%s/%s", pen::renderer_get_shader_platform(), "basictri",
                       "default.vsc");
    pen_error err = pen::filesystem_read_file_to_buffer(shader_file_buf, &vs_slp.byte_code, vs_slp.byte_code_size);
    PEN_ASSERT(!err);

    pen::string_format(shader_file_buf, 256, "data/pmfx/%s/%s/%s", pen::renderer_get_shader_platform(), "basictri",
                       "default.psc");
    err = pen::filesystem_read_file_to_buffer(shader_file_buf, &ps_slp.byte_code, ps_slp.byte_code_size);
    PEN_ASSERT(!err);

    u32 vertex_shader = pen::renderer_load_shader(vs_slp);
    u32 pixel_shader = pen::renderer_load_shader(ps_slp);

    // create input layout
    pen::input_layout_creation_params ilp;
    ilp.vs_byte_code = vs_slp.byte_code;
    ilp.vs_byte_code_size = vs_slp.byte_code_size;

    ilp.num_elements = 1;

    ilp.input_layout = (pen::input_layout_desc*)pen::memory_alloc(sizeof(pen::input_layout_desc) * ilp.num_elements);

    c8 buf[16];
    pen::string_format(&buf[0], 16, "POSITION");

    ilp.input_layout[0].semantic_name = (c8*)&buf[0];
    ilp.input_layout[0].semantic_index = 0;
    ilp.input_layout[0].format = PEN_VERTEX_FORMAT_FLOAT4;
    ilp.input_layout[0].input_slot = 0;
    ilp.input_layout[0].aligned_byte_offset = 0;
    ilp.input_layout[0].input_slot_class = PEN_INPUT_PER_VERTEX;
    ilp.input_layout[0].instance_data_step_rate = 0;

    u32 input_layout = pen::renderer_create_input_layout(ilp);

    // free byte code loaded from file
    pen::memory_free(vs_slp.byte_code);
    pen::memory_free(ps_slp.byte_code);

    // create vertex buffer
    vertex vertices[] = {0.0f, 0.5f, 0.5f, 1.0f, 0.5f, -0.5f, 0.5f, 1.0f, -0.5f, -0.5f, 0.5f, 1.0f};

    pen::buffer_creation_params bcp;
    bcp.usage_flags = PEN_USAGE_DEFAULT;
    bcp.bind_flags = PEN_BIND_VERTEX_BUFFER;
    bcp.cpu_access_flags = 0;

    bcp.buffer_size = sizeof(vertex) * 3;
    bcp.data = (void*)&vertices[0];

    u32 vertex_buffer = pen::renderer_create_buffer(bcp);

    while (1)
    {
        // set render targets to backbuffer
        pen::renderer_set_targets(PEN_BACK_BUFFER_COLOUR, PEN_BACK_BUFFER_DEPTH);

        // clear screen
        pen::viewport vp = {0.0f, 0.0f, (f32)pen_window.width, (f32)pen_window.height, 0.0f, 1.0f};

        pen::renderer_set_viewport(vp);
        pen::renderer_set_rasterizer_state(raster_state);
        pen::renderer_set_scissor_rect(rect{vp.x, vp.y, vp.width, vp.height});
        pen::renderer_clear(clear_state);

        // bind vertex layout
        pen::renderer_set_input_layout(input_layout);

        // bind vertex buffer
        u32 stride = sizeof(vertex);
        pen::renderer_set_vertex_buffer(vertex_buffer, 0, stride, 0);

        // bind shaders
        pen::renderer_set_shader(vertex_shader, PEN_SHADER_TYPE_VS);
        pen::renderer_set_shader(pixel_shader, PEN_SHADER_TYPE_PS);

        // draw
        pen::renderer_draw(3, 0, PEN_PT_TRIANGLELIST);

        // present
        pen::renderer_present();

        pen::renderer_consume_cmd_buffer();

        // test();

        // msg from the engine we want to terminate
        if (pen::semaphore_try_wait(p_thread_info->p_sem_exit))
        {
            break;
        }
    }

    // clean up mem here
    pen::renderer_release_buffer(vertex_buffer);
    pen::renderer_release_shader(vertex_shader, PEN_SHADER_TYPE_VS);
    pen::renderer_release_shader(pixel_shader, PEN_SHADER_TYPE_PS);
    pen::renderer_consume_cmd_buffer();

    // signal to the engine the thread has finished
    pen::semaphore_post(p_thread_info->p_sem_terminated, 1);

    return PEN_THREAD_OK;
}
