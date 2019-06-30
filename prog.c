#define _GNU_SOURCE
#include "prog.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLES2/gl2.h>

typedef struct image_info {
    char    *name;
    size_t   width;
    size_t   height;
    uint8_t *data;
} image_info;

typedef struct predefined_info {
    char       *name;
    predefined  value;
} predefined_info;

struct shd_prog {
    int              id;
    char            *vert_shader_source;
    char            *frag_shader_source;
    size_t           image_count;
    size_t           image_alloc;
    image_info *images;
    size_t           predef_count;
    size_t           predef_alloc;
    predefined_info *predefs;
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
    for (size_t i = 0; i < pp->image_count; i++) {
        free(pp->images[i].name);
        free(pp->images[i].data);
    }
    free(pp->images);
    for (size_t i = 0; i < pp->predef_count; i++)
        free(pp->predefs[i].name);
    free(pp->predefs);
    free(pp);
}

int prog_id(const prog *pp)
{
    return pp->id;
}

size_t prog_image_count(const prog *pp)
{
    return pp->image_count;
}

const char *prog_image_name(const prog *pp, size_t index)
{
    return pp->images[index].name;
}

size_t prog_image_width(const prog *pp, size_t index)
{
    return pp->images[index].width;
}

size_t prog_image_height(const prog *pp, size_t index)
{
    return pp->images[index].height;
}

const uint8_t *prog_image_data(const prog *pp, size_t index)
{
    return pp->images[index].data;
}

size_t prog_predefined_count(const prog *pp)
{
    return pp->predef_count;
}

const char *prog_predefined_name(const prog *pp, size_t index)
{
    if (index < pp->predef_count)
        return pp->predefs[index].name;
    return NULL;
}

predefined prog_predefined_value(const prog *pp, size_t index)
{
    if (index < pp->predef_count)
        return pp->predefs[index].value;
    return PD_UNKNOWN;
}

bool prog_is_okay(const prog *pp, char **info_log)
{
    GLuint prog = prog_instantiate(pp, info_log);

    // enumerate attributes.  Verify that "vert" is the only one.

    // enumerate uniforms.  Verify they are all accounted for.
    // for (name, value) in predefined:
    //     glGetUniformLocation(prog, name)
    //     glGetActiveUniform(...)
    //     check type, size against value.

    glDeleteProgram(prog);
    return prog != 0;
}

bool prog_attach_shader(prog *pp, shader_type type, const char *source)
{
    switch (type) {

    case PST_VERTEX:
        free(pp->vert_shader_source);
        pp->vert_shader_source = strdup(source);
        return true;

    case PST_FRAGMENT:
        free(pp->frag_shader_source);
        pp->frag_shader_source = strdup(source);
        return true;

    default:
        return false;
    }
}

bool prog_attach_image(prog       *pp,
                       const char *name,
                       size_t      width,
                       size_t      height,
                       uint8_t    *data)
{
    size_t n = pp->image_count;
    if (pp->image_alloc <= n) {
        size_t new_alloc = 2 * n + 10;
        pp->images = realloc(pp->images, new_alloc * sizeof *pp->images);
        pp->image_alloc = new_alloc;
    }
    // 4: four channels in RGBA
    size_t byte_count = width * height * 4 * sizeof *data;
    
    pp->images[n].name = strdup(name);
    pp->images[n].width = width;
    pp->images[n].height = height;
    pp->images[n].data = malloc(byte_count);
    memcpy(pp->images[n].data, data, byte_count);
    pp->image_count++;
    return true;
}

bool prog_attach_predefined(prog *pp, const char *name, predefined value)
{
    size_t n = pp->predef_count;
    if (pp->predef_alloc <= n) {
        size_t new_alloc = 2 * n + 10;
        pp->predefs = realloc(pp->predefs, new_alloc * sizeof *pp->predefs);
        pp->predef_alloc = new_alloc;
    }
    pp->predefs[n].name = strdup(name);
    pp->predefs[n].value = value;
    pp->predef_count++;
    return true;
}

static const char *GL_error_str(GLenum err)
{
    switch (err) {

    case GL_NO_ERROR:
        return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";

    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";

    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";

    default:
        return "unknown GL error";
    }
}

static void log_info(char **info_log, char *fmt, ...)
{
    if (!info_log)
        return;
    va_list ap;
    va_start(ap, fmt);
    vasprintf(info_log, fmt, ap);
    va_end(ap);
}

static GLuint create_shader(const prog *pp,
                            GLenum type,
                            const char *source,
                            char **info_log)
{
    GLuint s = glCreateShader(type);
    if (s == 0) {
        log_info(info_log,
                 "glCreateShader failed: %s", GL_error_str(glGetError()));
        return 0;
    }
    GLint length = -1;
    glShaderSource(s, 1, &source, &length);
    glCompileShader(s);
    if (info_log && !shader_is_ok(s)) {
        GLint info_len;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 0) {
            *info_log = malloc(info_len);
            glGetShaderInfoLog(s, info_len, NULL, *info_log);
        }
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint prog_instantiate(const prog *pp, char **info_log)
{
    GLuint v = 0, f = 0, p = 0;

    v = create_shader(pp, GL_VERTEX_SHADER, pp->vert_shader_source, info_log);
    if (!v)
        goto FAIL;
    f = create_shader(pp,
                      GL_FRAGMENT_SHADER,
                      pp->frag_shader_source,
                      info_log);
    if (!f)
        goto FAIL;
    p = glCreateProgram();
    if (!p) {
        log_info(info_log,
                 "glCreateProgram failed: %s", GL_error_str(glGetError()));
        goto FAIL;
    }

    // Link program.

    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    GLint link_status;
    glGetProgramiv(p, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
        if (info_log) {
            GLint info_len;
            glGetProgramiv(p, GL_INFO_LOG_LENGTH, &info_len);
            if (info_len > 0) {
                *info_log = malloc(info_len);
                glGetProgramInfoLog(p, info_len, NULL, *info_log);
            }
        }
        goto FAIL;
    }

    // Validate program.

    glValidateProgram(p);
    GLint valid_status;
    glGetProgramiv(p, GL_VALIDATE_STATUS, &valid_status);
    if (valid_status != GL_TRUE) {
        if (info_log) {
            GLint info_len;
            glGetProgramiv(p, GL_INFO_LOG_LENGTH, &info_len);
            if (info_len > 0) {
                *info_log = malloc(info_len);
                glGetProgramInfoLog(p, info_len, NULL, *info_log);
            }
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
