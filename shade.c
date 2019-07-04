#include "shade.h"

#include <stddef.h>
#include <stdlib.h>

#include "bcm.h"
#include "egl.h"
#include "exec.h"
#include "prog.h"

#define EXPORT __attribute__((visibility("default")))

EXPORT const int SHD_SHADER_VERTEX_VALUE = SHD_SHADER_VERTEX;
EXPORT const int SHD_SHADER_FRAGMENT_VALUE = SHD_SHADER_FRAGMENT;

EXPORT const int SHD_PREDEFINED_RESOLUTION_VALUE = SHD_PREDEFINED_RESOLUTION;
EXPORT const int SHD_PREDEFINED_PLAY_TIME_VALUE = SHD_PREDEFINED_PLAY_TIME;
EXPORT const int SHD_PREDEFINED_NOISE_SMALL_VALUE = SHD_PREDEFINED_NOISE_SMALL;
EXPORT const int SHD_PREDEFINED_NOISE_MEDIUM_VALUE =
                     SHD_PREDEFINED_NOISE_MEDIUM;
EXPORT const int SHD_PREDEFINED_BACK_BUFFER_VALUE = SHD_PREDEFINED_BACK_BUFFER;
EXPORT const int SHD_PREDEFINED_IMU_VALUE = SHD_PREDEFINED_IMU;

static bcm_context  *the_bcm;
static EGL_context  *the_EGL;
static LEDs_context *the_LEDs;
static exec         *the_exec;
static char         *the_info_log;

// Should pass in sizes?

EXPORT void shd_init(int LEDs_width, int LEDs_height)
{
    the_bcm = init_bcm();
    uint32_t bcm_surface    = bcm_get_surface(the_bcm);
    uint32_t surface_width  = bcm_get_surface_width(the_bcm);
    uint32_t surface_height = bcm_get_surface_height(the_bcm);
    uint32_t pixels_width   = surface_width / 2;
    uint32_t pixels_height  = surface_height / 2;
    uint32_t pixels_offset  = (pixels_height - LEDs_height) * pixels_width;
    the_EGL = init_EGL(bcm_surface, surface_width, surface_height);
    the_LEDs = init_LEDs(LEDs_width,
                         LEDs_height,
                         pixels_width,
                         pixels_height,
                         pixels_offset);
    the_exec = create_exec(the_bcm, the_LEDs);
}

EXPORT void shd_deinit(void)
{
    free(the_info_log);
    exec_stop(the_exec);
    // XXX destroy all programs
    destroy_exec(the_exec);
    deinit_LEDs(the_LEDs);
    deinit_EGL(the_EGL);
    deinit_bcm(the_bcm);
    the_info_log = NULL;
    the_exec = NULL;
    the_LEDs = NULL;
    the_EGL = NULL;
    the_bcm = NULL;
}

EXPORT void shd_start(void)
{
    exec_start(the_exec);
}

EXPORT void shd_stop(void)
{
    exec_stop(the_exec);
}

EXPORT double shd_fps(void)
{
    return exec_fps(the_exec);
}

EXPORT void shd_use_prog(shd_prog *pp)
{
    exec_use_prog(the_exec, pp);
}

EXPORT shd_prog *shd_create_prog(void)
{
    return create_prog();
}

EXPORT void shd_destroy_prog(shd_prog *prog)
{
    destroy_prog(prog);
}

EXPORT bool shd_prog_is_okay(const shd_prog *prog, char **info_log)
{
    free(the_info_log);
    the_info_log = NULL;
    bool ok = prog_is_okay(prog, &the_info_log);
    if (!ok && info_log)
        *info_log = the_info_log;
    return ok;
}

EXPORT bool shd_prog_attach_shader(shd_prog       *prog,
                                   shd_shader_type type,
                                   const char     *source)
{
    shader_type ptype;
    switch (type) {

    case SHD_SHADER_VERTEX:
        ptype = PST_VERTEX;
        break;

    case SHD_SHADER_FRAGMENT:
        ptype = PST_FRAGMENT;
        break;

    default:
        return false;
    }
    return prog_attach_shader(prog, ptype, source);
}

EXPORT bool shd_prog_attach_image(shd_prog   *pp,
                                  const char *name,
                                  size_t      width,
                                  size_t      height,
                                  uint8_t    *data)
{
    return prog_attach_image(pp, name, width, height, data);
}

EXPORT bool shd_prog_attach_predefined(shd_prog       *pp,
                                       const char     *name,
                                       shd_predefined  predef)
{
    predefined pd;
    switch (predef) {

    case SHD_PREDEFINED_RESOLUTION:
        pd = PD_RESOLUTION;
        break;

    case SHD_PREDEFINED_PLAY_TIME:
        pd = PD_PLAY_TIME;
        break;

    case SHD_PREDEFINED_NOISE_SMALL:
        pd = PD_NOISE_SMALL;
        break;

    case SHD_PREDEFINED_NOISE_MEDIUM:
        pd = PD_NOISE_MEDIUM;
        break;

    default:
        return false;
    }
    return prog_attach_predefined(pp, name, pd);
}
