#ifndef EK_GFX_H
#define EK_GFX_H

#if defined(__ANDROID__)

//#define SOKOL_GLES3

//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>

#define SOKOL_GLES2

//#include <EGL/egl.h>

#ifndef GL_EXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#undef GL_ANGLE_instanced_arrays
#undef GL_EXT_draw_instanced

#elif defined(__EMSCRIPTEN__)

//#define SOKOL_GLES3

//#include <GLES3/gl3.h>

#define SOKOL_GLES2

#ifndef GL_EXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#elif defined (__APPLE__)

#define SOKOL_METAL

#elif defined(__linux__)

#define SOKOL_GLCORE33

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#elif defined(_WIN32) || defined(_WIN64)

#define SOKOL_D3D11

#endif

#include <sokol_gfx.h>

#include <stdint.h>
#include <ek/rr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * initialize sokol gfx subsystem and glue it to ek_app module
 * @param max_draw_calls - if <= 0 - default 128 will be applied
 */
void ek_gfx_setup(int max_draw_calls);

/**
 * shutdown sokol gfx subsystem
 */
void ek_gfx_shutdown(void);

bool ek_gfx_read_pixels(sg_image image, void* pixels);

sg_image ek_gfx_make_color_image(int width, int height, uint32_t color);

sg_image ek_gfx_make_render_target(int width, int height, const char* label);

void ek_gfx_update_image_0(sg_image image, void* data, size_t size);

typedef struct ek_shader {
    sg_shader shader;
    uint32_t images_num;
} ek_shader;

ek_shader ek_shader_make(const sg_shader_desc* desc);

struct res_shader {
    ek_shader data[16];
    string_hash_t names[16];
    rr_man_t rr;
};

struct res_image {
    string_hash_t names[16];
    sg_image data[16];
    rr_man_t rr;
};

extern struct res_shader res_shader;
extern struct res_image res_image;

#define R_IMAGE(name) REF_NAME(res_image, name)
#define R_SHADER(name) REF_NAME(res_shader, name)

void ek_gfx_res_setup();

#ifdef __cplusplus
}
#endif

#endif // EK_GFX_H
