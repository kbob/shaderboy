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
    image_info      *images;
    size_t           predef_count;
    size_t           predef_alloc;
    predefined_info *predefs;
};

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

static bool check_uniform(GLuint prog,
                          const char *name,
                          GLint index,
                          GLenum expected_type,
                          GLint expected_size,
                          GLint name_max_length,
                          char **info_log)
{
    GLint u_size;
    GLenum u_type;
    char u_name[name_max_length];
    glGetActiveUniform(prog,
                       index,
                       sizeof u_name,
                       NULL,
                       &u_size,
                       &u_type,
                       u_name);
    if (u_type != expected_type || u_size != expected_size) {
        log_info(info_log,
                 "uniform %s: unexpected type/size %#0x/%d\n",
                 name, u_type, u_size);
        return false;
    }
    return true;
}

bool prog_is_okay(const prog *pp, char **info_log)
{
    GLuint prog = prog_instantiate(pp, info_log);
    if (prog == 0)
        return false;

    // Enumerate attributes.  Verify that "vert" is the only one.

    GLint attrib_count;
    glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &attrib_count);
    if (attrib_count != 1) {
        return false;
    }
    GLchar attrib_name[16];
    GLint attrib_size;
    GLenum attrib_type;
    glGetActiveAttrib(prog,
                      0,
                      sizeof attrib_name,
                      NULL,
                      &attrib_size,
                      &attrib_type,
                      attrib_name);
    if (strcmp(attrib_name, "vert") != 0) {
        log_info(info_log, "unexpected attribute: %s", attrib_name);
        return false;
    }
    if (attrib_type != GL_FLOAT_VEC3 || attrib_size != 1) {
        log_info(info_log, "`vert' attribute type/size should be GL_VEC3/1");
    }

    // Enumerate uniforms.  Verify each is bound with the correct
    // type and size.

    GLint uniform_count;
    glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &uniform_count);
    GLint uniform_max_length;
    glGetProgramiv(prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_max_length);
    bool uniform_bound[uniform_count];
    memset(uniform_bound, 0, sizeof uniform_bound);

    // Verify all images in pp.
    for (size_t i = 0; i < pp->image_count; i++) {
        const image_info *ip = &pp->images[i];
        GLint index = glGetUniformLocation(prog, ip->name);
        if (index == -1)
           continue;  // It's okay to provide unused uniforms.
        if (!check_uniform(prog,
                           ip->name,
                           index,
                           GL_SAMPLER_2D,
                           1,
                           uniform_max_length,
                           info_log))
            return false;
        uniform_bound[index] = true;
    }

    // Verify all predefineds in pp.
    for (size_t i = 0; i < pp->predef_count; i++) {
        const predefined_info *pip = &pp->predefs[i];
        GLint index = glGetUniformLocation(prog, pip->name);
        if (index == -1)
           continue;  // It's okay to provide unused uniforms.
        GLenum expected_type;
        GLint expected_size;
        switch (pip->value) {

        case PD_RESOLUTION:
            expected_type = GL_FLOAT_VEC3;
            expected_size = 1;
            break;

        case PD_PLAY_TIME:
            expected_type = GL_FLOAT;
            expected_size = 1;
            break;

        case PD_FRAME:
            expected_type = GL_INT;
            expected_size = 1;
            break;

        case PD_NOISE_SMALL:
            expected_type = GL_SAMPLER_2D;
            expected_size = 1;
            break;

        case PD_NOISE_MEDIUM:
            expected_type = GL_SAMPLER_2D;
            expected_size = 1;
            break;

        default:
            log_info(info_log, "unknown predefined %d", pip->value);
            return false;
        }
        if (!check_uniform(prog,
                           pip->name,
                           index,
                           expected_type,
                           expected_size,
                           uniform_max_length,
                           info_log)) {
            return false;
            }
        uniform_bound[index] = true;
    }

    for (size_t i = 0; i < uniform_count; i++) {
        if (!uniform_bound[i]) {
            char u_name[uniform_max_length];
            GLenum u_type;
            GLint u_size;
            glGetActiveUniform(prog,
                               i,
                               sizeof u_name,
                               NULL,
                               &u_size,
                               &u_type,
                               u_name);
            log_info(info_log, "uniform %s is not bound.", u_name);
            return false;
            
        }
    }
    
    // 
    // get max uniform index
    // alloc flag.

    // for all in pp.images:
    //   if variable in shader:
    //       verify type

    // for all in pp.predef:
    //   if variable in shader:
    //     verify type matches predef type.

    // for all shader uniforms:
    //   verify either in predef or 

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
