#include "render.h"

#include <stdlib.h>
#include <time.h>

#include <GLES2/gl2.h>

#include "egl.h"

static GLfloat vertices[] = {
    -1.0, -1.0, 0.0,
    +1.0, -1.0, 0.0,
    -1.0, +1.0, 0.0,
    +1.0, +1.0, 0.0,
};
static size_t vertex_count = ((&vertices)[1] - vertices) / 3;

typedef struct pd_map {
    GLint      index;
    predefined value;
} pd_map;

struct render_state {
    bcm_context      bcm;
    EGL_context     *egl;
    int              prog_id;
    GLuint           prog;
    GLint            vert_index;
    struct timespec  time_zero;
    size_t           pd_count;
    pd_map          *pd_map;
};

render_state *render_init(const bcm_context bcm)
{    
    render_state *rs = calloc(1, sizeof *rs);

    rs->bcm = bcm;
    uint32_t bcm_surface = bcm_get_surface(bcm);
    uint32_t surface_width = bcm_get_surface_width(bcm);
    uint32_t surface_height = bcm_get_surface_height(bcm);
    rs->egl = init_EGL(bcm_surface, surface_width, surface_height);

    glViewport(0, 0, 128*6, 128);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    return rs;
}

void render_deinit(render_state *rs)
{
    glDeleteProgram(rs->prog);
    deinit_EGL(rs->egl);
    free(rs->pd_map);
    free(rs);
}

static void update_predefineds(render_state *rs, const prog *pp, bool new_prog)
{
    size_t pd_count = prog_predefined_count(pp);
    if (new_prog) {
        free(rs->pd_map);
        rs->pd_map = calloc(pd_count, sizeof *rs->pd_map);
        rs->pd_count = pd_count;
        for (size_t i = 0; i < pd_count; i++) {
            const char *name = prog_predefined_name(pp, i);
            predefined value = prog_predefined_value(pp, i);
            GLint index = glGetUniformLocation(rs->prog, name);
            rs->pd_map[i].index = index;
            rs->pd_map[i].value = value;
            switch (value) {
            case PD_RESOLUTION:
                glUniform3f(index,
                            (GLfloat)bcm_get_surface_width(rs->bcm),
                            (GLfloat)bcm_get_surface_height(rs->bcm),
                            (GLfloat)1.0);
                break;
            case PD_PLAY_TIME:
                clock_gettime(CLOCK_MONOTONIC, &rs->time_zero);
                glUniform1f(index, 0.0);
                break;
            default:
                break;
            }
        }
    }
    for (size_t i = 0; i < rs->pd_count; i++) {
        switch (rs->pd_map[i].value) {
        case PD_PLAY_TIME:
            if (!new_prog) {
                struct timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                GLfloat t = (now.tv_nsec - rs->time_zero.tv_nsec) / 1.0e9;
                t = now.tv_sec - rs->time_zero.tv_sec + t;
                glUniform1f(rs->pd_map[i].index, t);
            }
            break;
        default:
            break;
        }
    }
}

void render_frame(render_state *rs, const prog *pp)
{
    bool new_prog = rs->prog_id != prog_id(pp);
    if (new_prog) {
        rs->prog_id = prog_id(pp);
        glDeleteProgram(rs->prog);
        rs->prog = prog_instantiate((prog *)pp);
        glUseProgram(rs->prog);
        rs->vert_index = glGetAttribLocation(rs->prog, "vert");
        glVertexAttribPointer(rs->vert_index,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              vertices);
        glEnableVertexAttribArray(rs->vert_index);
    }

    update_predefineds(rs, pp, new_prog);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, rs->vert_index, vertex_count);

    EGL_swap_buffers(rs->egl);
}
