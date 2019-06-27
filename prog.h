#ifndef PROG_included
#define PROG_included

#include <stdbool.h>
#include <stddef.h>

#include <GLES2/gl2.h>

typedef enum shader_type {
    PST_VERTEX,
    PST_FRAGMENT,
} shader_type;

typedef enum predefined {
    PD_RESOLUTION,
    PD_PLAY_TIME,

    PD_UNKNOWN = -999,
} predefined;

typedef struct shd_prog prog;

extern prog       *create_prog(void);
extern void        destroy_prog(prog *);
extern bool        prog_is_okay(prog *);
extern const char *prog_info_log(const prog *);
extern void        prog_attach_shader(prog *, shader_type, const char *source);
extern void        prog_attach_predefined(prog *,
                                          const char *name,
                                          predefined value);

extern GLuint      prog_instantiate(prog *);
extern int         prog_id(const prog *);
extern size_t      prog_predefined_count(const prog *);
extern const char *prog_predefined_name(const prog *, size_t index);
extern predefined  prog_predefined_value(const prog *, size_t index);

#endif /* !PROG_included */
