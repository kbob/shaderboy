#include "leds.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "mpsse.h"

#define FRONT_PORCH_BYTES 7 /* could be 8 */
#define BACK_PORCH_BYTES 14

struct LEDs_context {
    size_t framebuffer_width;
    size_t framebuffer_height;
    size_t framebuffer_offset;
    size_t led_width;
    size_t led_height;
    size_t cmdbuf_size;
    size_t best_offset;
    size_t best_row_pitch;
    size_t best_buffer_size;
};

LEDs_context *init_LEDs(size_t led_width,
                        size_t led_height,
                        size_t framebuffer_width,
                        size_t framebuffer_height,
                        size_t framebuffer_offset)
{
    int ifnum = 0;
    const char *devstr = NULL;
    bool slow_clock = false;
    mpsse_init(ifnum, devstr, slow_clock);

    LEDs_context *ctx = calloc(1, sizeof *ctx);
    ctx->framebuffer_width  = framebuffer_width;
    ctx->framebuffer_height = framebuffer_height;
    ctx->framebuffer_offset = framebuffer_offset;
    ctx->led_width          = led_width;
    ctx->led_height         = led_height;
    size_t row_pix_bytes = led_width * sizeof (LED_pixel);
    size_t row_bytes = FRONT_PORCH_BYTES + row_pix_bytes + BACK_PORCH_BYTES;
    ctx->cmdbuf_size = led_height * row_bytes;
    ctx->best_offset = FRONT_PORCH_BYTES;
    ctx->best_row_pitch = (FRONT_PORCH_BYTES / sizeof (uint16_t) +
                           led_width +
                           BACK_PORCH_BYTES / sizeof (uint16_t));;
    ctx->best_buffer_size = led_height * ctx->best_row_pitch;
    return ctx;
}

void deinit_LEDs(LEDs_context *ctx)
{
    mpsse_close();
    free(ctx);
}

LED_pixel *LEDs_alloc_framebuffer(LEDs_context *ctx)
{
    size_t count = ctx->framebuffer_width * ctx->framebuffer_height;
    return calloc(count, sizeof (LED_pixel));
}

LED_cmd *LEDs_alloc_cmdbuffer(LEDs_context *ctx)
{
    return calloc(ctx->cmdbuf_size, sizeof (LED_cmd));
}

void LEDs_free_framebuffer(LED_pixel *framebuffer)
{
    free(framebuffer);
}

void LEDs_free_cmdbuffer(LED_cmd *cmdbuffer)
{
    free(cmdbuffer);
}

size_t LEDs_framebuffer_pitch(LEDs_context *ctx)
{
    return ctx->framebuffer_width;
}

size_t LEDs_best_buffer_size(const LEDs_context *ctx)
{
    return ctx->best_buffer_size;
}

size_t LEDs_best_offset(const LEDs_context *ctx)
{
    return ctx->best_offset;
}

size_t LEDs_best_row_pitch(const LEDs_context *ctx)
{
    return ctx->best_row_pitch;
}

void LEDs_create_cmds(LEDs_context *ctx,
                      const LED_pixel *pixels,
                      LED_cmd *cmds)
{
    size_t fb_row_pitch = ctx->framebuffer_width;
    size_t fb_offset    = ctx->framebuffer_offset;
    size_t row_size     = ctx->led_width * sizeof (LED_pixel);
    size_t row_count    = ctx->led_height;
    size_t cmd_idx      = 0;
    for (size_t row = 0; row < row_count; row++) {
        // Set CS low
        cmds[cmd_idx++] = 0x80; // MC_SETB_LOW
        cmds[cmd_idx++] = 0x00; // gpio
        cmds[cmd_idx++] = 0x2b; // dir

        // SPI packet header
        cmds[cmd_idx++] = 0x11;
        cmds[cmd_idx++] = (row_size + 1 - 1) & 0xFF;
        cmds[cmd_idx++] = (row_size + 1 - 1) >> 8;

        // SPI payload
        cmds[cmd_idx++] = 0x80;
        size_t y = row;
        memcpy(cmds + cmd_idx,
               &pixels[y * fb_row_pitch + fb_offset],
               row_size);
        cmd_idx += row_size;

        // Set CS high
        cmds[cmd_idx++] = 0x80; // MZC_SETB_LOW
        cmds[cmd_idx++] = 0x28; // gpio
        cmds[cmd_idx++] = 0x2b;  // dir

        // Set CS low
        cmds[cmd_idx++] = 0x80; // MC_SETB_LOW
        cmds[cmd_idx++] = 0x00; // gpio
        cmds[cmd_idx++] = 0x2b; // dir

        // SPI header
        cmds[cmd_idx++] = 0x11; // MC_DATA_OUT | MC_DATA_OCN
        cmds[cmd_idx++] = 2-1;
        cmds[cmd_idx++] = 0;

        // SPI payload
        cmds[cmd_idx++] = 0x03;
        cmds[cmd_idx++] = row;

        // Set CS high
        cmds[cmd_idx++] = 0x80; // MC_SETB_LOW
        cmds[cmd_idx++] = 0x28; // gpio
        cmds[cmd_idx++] = 0x2b; // dir
    }
    assert(cmd_idx == ctx->cmdbuf_size);
}

static void set_cs(int cs_b)
{
    uint8_t gpio = cs_b ? 0x28 : 0;
    uint8_t direction = 0x2b;
    mpsse_set_gpio(gpio, direction);
}

void LEDs_write_cmds(LEDs_context *ctx, const LED_cmd *cmds)
{
    mpsse_send_raw((uint8_t *)cmds, ctx->cmdbuf_size);

    // Swap
    unsigned char cmd_buf[2];
    set_cs(0);
    cmd_buf[0] = 0x04;
    cmd_buf[1] = 0x00;
    mpsse_send_spi(cmd_buf, 2);
    set_cs(1);
}

void LEDs_await_vsync(LEDs_context *ctx)
{
    uint8_t spi_buf[2];
    do {
        spi_buf[0] = 0x00;
        spi_buf[1] = 0x00;
        set_cs(0);
        mpsse_xfer_spi(spi_buf, 2);
        set_cs(1);
    } while (((spi_buf[0] | spi_buf[1]) & 0x02) != 0x02);
}
