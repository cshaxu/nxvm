/* Copyright 2012-2026 Neko. */
#include "x86/devices/ppi8255/ppi8255_interface.h"

struct x86_ppi8255 {
    lib_u8 control;
    lib_u8 latch[3];
};

static lib_bool ppi_mode0(const x86_ppi8255 *ppi)
{
    return (ppi->control & 0x64u) == 0u;
}

lib_status x86_ppi8255_create(x86_ppi8255 **out_ppi)
{
    x86_ppi8255 *ppi;
    if (out_ppi == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_ppi = LIB_NULL;
    ppi = lib_allocate_zero(1u, sizeof(*ppi));
    if (ppi == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_ppi8255_reset(ppi);
    *out_ppi = ppi;
    return LIB_STATUS_OK;
}

void x86_ppi8255_destroy(x86_ppi8255 *ppi)
{
    lib_release(ppi);
}

void x86_ppi8255_reset(x86_ppi8255 *ppi)
{
    if (ppi == LIB_NULL) return;
    ppi->control = 0x9bu;
    lib_memory_set(ppi->latch, 0, sizeof(ppi->latch));
}

lib_status x86_ppi8255_output(const x86_ppi8255 *ppi, lib_u8 selector,
    x86_ppi8255_pins *out_pins)
{
    lib_u8 mask = 0u;
    if (ppi == LIB_NULL || selector > 2u || out_pins == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (ppi_mode0(ppi)) {
        if (selector == 0u) mask = (ppi->control & 0x10u) == 0u ? 0xffu : 0u;
        else if (selector == 1u) mask = (ppi->control & 0x02u) == 0u ? 0xffu : 0u;
        else mask = ((ppi->control & 0x01u) == 0u ? 0x0fu : 0u) |
            ((ppi->control & 0x08u) == 0u ? 0xf0u : 0u);
    }
    *out_pins = (x86_ppi8255_pins){ppi->latch[selector], mask};
    return LIB_STATUS_OK;
}

lib_status x86_ppi8255_read(const x86_ppi8255 *ppi, lib_u8 selector,
    lib_u8 input, lib_u8 *out_value)
{
    x86_ppi8255_pins pins;
    if (ppi == LIB_NULL || out_value == LIB_NULL || selector > 3u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (selector == 3u) return LIB_STATUS_UNSUPPORTED;
    if (!ppi_mode0(ppi)) {
        *out_value = selector == 0u ? ppi->latch[0] : selector == 1u ? input : 0u;
        return LIB_STATUS_OK;
    }
    x86_ppi8255_output(ppi, selector, &pins);
    *out_value = (pins.latch & pins.output_mask) | (input & (lib_u8)~pins.output_mask);
    return LIB_STATUS_OK;
}

lib_status x86_ppi8255_write(x86_ppi8255 *ppi, lib_u8 selector, lib_u8 value)
{
    if (ppi == LIB_NULL || selector > 3u) return LIB_STATUS_INVALID_ARGUMENT;
    if (selector < 3u) ppi->latch[selector] = value;
    else if ((value & 0x80u) != 0u) ppi->control = value;
    else {
        lib_u8 mask = (lib_u8)(1u << ((value >> 1u) & 0x07u));
        if ((value & 0x01u) != 0u) ppi->latch[2] |= mask;
        else ppi->latch[2] &= (lib_u8)~mask;
    }
    return LIB_STATUS_OK;
}
