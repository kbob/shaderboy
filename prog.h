#ifndef PROG_included
#define PROG_included

#include <stdbool.h>
#include <stddef.h>

#include <GLES2/gl2.h>

typedef enum shader_type {
    PST_VERTEX,
    PST_FRAGMENT,
} shader_type;

typedef struct shd_prog prog;

extern prog       *create_prog(void);
extern void        destroy_prog(prog *);
extern bool        prog_is_okay(prog *);
extern const char *prog_info_log(const prog *);
extern void        prog_attach_shader(prog *, shader_type, const char *source);

extern GLuint      prog_instantiate(prog *);
extern int         prog_id(const prog *);

#endif /* !PROG_included */
