/*
 *  iceprog -- simple programming tool for FTDI-based Lattice iCE programmers
 *
 *  Copyright (C) 2015  Clifford Wolf <clifford@clifford.at>
 *  Copyright (C) 2018  Piotr Esden-Tempski <piotr@esden.net>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef MPSSE_H
#define MPSSE_H

#include <stdbool.h>

void mpsse_check_rx(void);
void mpsse_error(int status);
uint8_t mpsse_recv_byte(void);
void mpsse_send_byte(uint8_t data);
void mpsse_send_spi(uint8_t *data, int n);
void mpsse_xfer_spi(uint8_t *data, int n);
uint8_t mpsse_xfer_spi_bits(uint8_t data, int n);
void mpsse_set_gpio(uint8_t gpio, uint8_t direction);
int mpsse_readb_low(void);
int mpsse_readb_high(void);
void mpsse_send_dummy_bytes(uint8_t n);
void mpsse_send_dummy_bit(void);
void mpsse_init(int ifnum, const char *devstr, bool slow_clock);
void mpsse_close(void);
void mpsse_send_raw(uint8_t *data, int n);

#endif /* MPSSE_H */
