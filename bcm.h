#ifndef BCM_included
#define BCM_included

#include <stdint.h>

typedef void *bcm_context;

extern bcm_context init_bcm(void);
extern void        deinit_bcm(bcm_context);

extern int  bcm_get_surface_width(const bcm_context);
extern int  bcm_get_surface_height(const bcm_context);
extern int  bcm_get_framebuffer_width(const bcm_context);
extern int  bcm_get_framebuffer_height(const bcm_context);
extern int  bcm_get_surface(const bcm_context);

// returns zero on success
extern int  bcm_read_pixels(bcm_context,
                            uint16_t *pixels,
                            uint16_t row_pitch);

extern const char *bcm_last_error(void);

#endif /* !BCM_included */
