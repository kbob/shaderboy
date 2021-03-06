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
    PD_FRAME,
    PD_NOISE_SMALL,
    PD_NOISE_MEDIUM,

    PD_UNKNOWN = -999,
} predefined;

typedef struct shd_prog prog;

extern prog          *create_prog(void);
extern void           destroy_prog(prog *);
extern bool           prog_is_okay(const prog *, char **info_log);
extern bool           prog_attach_shader(prog *,
                                         shader_type,
                                         const char *source);
extern bool           prog_attach_image(prog       *,
                                        const char *name,
                                        size_t      width,
                                        size_t      height,
                                        uint8_t    *data);
extern bool           prog_attach_predefined(prog       *,
                                             const char *name,
                                             predefined value);

extern GLuint         prog_instantiate(const prog *, char **info_log);
extern int            prog_id(const prog *);

extern size_t         prog_image_count(const prog *);
extern const char    *prog_image_name(const prog *, size_t index);
extern size_t         prog_image_width(const prog *, size_t index);
extern size_t         prog_image_height(const prog *, size_t index);
extern const uint8_t *prog_image_data(const prog *, size_t index);

extern size_t         prog_predefined_count(const prog *);
extern const char    *prog_predefined_name(const prog *, size_t index);
extern predefined     prog_predefined_value(const prog *, size_t index);

#endif /* !PROG_included */
