#ifndef EGL_included
#define EGL_included

#include <stdbool.h>
#include <stdint.h>

typedef struct EGL_context EGL_context;

extern EGL_context *init_EGL(uint32_t native_surface,
                             uint32_t surface_width,
                             uint32_t surface_height);
extern void         deinit_EGL(EGL_context *);

extern const char  *EGL_last_error(void);

extern void         EGL_swap_buffers(EGL_context *);

#endif /* !EGL_included */
