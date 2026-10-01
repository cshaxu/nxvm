/* Copyright 2012-2026 Neko. */
#ifndef X86_PPI8255_INTERFACE_H
#define X86_PPI8255_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_ppi8255 x86_ppi8255;
typedef struct x86_ppi8255_pins {
    lib_u8 latch;
    lib_u8 output_mask;
} x86_ppi8255_pins;

/* Qualified Mode-0 register model only. Selectors 0/1/2 are A/B/C; 3 is
 * write-only control. Inputs are sampled copied values. No ports or wiring.
 * Mode-1/2 encodings retain the existing inactive model: A reads its latch,
 * B samples input, C reads zero, and no output is driven. A mode-set write
 * retains latches in this qualified model; reset clears them. This is not a
 * claim of complete 8255 silicon behavior. One stopped-before-destroy owner. */
lib_status x86_ppi8255_create(x86_ppi8255 **out_ppi);
void x86_ppi8255_destroy(x86_ppi8255 *ppi);
void x86_ppi8255_reset(x86_ppi8255 *ppi);
lib_status x86_ppi8255_read(const x86_ppi8255 *ppi, lib_u8 selector,
    lib_u8 input, lib_u8 *out_value);
lib_status x86_ppi8255_write(x86_ppi8255 *ppi, lib_u8 selector, lib_u8 value);
/* Copy one port's output latch and drive mask for physical board wiring. */
lib_status x86_ppi8255_output(const x86_ppi8255 *ppi, lib_u8 selector,
    x86_ppi8255_pins *out_pins);
#endif
