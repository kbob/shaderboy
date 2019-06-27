#ifndef SHADE_included
#define SHADE_included

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum shd_shader_type {
    SHD_SHADER_VERTEX,
    SHD_SHADER_FRAGMENT,
} shd_shader_type;

typedef enum shd_predefined {
    SHD_PREDEFINED_RESOLUTION,
    SHD_PREDEFINED_PLAY_TIME,
    SHD_PREDEFINED_NOISE_SMALL,
    SHD_PREDEFINED_NOISE_MEDIUM,
    SHD_PREDEFINED_BACK_BUFFER,
    SHD_PREDEFINED_IMU,
} shd_predefined;

// These are integer constants matching the enum values above.
// Python can't access the enum values directly.
extern const int SHD_SHADER_VERTEX_VALUE;
extern const int SHD_SHADER_FRAGMENT_VALUE;

extern const int SHD_PREDEFINED_RESOLUTION_VALUE;
extern const int SHD_PREDEFINED_PLAY_TIME_VALUE;
extern const int SHD_PREDEFINED_NOISE_SMALL_VALUE;
extern const int SHD_PREDEFINED_NOISE_MEDIUM_VALUE;
extern const int SHD_PREDEFINED_BACK_BUFFER_VALUE;
extern const int SHD_PREDEFINED_IMU_VALUE;

typedef struct shd_prog shd_prog;

extern void        shd_init(int LEDs_width, int LEDs_height);
extern void        shd_deinit(void);

extern void        shd_start(void);
extern void        shd_stop(void);
extern double      shd_fps(void);
extern void        shd_use_prog(shd_prog *);

extern shd_prog   *shd_create_prog(void);
extern void        shd_destroy_prog(shd_prog *);
extern bool        shd_prog_is_okay(shd_prog *);
extern const char *shd_prog_info_log(const shd_prog *);
extern void        shd_prog_attach_shader(shd_prog *,
                                          shd_shader_type type,
                                          const char      *source);
extern void        shd_prog_attach_image(shd_prog *,
                                         const char *name,
                                         size_t width,
                                         size_t height,
                                         uint32_t *values);
extern void        shd_prog_attach_predefined(shd_prog *,
                                              const char *name,
                                              shd_predefined);

#endif /* !SHADE_included */
