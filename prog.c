#define _GNU_SOURCE
#include "prog.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLES2/gl2.h>

struct shd_prog {
    int  id;
    char *vert_shader_source;
    char *frag_shader_source;
    char *info_log;
};

static bool shader_is_ok(GLuint shader)
{
    GLint status = ~GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    return status == GL_TRUE;
}

prog *create_prog(void)
{
    static int next_id;
    prog *pp = calloc(1, sizeof *pp);
    pp->id = ++next_id;
    return pp;
}

void destroy_prog(prog *pp)
{
    free(pp->vert_shader_source);
    free(pp->frag_shader_source);
    free(pp->info_log);
    free(pp);
}

int prog_id(const prog *pp)
{
    return pp->id;
}

bool prog_is_okay(prog *pp)
{
    if (pp->info_log)
        return false;
    GLuint prog = prog_instantiate(pp);
    glDeleteProgram(prog);
    return prog != 0;
}

const char *prog_info_log(const prog *pp)
{
    return pp->info_log;
}

void prog_attach_shader(prog *pp, shader_type type, const char *source)
{
    switch (type) {

    case PST_VERTEX:
        free(pp->vert_shader_source);
        pp->vert_shader_source = strdup(source);
        break;

    case PST_FRAGMENT:
        free(pp->frag_shader_source);
        pp->frag_shader_source = strdup(source);
        break;

    default:
        if (!pp->info_log)
            asprintf(&pp->info_log, "unknown shader type %d", type);
        return;
    }
}

static GLuint create_shader(prog *pp, GLenum type, const char *source)
{
    GLuint s = glCreateShader(type);
    if (s == 0)
        return 0;
    GLint length = -1;
    glShaderSource(s, 1, &source, &length);
    glCompileShader(s);
    if (!shader_is_ok(s)) {
        GLint info_len;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 0) {
            pp->info_log = malloc(info_len);
            glGetShaderInfoLog(s, info_len, NULL, pp->info_log);
        }
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint prog_instantiate(prog *pp)
{
    GLuint v = 0, f = 0, p = 0;

    if (pp->info_log)
        return 0;
    v = create_shader(pp, GL_VERTEX_SHADER, pp->vert_shader_source);
    if (!v)
        goto FAIL;
    f = create_shader(pp, GL_FRAGMENT_SHADER, pp->frag_shader_source);
    if (!f)
        goto FAIL;
    p = glCreateProgram();
    if (!p) {
        // glGetError
        pp->info_log = strdup("glCreateProgram failed");
        goto FAIL;
    }

    // Link program.

    glAttachShader(p, v);
    glAttachShader(p, f);
    // glBindAttribLocation(p, 0, "vert");
    glLinkProgram(p);
    GLint link_status;
    glGetProgramiv(p, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
        GLint info_len;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 0) {
            pp->info_log = malloc(info_len);
            glGetProgramInfoLog(p, info_len, NULL, pp->info_log);
        }
        goto FAIL;
    }

    // Validate program.

    glValidateProgram(p);
    GLint valid_status;
    glGetProgramiv(p, GL_VALIDATE_STATUS, &valid_status);
    if (valid_status != GL_TRUE) {
        GLint info_len;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 0) {
            pp->info_log = malloc(info_len);
            glGetProgramInfoLog(p, info_len, NULL, pp->info_log);
        }
        goto FAIL;
    }

    return p;

FAIL:
    if (p)
        glDeleteProgram(p);
    if (f)
        glDeleteShader(f);
    if (v)
        glDeleteShader(v);
    return 0;
}
