#include "render.h"

#include <stdlib.h>

#include <GLES2/gl2.h>

#include "egl.h"

static GLfloat vertices[] = {
    -1.0, -1.0, 0.0,
    +1.0, -1.0, 0.0,
    -1.0, +1.0, 0.0,
    +1.0, +1.0, 0.0,
};
static size_t vertex_count = ((&vertices)[1] - vertices) / 3;

struct render_state {
    EGL_context *egl;
    int         prog_id;
    GLuint      prog;
    GLint       vert_index;
};

render_state *render_init(const bcm_context bcm)
{    
    render_state *rs = calloc(1, sizeof *rs);

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
    free(rs);
}

void render_frame(render_state *rs, const prog *pp)
{
    if (rs->prog_id != prog_id(pp)) {
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

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, rs->vert_index, vertex_count);

    EGL_swap_buffers(rs->egl);
}
