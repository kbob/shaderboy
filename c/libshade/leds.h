#ifndef LEDS_included
#define LEDS_included

#include <stddef.h>
#include <stdint.h>

typedef struct LEDs_context LEDs_context;
typedef uint16_t LED_pixel;
typedef uint8_t LED_cmd;

extern LEDs_context *init_LEDs(size_t LEDs_width,
                               size_t LEDs_height,
                               size_t framebuffer_width,
                               size_t framebuffer_height,
                               size_t framebuffer_offset);
extern void          deinit_LEDs(LEDs_context *);

extern LED_pixel    *LEDs_alloc_framebuffer(LEDs_context *);
extern LED_cmd      *LEDs_alloc_cmdbuffer(LEDs_context *);
extern void          LEDs_free_framebuffer(LED_pixel *);
extern void          LEDs_free_cmdbuffer(LED_cmd *);
extern size_t        LEDs_framebuffer_pitch(LEDs_context *);

extern size_t        LEDs_best_buffer_size(const LEDs_context *);
extern size_t        LEDs_best_offset(const LEDs_context *);
extern size_t        LEDs_best_row_pitch(const LEDs_context *);

extern void          LEDs_create_cmds(LEDs_context *,
                                      const LED_pixel *,
                                      LED_cmd *);
extern void          LEDs_write_cmds(LEDs_context *, const LED_cmd *);

// extern void          LEDs_write_pixels(LEDs_context *,
//                                        LED_pixel *pixel_buf,
//                                        size_t     row_offset,
//                                        size_t     row_pitch);

extern void          LEDs_await_vsync(LEDs_context *);

#endif /* !LEDS_included */
