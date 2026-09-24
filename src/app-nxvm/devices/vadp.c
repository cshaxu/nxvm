/* Copyright 2012-2014 Neko. */

/* Core-owned CGA text-controller state. Host presentation is outside core. */
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/device_support.h"



#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/vadp.h"

#define CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE 0x01u
#define CORE_MACHINE_VADP_STATUS_LIGHTPEN_TRIGGER 0x02u
#define CORE_MACHINE_VADP_STATUS_LIGHTPEN_SWITCH_OPEN 0x04u
#define CORE_MACHINE_VADP_STATUS_VERTICAL_RETRACE 0x08u
#define CORE_MACHINE_VADP_DEFAULT_ACTIVE_DISPLAY_TICKS 48u
#define CORE_MACHINE_VADP_DEFAULT_HORIZONTAL_BLANK_TICKS 8u
#define CORE_MACHINE_VADP_DEFAULT_VERTICAL_RETRACE_TICKS 8u
#define CORE_MACHINE_VADP_CRTC_HORIZONTAL_TOTAL 0x00u
#define CORE_MACHINE_VADP_CRTC_HORIZONTAL_DISPLAYED 0x01u
#define CORE_MACHINE_VADP_CRTC_HORIZONTAL_SYNC_POSITION 0x02u
#define CORE_MACHINE_VADP_CRTC_SYNC_WIDTH 0x03u
#define CORE_MACHINE_VADP_CRTC_VERTICAL_TOTAL 0x04u
#define CORE_MACHINE_VADP_CRTC_VERTICAL_TOTAL_ADJUST 0x05u
#define CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAYED 0x06u
#define CORE_MACHINE_VADP_CRTC_OVERFLOW 0x07u
#define CORE_MACHINE_VADP_CRTC_VERTICAL_SYNC_POSITION 0x07u
#define CORE_MACHINE_VADP_CRT_INTERLACE_SKEW 0x08u
#define CORE_MACHINE_VADP_CRTC_MAXIMUM_RASTER_ADDRESS 0x09u
#define CORE_MACHINE_VADP_CRTC_CURSOR_TOP 0x0au
#define CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM 0x0bu
#define CORE_MACHINE_VADP_CRTC_START_HIGH 0x0cu
#define CORE_MACHINE_VADP_CRTC_START_LOW 0x0du
#define CORE_MACHINE_VADP_CRTC_CURSOR_HIGH 0x0eu
#define CORE_MACHINE_VADP_CRTC_CURSOR_LOW 0x0fu
#define CORE_MACHINE_VADP_CRTC_OFFSET 0x13u
#define CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAY_END 0x12u
#define CORE_MACHINE_VADP_CRTC_VERTICAL_RETRACE_END 0x11u
#define CORE_MACHINE_VADP_CRTC_UNDERLINE_LOCATION 0x14u
#define CORE_MACHINE_VADP_CRTC_END_VERTICAL_BLANK 0x16u
#define CORE_MACHINE_VADP_MODE_GRAPHICS 0x02u
#define CORE_MACHINE_VADP_MODE_VIDEO_ENABLE 0x08u
#define CORE_MACHINE_VADP_MODE_HIGH_RES 0x10u
#define CORE_MACHINE_VADP_COLOR_PALETTE_SELECT 0x20u
#define CORE_MACHINE_VADP_PORT_CGA_LIGHTPEN_CLEAR 0x03dbu
#define CORE_MACHINE_VADP_PORT_CGA_LIGHTPEN_PRESET 0x03dcu
#define CORE_MACHINE_VADP_GRAPHICS_BYTES_PER_ROW 80u
#define CORE_MACHINE_VADP_GRAPHICS_ODD_ROW_OFFSET 0x2000u
#define CORE_MACHINE_VADP_EGA_320X200_ROW_BYTES 40u
#define CORE_MACHINE_VADP_EGA_640X200_ROW_BYTES 80u
#define CORE_MACHINE_VADP_EGA_640X350_ROW_BYTES 80u
#define CORE_MACHINE_VADP_EGA_320X200_CRTC_OFFSET 20u
#define CORE_MACHINE_VADP_EGA_640X200_CRTC_OFFSET 40u
#define CORE_MACHINE_VADP_EGA_640X350_CRTC_OFFSET 40u

_Static_assert(CORE_MACHINE_VADP_CRTC_CURSOR_TOP <
        CORE_MACHINE_VADP_CRTC_REGISTER_COUNT &&
    CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM <
        CORE_MACHINE_VADP_CRTC_REGISTER_COUNT &&
    CORE_MACHINE_VADP_CRTC_START_HIGH + 1u <
        CORE_MACHINE_VADP_CRTC_REGISTER_COUNT &&
    CORE_MACHINE_VADP_CRTC_CURSOR_HIGH + 1u <
        CORE_MACHINE_VADP_CRTC_REGISTER_COUNT &&
    CORE_MACHINE_VADP_CRTC_OFFSET < CORE_MACHINE_VADP_CRTC_REGISTER_COUNT,
    "CRTC constant indices must fit the VADP CRTC register bank");

static void core_machine_vadp_mark_dirty(t_vadp *adapter);
static lib_i32 core_machine_vadp_ega_display_kind(const t_vadp *adapter,
    core_machine_display_kind *out_kind);

static lib_status core_machine_vadp_cga_read(void *owner,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (adapter == LIB_NULL || destination == 0u || physical < CORE_MACHINE_VADP_VIDEO_BASE ||
        (lib_u64)physical - CORE_MACHINE_VADP_VIDEO_BASE + bytes >
        CORE_MACHINE_VADP_VIDEO_BYTES) return LIB_STATUS_UNSUPPORTED;
    lib_memory_copy((void *)destination, adapter->data.cga_vram +
        physical - CORE_MACHINE_VADP_VIDEO_BASE, bytes);
    return LIB_STATUS_OK;
}

static lib_status core_machine_vadp_cga_write(void *owner,
    lib_u32 physical, lib_uptr source,
    lib_uptr bytes)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (adapter == LIB_NULL || source == 0u || physical < CORE_MACHINE_VADP_VIDEO_BASE ||
        (lib_u64)physical - CORE_MACHINE_VADP_VIDEO_BASE + bytes >
        CORE_MACHINE_VADP_VIDEO_BYTES) return LIB_STATUS_UNSUPPORTED;
    lib_memory_copy(adapter->data.cga_vram + physical - CORE_MACHINE_VADP_VIDEO_BASE,
        (const void *)source, bytes);
    core_machine_vadp_mark_dirty(adapter);
    return LIB_STATUS_OK;
}

static lib_status core_machine_vadp_cga_query(void *owner,
    lib_u32 physical, lib_uptr bytes,
    core_machine_memory_access access)
{
    (void)owner;
    return (access == CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == CORE_MACHINE_MEMORY_ACCESS_WRITE) &&
        physical >= CORE_MACHINE_VADP_VIDEO_BASE &&
        (lib_u64)physical - CORE_MACHINE_VADP_VIDEO_BASE + bytes <=
        CORE_MACHINE_VADP_VIDEO_BYTES ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_i32 core_machine_vadp_is_graphics_mode(const t_vadp *adapter)
{
    return adapter != LIB_NULL &&
        (adapter->data.mode_control & (CORE_MACHINE_VADP_MODE_GRAPHICS |
            CORE_MACHINE_VADP_MODE_HIGH_RES)) == CORE_MACHINE_VADP_MODE_GRAPHICS;
}

static lib_i32 core_machine_vadp_is_high_res_graphics_mode(const t_vadp *adapter)
{
    return adapter != LIB_NULL && (adapter->data.mode_control &
        (CORE_MACHINE_VADP_MODE_GRAPHICS | CORE_MACHINE_VADP_MODE_HIGH_RES)) ==
        (CORE_MACHINE_VADP_MODE_GRAPHICS | CORE_MACHINE_VADP_MODE_HIGH_RES);
}

static lib_u32 core_machine_vadp_rgbi_color(lib_u8 index)
{
    static const lib_u32 colors[16] = {
        0x000000u, 0x0000aau, 0x00aa00u, 0x00aaaau,
        0xaa0000u, 0xaa00aau, 0xaa5500u, 0xaaaaaau,
        0x555555u, 0x5555ffu, 0x55ff55u, 0x55ffffu,
        0xff5555u, 0xff55ffu, 0xffff55u, 0xffffffu
    };

    return colors[index & 0x0fu];
}

/* The Compaq CECG guide names its six digital palette bits r g b R G B,
 * from bit 5 through bit 0. Snapshot RGB expands each primary/secondary
 * pair to the project 0x00..0xff capture range; it is not a monitor model. */
static lib_u32 core_machine_vadp_compaq_ega_color(lib_u8 value)
{
    lib_u8 red = (lib_u8)(((value >> 2u) & 1u) * 2u +
        ((value >> 5u) & 1u));
    lib_u8 green = (lib_u8)(((value >> 1u) & 1u) * 2u +
        ((value >> 4u) & 1u));
    lib_u8 blue = (lib_u8)(((value >> 0u) & 1u) * 2u +
        ((value >> 3u) & 1u));

    return ((lib_u32)red * 0x55u << 16u) |
        ((lib_u32)green * 0x55u << 8u) |
        (lib_u32)blue * 0x55u;
}

static lib_u32 core_machine_vadp_ega_palette_color(const t_vadp *adapter,
    lib_u8 value)
{
    if (adapter != LIB_NULL && adapter->data.ega_personality ==
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        return core_machine_vadp_compaq_ega_color(value & 0x3fu);
    }
    return core_machine_vadp_rgbi_color(value & 0x0fu);
}

static lib_i32 core_machine_vadp_ega_output_active(const t_vadp *adapter);

static void core_machine_vadp_graphics_palette(const t_vadp *adapter,
    lib_u32 palette[4])
{
    lib_i32 alternate;
    lib_i32 intensified;

    if (adapter == LIB_NULL || palette == LIB_NULL) return;
    alternate = (adapter->data.color_select &
        CORE_MACHINE_VADP_COLOR_PALETTE_SELECT) != 0u;
    intensified = (adapter->data.color_select & 0x10u) != 0u;
    palette[0] = core_machine_vadp_rgbi_color(adapter->data.color_select);
    palette[1] = core_machine_vadp_rgbi_color((alternate ? 3u : 2u) |
        (intensified ? 8u : 0u));
    palette[2] = core_machine_vadp_rgbi_color((alternate ? 5u : 4u) |
        (intensified ? 8u : 0u));
    palette[3] = core_machine_vadp_rgbi_color((alternate ? 7u : 6u) |
        (intensified ? 8u : 0u));
    if ((adapter->data.ega_controller_configured &&
        !core_machine_vadp_ega_output_active(adapter)) ||
        (!adapter->data.ega_controller_configured &&
        (adapter->data.mode_control & CORE_MACHINE_VADP_MODE_VIDEO_ENABLE) == 0u)) {
        palette[0] = 0u;
        palette[1] = 0u;
        palette[2] = 0u;
        palette[3] = 0u;
    }
}

static void core_machine_vadp_high_res_palette(const t_vadp *adapter,
    lib_u32 palette[CORE_MACHINE_DISPLAY_PALETTE_ENTRIES])
{
    if (adapter == LIB_NULL || palette == LIB_NULL) return;
    palette[0] = 0u;
    palette[1] = (adapter->data.mode_control & CORE_MACHINE_VADP_MODE_VIDEO_ENABLE) != 0u ?
        core_machine_vadp_rgbi_color(adapter->data.color_select & 0x0fu) : 0u;
}

static void core_machine_vadp_active_ega_aperture(const t_vadp *adapter,
    lib_u32 *out_base, lib_u32 *out_bytes);

static lib_i32 core_machine_vadp_ega_output_active(const t_vadp *adapter)
{
    return adapter != LIB_NULL && adapter->data.ega_planar_enabled &&
        adapter->data.ega_planar_vram != 0u &&
        adapter->data.ega_sequencer_configured &&
        (adapter->data.sequencer[0] & 0x03u) == 0x03u &&
        adapter->data.ega_controller_configured &&
        (adapter->data.graphics[5] & 0x04u) == 0u &&
        adapter->data.attribute_display_enabled;
}

/* CPU access to the EGA aperture is a mapping decision, not a presentation
 * decision.  In particular, firmware may clear text memory while display
 * output is disabled.  Letting that access fall through to ordinary RAM
 * creates a second owner for video state and corrupts board aliases which
 * legitimately reuse the conventional video-hole backing. */
static lib_i32 core_machine_vadp_ega_aperture_mapped(const t_vadp *adapter)
{
    return adapter != LIB_NULL && adapter->data.ega_planar_enabled &&
        adapter->data.ega_planar_vram != 0u && adapter->data.ega_sequencer_configured &&
        (adapter->data.sequencer[0] & 0x03u) == 0x03u &&
        adapter->data.ega_controller_configured;
}

static lib_i32 core_machine_vadp_ega_cpu_aperture_active(const t_vadp *adapter)
{
    return core_machine_vadp_ega_aperture_mapped(adapter) &&
        (adapter->data.graphics[5] & 0x04u) == 0u &&
        (adapter->data.ega_personality !=
            CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR ||
         !adapter->data.compaq_cpu_video_memory_disabled);
}

static lib_i32 core_machine_vadp_ega_planar_active(const t_vadp *adapter)
{
    return core_machine_vadp_ega_output_active(adapter) &&
        (adapter->data.attribute[16] & 0x01u) != 0u;
}

static lib_i32 core_machine_vadp_vga_chain4_active(const t_vadp *adapter)
{
    return core_machine_vadp_ega_cpu_aperture_active(adapter) &&
        adapter->data.vga_configured && (adapter->data.sequencer[4] & 0x08u) != 0u;
}

static lib_i32 core_machine_vadp_vga_mode13_active(const t_vadp *adapter)
{
    return core_machine_vadp_vga_chain4_active(adapter) &&
        (adapter->data.graphics[5] & 0x40u) != 0u;
}

static lib_i32 core_machine_vadp_ega_planar_display_active(const t_vadp *adapter)
{
    core_machine_display_kind kind;

    return core_machine_vadp_ega_planar_active(adapter) &&
        core_machine_vadp_ega_display_kind(adapter, &kind);
}

static lib_u8 core_machine_vadp_rotate_right(lib_u8 value, lib_u8 count)
{
    count &= 7u;
    return count == 0u ? value : (lib_u8)((value >> count) |
        (value << (8u - count)));
}

static lib_u8 core_machine_vadp_logical_operation(lib_u8 operation,
    lib_u8 source, lib_u8 latch)
{
    switch (operation & 0x03u) {
    case 1u: return source & latch;
    case 2u: return source | latch;
    case 3u: return source ^ latch;
    default: return source;
    }
}

static lib_u8 core_machine_vadp_ega_color_compare(
    const t_vadp *adapter)
{
    lib_u8 value = 0xffu;
    lib_u8 plane;

    if (adapter == LIB_NULL) return 0u;
    for (plane = 0u; plane < CORE_MACHINE_VADP_EGA_PLANES; ++plane) {
        if ((adapter->data.graphics[7] & (1u << plane)) != 0u) continue;
        value &= (adapter->data.graphics[2] & (1u << plane)) != 0u ?
            adapter->data.ega_latches[plane] :
            (lib_u8)~adapter->data.ega_latches[plane];
    }
    return value;
}

static lib_u8 core_machine_vadp_ega_write_source(const t_vadp *adapter,
    lib_u8 input, lib_u8 plane)
{
    lib_u8 mode = adapter->data.graphics[5] & 0x03u;

    if (mode == 1u) return adapter->data.ega_latches[plane];
    if (mode == 2u) return (input & (1u << plane)) != 0u ? 0xffu : 0u;
    return (adapter->data.graphics[1] & (1u << plane)) != 0u ?
        (adapter->data.graphics[0] & (1u << plane)) != 0u ? 0xffu : 0u :
        core_machine_vadp_rotate_right(input, adapter->data.graphics[3]);
}

static lib_i32 core_machine_vadp_compaq_odd_even_page_active(const t_vadp *adapter)
{
    return adapter != LIB_NULL && adapter->data.ega_personality ==
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR &&
        (adapter->data.sequencer[4] & 0x04u) == 0u &&
        (adapter->data.graphics[6] & 0x02u) != 0u;
}

/* The selected Compaq primary/only CECG route is 3Dx/B800h.  Its firmware
 * nevertheless uses B0000h for POST output while GDC map 3 is selected.
 * On the real board that compatibility write is still video memory, never
 * ordinary system RAM.  Keep it in the one planar store by canonically
 * routing the B0000h compatibility window to the selected B8000h window. */
static lib_i32 core_machine_vadp_compaq_b000_compatibility_contains(
    const t_vadp *adapter, lib_u32 physical, lib_uptr bytes)
{
    lib_u64 request_end = (lib_u64)physical + bytes;

    return adapter != LIB_NULL && bytes != 0u && adapter->data.ega_personality ==
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR &&
        core_machine_vadp_ega_cpu_aperture_active(adapter) &&
        ((adapter->data.graphics[6] >> 2u) & 0x03u) == 3u &&
        physical >= 0x000b0000u && request_end <= 0x000b8000u;
}

static lib_u32 core_machine_vadp_ega_planar_offset(const t_vadp *adapter,
    lib_u32 physical)
{
    lib_u32 aperture_base;
    lib_u32 aperture_bytes;
    lib_u32 offset;

    if (core_machine_vadp_compaq_b000_compatibility_contains(adapter, physical, 1u)) {
        physical += 0x00008000u;
    }
    core_machine_vadp_active_ega_aperture(adapter, &aperture_base, &aperture_bytes);
    offset = physical - aperture_base;
    if (aperture_bytes == 0x00020000u) offset &= 0x0000ffffu;
    else if (aperture_bytes == 0x00008000u) offset &= 0x00007fffu;
    if ((adapter->data.sequencer[4] & 0x02u) == 0u) offset &= 0x00003fffu;

    if (core_machine_vadp_compaq_odd_even_page_active(adapter)) {
        return ((offset >> 1u) & (CORE_MACHINE_VADP_EGA_ODD_EVEN_PAGE_BYTES - 1u)) |
            (adapter->data.compaq_odd_even_high_page ?
            CORE_MACHINE_VADP_EGA_ODD_EVEN_PAGE_BYTES : 0u);
    }
    return offset;
}

static lib_i32 core_machine_vadp_ega_cpu_aperture_contains(const t_vadp *adapter,
    lib_u32 physical, lib_uptr bytes)
{
    return core_machine_vadp_ega_cpu_aperture_active(adapter) &&
        (core_machine_vadp_ega_aperture_contains(adapter, physical, bytes) ||
        core_machine_vadp_compaq_b000_compatibility_contains(adapter, physical, bytes));
}

static lib_status core_machine_vadp_ega_planar_read(void *owner,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_uptr index;
    lib_u8 *out = (lib_u8 *)destination;

    if (adapter == LIB_NULL || destination == 0u ||
        !core_machine_vadp_ega_aperture_mapped(adapter)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if (!core_machine_vadp_ega_cpu_aperture_contains(adapter, physical, bytes)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    for (index = 0u; index < bytes; ++index) {
        lib_u32 address = physical + (lib_u32)index;
        lib_u32 offset = core_machine_vadp_vga_chain4_active(adapter) ?
            core_machine_vadp_ega_planar_offset(adapter, address) >> 2u :
            core_machine_vadp_ega_planar_offset(adapter, address);
        lib_u8 plane;

        for (plane = 0u; plane < CORE_MACHINE_VADP_EGA_PLANES; ++plane) {
            adapter->data.ega_latches[plane] = ((lib_u8 *)adapter->data.ega_planar_vram)
                [(lib_size)plane * CORE_MACHINE_VADP_EGA_PLANE_BYTES + offset];
        }
        if ((adapter->data.graphics[5] & 0x08u) != 0u) {
            out[index] = core_machine_vadp_ega_color_compare(adapter);
        } else {
            lib_u8 map = core_machine_vadp_vga_chain4_active(adapter) ?
                (lib_u8)(address & 3u) : adapter->data.graphics[4];

            out[index] = map < CORE_MACHINE_VADP_EGA_PLANES ?
                adapter->data.ega_latches[map] : 0u;
        }
    }
    return LIB_STATUS_OK;
}

static lib_status core_machine_vadp_ega_planar_write(void *owner,
    lib_u32 physical, lib_uptr source,
    lib_uptr bytes)
{
    t_vadp *adapter = (t_vadp *)owner;
    const lib_u8 *input = (const lib_u8 *)source;
    lib_uptr index;

    if (adapter == LIB_NULL || source == 0u ||
        !core_machine_vadp_ega_aperture_mapped(adapter)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if (!core_machine_vadp_ega_cpu_aperture_contains(adapter, physical, bytes)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if ((adapter->data.graphics[5] & 0x03u) == 0x03u) {
        return LIB_STATUS_UNSUPPORTED;
    }
    for (index = 0u; index < bytes; ++index) {
        lib_u32 address = physical + (lib_u32)index;
        lib_u32 offset = core_machine_vadp_vga_chain4_active(adapter) ?
            core_machine_vadp_ega_planar_offset(adapter, address) >> 2u :
            core_machine_vadp_ega_planar_offset(adapter, address);
        lib_u8 plane;

        for (plane = 0u; plane < CORE_MACHINE_VADP_EGA_PLANES; ++plane) {
            lib_u8 *target = (lib_u8 *)adapter->data.ega_planar_vram +
                (lib_size)plane * CORE_MACHINE_VADP_EGA_PLANE_BYTES + offset;
            lib_u8 source_byte = core_machine_vadp_ega_write_source(adapter,
                input[index], plane);
            lib_u8 merged = core_machine_vadp_logical_operation(
                adapter->data.graphics[3] >> 3, source_byte,
                adapter->data.ega_latches[plane]);

            if ((!core_machine_vadp_vga_chain4_active(adapter) ||
                plane == (address & 3u)) &&
                (adapter->data.sequencer[2] & (1u << plane)) != 0u) {
                *target = (adapter->data.graphics[5] & 0x03u) == 1u ?
                    adapter->data.ega_latches[plane] :
                    (lib_u8)((merged & adapter->data.graphics[8]) |
                    (adapter->data.ega_latches[plane] &
                    (lib_u8)~adapter->data.graphics[8]));
            }
        }
    }
    core_machine_vadp_mark_dirty(adapter);
    return LIB_STATUS_OK;
}

static lib_status core_machine_vadp_ega_planar_query(void *owner,
    lib_u32 physical, lib_uptr bytes,
    core_machine_memory_access access)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (adapter == LIB_NULL ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != CORE_MACHINE_MEMORY_ACCESS_WRITE) ||
        !core_machine_vadp_ega_cpu_aperture_contains(adapter, physical, bytes)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static lib_i32 core_machine_vadp_cga_logical_raster_active(const t_vadp *adapter)
{
    return adapter != LIB_NULL && !adapter->data.ega_controller_configured &&
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_HORIZONTAL_DISPLAYED] != 0u &&
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAYED] != 0u;
}

static lib_u32 core_machine_vadp_cga_scanlines_per_row(const t_vadp *adapter)
{
    return (lib_u32)adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_MAXIMUM_RASTER_ADDRESS] + 1u;
}

static lib_u32 core_machine_vadp_cga_logical_raster_period(const t_vadp *adapter)
{
    lib_u32 rows = (lib_u32)adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_VERTICAL_TOTAL] + 1u;
    lib_u32 scanlines = rows * core_machine_vadp_cga_scanlines_per_row(adapter) +
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_VERTICAL_TOTAL_ADJUST];

    return scanlines * ((lib_u32)adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_HORIZONTAL_TOTAL] + 1u);
}

static lib_u16 core_machine_vadp_text_columns(const t_vadp *adapter)
{
    lib_u16 columns;

    if (!core_machine_vadp_cga_logical_raster_active(adapter)) return adapter->data.columns;
    columns = adapter->data.crtc[CORE_MACHINE_VADP_CRTC_HORIZONTAL_DISPLAYED];
    return columns > CORE_MACHINE_DISPLAY_MAX_COLUMNS ? CORE_MACHINE_DISPLAY_MAX_COLUMNS :
        columns;
}

static lib_u16 core_machine_vadp_text_rows(const t_vadp *adapter)
{
    lib_u16 rows;

    if (!core_machine_vadp_cga_logical_raster_active(adapter)) return adapter->data.rows;
    rows = adapter->data.crtc[CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAYED];
    return rows > CORE_MACHINE_DISPLAY_MAX_ROWS ? CORE_MACHINE_DISPLAY_MAX_ROWS : rows;
}

static lib_i32 core_machine_vadp_supported_crtc_index(const t_vadp *adapter,
    lib_u8 index)
{
    if (adapter == LIB_NULL || index >= CORE_MACHINE_VADP_CRTC_REGISTER_COUNT) return LIB_FALSE;
    if (!adapter->data.ega_controller_configured) return index <= 0x11u;
    return index <= CORE_MACHINE_VADP_CRTC_EGA_LAST;
}

static lib_i32 core_machine_vadp_crtc_index_readable(const t_vadp *adapter,
    lib_u8 index)
{
    if (!core_machine_vadp_supported_crtc_index(adapter, index)) return LIB_FALSE;
    if (!adapter->data.ega_controller_configured) {
        return index >= CORE_MACHINE_VADP_CRTC_CURSOR_HIGH;
    }
    return index >= CORE_MACHINE_VADP_CRTC_START_HIGH &&
        index <= CORE_MACHINE_VADP_CRTC_CURSOR_LOW;
}

static lib_i32 core_machine_vadp_crtc_index_writable(const t_vadp *adapter,
    lib_u8 index)
{
    if (!core_machine_vadp_supported_crtc_index(adapter, index)) return LIB_FALSE;
    return adapter->data.ega_controller_configured ||
        index <= CORE_MACHINE_VADP_CRTC_CURSOR_LOW;
}

static lib_u8 core_machine_vadp_crtc_mask(const t_vadp *adapter,
    lib_u8 index)
{
    if (adapter != LIB_NULL && adapter->data.ega_controller_configured) {
        switch (index) {
        case 0x03u: return 0x7fu;
        case 0x07u: return 0x3fu;
        case CORE_MACHINE_VADP_CRT_INTERLACE_SKEW:
        case CORE_MACHINE_VADP_CRTC_MAXIMUM_RASTER_ADDRESS:
        case CORE_MACHINE_VADP_CRTC_CURSOR_TOP:
        case CORE_MACHINE_VADP_CRTC_UNDERLINE_LOCATION:
        case CORE_MACHINE_VADP_CRTC_END_VERTICAL_BLANK:
            return 0x1fu;
        case CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM: return 0x7fu;
        case CORE_MACHINE_VADP_CRTC_VERTICAL_RETRACE_END: return 0x3fu;
        default: return 0xffu;
        }
    }
    switch (index) {
    case CORE_MACHINE_VADP_CRTC_VERTICAL_TOTAL:
    case CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAYED:
    case CORE_MACHINE_VADP_CRTC_VERTICAL_SYNC_POSITION:
        return 0x7fu;
    case CORE_MACHINE_VADP_CRTC_VERTICAL_TOTAL_ADJUST:
    case CORE_MACHINE_VADP_CRTC_MAXIMUM_RASTER_ADDRESS:
        return 0x1fu;
    case CORE_MACHINE_VADP_CRTC_CURSOR_TOP:
        return 0x3fu;
    case CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM:
        return 0x1fu;
    case CORE_MACHINE_VADP_CRTC_START_HIGH:
    case CORE_MACHINE_VADP_CRTC_CURSOR_HIGH:
        return 0x3fu;
    default:
        return 0xffu;
    }
}

static lib_u16 core_machine_vadp_crtc_word(const t_vadp *adapter,
    lib_u8 high_index)
{
    lib_u8 low_index = (lib_u8)(high_index + 1u);

    if (adapter == LIB_NULL || !core_machine_vadp_supported_crtc_index(adapter,
            high_index) || !core_machine_vadp_supported_crtc_index(adapter, low_index)) {
        return 0u;
    }
    return (lib_u16)(((lib_u16)adapter->data.crtc[high_index] << 8) |
        adapter->data.crtc[low_index]);
}

static lib_u16 core_machine_vadp_ega_vertical_displayed(
    const t_vadp *adapter)
{
    return adapter == LIB_NULL ? 0u : (lib_u16)(
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAY_END] +
        ((adapter->data.crtc[CORE_MACHINE_VADP_CRTC_OVERFLOW] & 0x02u) << 7u) + 1u);
}

static lib_i32 core_machine_vadp_ega_display_kind(const t_vadp *adapter,
    core_machine_display_kind *out_kind)
{
    lib_u16 horizontal;
    lib_u16 vertical;

    if (adapter == LIB_NULL || out_kind == LIB_NULL) return LIB_FALSE;
    horizontal = (lib_u16)adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_HORIZONTAL_DISPLAYED] + 1u;
    vertical = core_machine_vadp_ega_vertical_displayed(adapter);
    if (horizontal == 40u && vertical == 200u &&
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_OFFSET] ==
        CORE_MACHINE_VADP_EGA_320X200_CRTC_OFFSET) {
        *out_kind = CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16;
        return LIB_TRUE;
    }
    if (horizontal == 80u && vertical == 200u &&
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_OFFSET] ==
        CORE_MACHINE_VADP_EGA_640X200_CRTC_OFFSET) {
        *out_kind = CORE_MACHINE_DISPLAY_KIND_EGA_640X200X16;
        return LIB_TRUE;
    }
    if (horizontal == 80u && vertical == 350u &&
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_OFFSET] ==
        CORE_MACHINE_VADP_EGA_640X350_CRTC_OFFSET) {
        *out_kind = CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16;
        return LIB_TRUE;
    }
    return LIB_FALSE;
}

static void core_machine_vadp_mark_dirty(t_vadp *adapter)
{
    if (adapter != LIB_NULL) ++adapter->data.dirty_generation;
}

static lib_i32 core_machine_vadp_sequencer_index_supported(lib_u8 index)
{
    return index == 0u || index == 1u || index == 2u || index == 3u || index == 4u;
}

static lib_u8 core_machine_vadp_sequencer_mask(lib_u8 index)
{
    switch (index) {
    case 0u: return 0x03u;
    case 1u: return 0x3du;
    case 2u: return 0x0fu;
    case 3u: return 0x3fu;
    case 4u: return 0x0eu;
    default: return 0u;
    }
}

static lib_i32 core_machine_vadp_graphics_index_supported(lib_u8 index)
{
    return index < CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT;
}

static lib_u8 core_machine_vadp_graphics_mask(lib_u8 index)
{
    static const lib_u8 masks[CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT] = {
        0x0fu, 0x0fu, 0x0fu, 0x1fu, 0x07u, 0x7fu, 0x0fu, 0x0fu, 0xffu
    };

    return core_machine_vadp_graphics_index_supported(index) ? masks[index] : 0u;
}

static lib_i32 core_machine_vadp_attribute_index_supported(lib_u8 index)
{
    return index < 20u;
}

static lib_u8 core_machine_vadp_attribute_mask(lib_u8 index)
{
    if (index < 16u) return 0x3fu;
    switch (index) {
    case 16u: return 0x0fu;
    case 17u: return 0x3fu;
    case 18u: return 0x0fu;
    case 19u: return 0x0fu;
    case 20u: return 0x0fu;
    default: return 0u;
    }
}

static void core_machine_vadp_active_ega_aperture(const t_vadp *adapter,
    lib_u32 *out_base, lib_u32 *out_bytes)
{
    lib_u8 map_select;

    if (adapter == LIB_NULL || out_base == LIB_NULL || out_bytes == LIB_NULL) {
        return;
    }
    *out_base = adapter->data.ega_sequencer.aperture_base;
    *out_bytes = adapter->data.ega_sequencer.aperture_bytes;
    if (!adapter->data.ega_controller_configured) return;
    map_select = (adapter->data.graphics[6] >> 2) & 0x03u;
    switch (map_select) {
    case 0u:
        *out_base = 0x000a0000u;
        *out_bytes = 0x00020000u;
        break;
    case 1u:
        *out_base = 0x000a0000u;
        *out_bytes = 0x00010000u;
        break;
    case 2u:
        *out_base = 0x000b0000u;
        *out_bytes = 0x00008000u;
        break;
    default:
        *out_base = 0x000b8000u;
        *out_bytes = 0x00008000u;
        break;
    }
}

static void core_machine_vadp_reset_sequencer(t_vadp *adapter)
{
    if (adapter == LIB_NULL || !adapter->data.ega_sequencer_configured) return;
    adapter->data.sequencer_index = 0u;
    adapter->data.sequencer[0] = adapter->data.ega_sequencer.reset & 0x03u;
    adapter->data.sequencer[1] = adapter->data.ega_sequencer.clocking_mode & 0x3du;
    adapter->data.sequencer[2] = adapter->data.ega_sequencer.map_mask & 0x0fu;
    adapter->data.sequencer[4] = adapter->data.ega_sequencer.memory_mode & 0x0eu;
}

static void core_machine_vadp_reset_ega_controllers(t_vadp *adapter)
{
    if (adapter == LIB_NULL || !adapter->data.ega_controller_configured) return;
    adapter->data.graphics_index = 0u;
    lib_memory_copy(adapter->data.graphics, adapter->data.ega_controller.graphics,
        sizeof(adapter->data.graphics));
    adapter->data.attribute_index = 0u;
    lib_memory_copy(adapter->data.attribute, adapter->data.ega_controller.attribute,
        sizeof(adapter->data.attribute));
    adapter->data.attribute_data_phase = LIB_FALSE;
    adapter->data.attribute_display_enabled = LIB_TRUE;
}

static void core_machine_vadp_normalize_ega_controllers(
    core_machine_vadp_ega_controller_config *config)
{
    lib_u8 index;

    if (config == LIB_NULL) return;
    for (index = 0u; index < CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT; ++index) {
        config->graphics[index] &= core_machine_vadp_graphics_mask(index);
    }
    for (index = 0u; index < CORE_MACHINE_VADP_ATTRIBUTE_REGISTER_COUNT; ++index) {
        config->attribute[index] &= core_machine_vadp_attribute_mask(index);
    }
}

static void core_machine_vadp_ega_write_observer(void *owner,
    lib_u32 physical, lib_uptr bytes)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_u64 write_end;
    lib_u64 aperture_end;

    /* This observer owns presentation freshness for both planar EGA and the
     * non-planar memory-backed EGA configuration.  CPU mapping still belongs
     * to the planar provider when present; requiring that provider here made
     * the non-planar path silently miss real writes. */
    if (adapter == LIB_NULL || !adapter->data.ega_sequencer_configured ||
        bytes == 0u || !core_machine_vadp_ega_aperture_contains(adapter,
            physical, bytes)) return;
    if (core_machine_vadp_compaq_b000_compatibility_contains(adapter, physical, bytes)) {
        core_machine_vadp_mark_dirty(adapter);
        return;
    }
    write_end = (lib_u64)physical + bytes;
    {
        lib_u32 aperture_base;
        lib_u32 aperture_bytes;

        core_machine_vadp_active_ega_aperture(adapter, &aperture_base,
            &aperture_bytes);
        aperture_end = (lib_u64)aperture_base + aperture_bytes;
        if ((lib_u64)physical < aperture_end &&
            (lib_u64)aperture_base < write_end) {
            core_machine_vadp_mark_dirty(adapter);
        }
    }
}

static lib_u32 core_machine_vadp_raster_period(
    const core_machine_vadp_text_timing *timing)
{
    return timing->active_display_ticks + timing->horizontal_blank_ticks +
        timing->vertical_retrace_ticks;
}

static lib_i32 core_machine_vadp_valid_text_timing(
    const core_machine_vadp_text_timing *timing)
{
    lib_u32 period;

    if (timing == LIB_NULL || timing->active_display_ticks == 0u ||
        timing->vertical_retrace_ticks == 0u) {
        return LIB_FALSE;
    }
    period = core_machine_vadp_raster_period(timing);
    return period >= timing->active_display_ticks &&
        period >= timing->horizontal_blank_ticks &&
        period >= timing->vertical_retrace_ticks;
}

static lib_u8 core_machine_vadp_status(const t_vadp *adapter)
{
    lib_u32 vertical_end;
    lib_u32 display_end;
    lib_u8 status = 0u;

    if (adapter == LIB_NULL) return 0u;
    if (core_machine_vadp_cga_logical_raster_active(adapter)) {
        lib_u32 horizontal_total = (lib_u32)adapter->data.crtc[
            CORE_MACHINE_VADP_CRTC_HORIZONTAL_TOTAL] + 1u;
        lib_u32 period = core_machine_vadp_cga_logical_raster_period(adapter);
        lib_u32 scanline;
        lib_u32 character;
        lib_u32 display_scanlines = (lib_u32)adapter->data.crtc[
            CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAYED] *
            core_machine_vadp_cga_scanlines_per_row(adapter);
        lib_u32 vertical_sync_start = (lib_u32)adapter->data.crtc[
            CORE_MACHINE_VADP_CRTC_VERTICAL_SYNC_POSITION] *
            core_machine_vadp_cga_scanlines_per_row(adapter);

        if (!adapter->data.cga_logical_raster_started || period == 0u) {
            return CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE;
        }
        scanline = (adapter->data.raster_phase % period) / horizontal_total;
        character = (adapter->data.raster_phase % period) % horizontal_total;
        if (scanline >= vertical_sync_start && scanline - vertical_sync_start < 16u) {
            status |= CORE_MACHINE_VADP_STATUS_VERTICAL_RETRACE;
        }
        if (scanline >= display_scanlines || character >= adapter->data.crtc[
                CORE_MACHINE_VADP_CRTC_HORIZONTAL_DISPLAYED]) {
            status |= CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE;
        }
        return status;
    }
    vertical_end = adapter->data.text_timing.vertical_retrace_ticks;
    display_end = vertical_end + adapter->data.text_timing.active_display_ticks;
    if (adapter->data.raster_phase < vertical_end) {
        status = CORE_MACHINE_VADP_STATUS_VERTICAL_RETRACE;
    }
    /* CGA status bit 0 reports that buffer access can proceed without
     * display interference. EGA retains its existing display-enable view. */
    if (adapter->data.ega_controller_configured) {
        if (adapter->data.raster_phase >= vertical_end &&
            adapter->data.raster_phase < display_end) {
            status |= CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE;
        }
    } else if (adapter->data.raster_phase < vertical_end ||
        adapter->data.raster_phase >= display_end) {
        status |= CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE;
    }
    return status;
}

static lib_i32 core_machine_vadp_compaq_io_route_active(const t_vadp *adapter,
    lib_u16 port_id, lib_u16 monochrome_port,
    lib_u16 color_port)
{
    if (adapter == LIB_NULL) return LIB_FALSE;
    if (adapter->data.ega_personality ==
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        return port_id == (adapter->data.compaq_color_io_base ? color_port :
            monochrome_port);
    }
    if (!adapter->data.ega_external_configured) {
        return port_id == color_port;
    }
    return port_id == ((adapter->data.ega_miscellaneous_output & 0x01u) != 0u ?
        color_port :
        monochrome_port);
}

static void core_machine_vadp_write_crtc_index(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (port != LIB_NULL && adapter != LIB_NULL &&
        core_machine_vadp_compaq_io_route_active(adapter, port_id,
        CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX,
        CORE_MACHINE_VADP_PORT_CRTC_INDEX)) {
        adapter->data.crtc_index = port->data.ioByte & 0x1fu;
    }
}

static void core_machine_vadp_read_crtc_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (port == LIB_NULL || adapter == LIB_NULL) return;
    if (!core_machine_vadp_compaq_io_route_active(adapter, port_id,
        CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA,
        CORE_MACHINE_VADP_PORT_CRTC_DATA)) {
        port->data.ioByte = 0u;
        return;
    }
    port->data.ioByte = core_machine_vadp_crtc_index_readable(adapter,
        adapter->data.crtc_index) ?
        adapter->data.crtc[adapter->data.crtc_index] : 0u;
}

static void core_machine_vadp_write_crtc_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (port == LIB_NULL || adapter == LIB_NULL ||
        !core_machine_vadp_compaq_io_route_active(adapter, port_id,
        CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA,
        CORE_MACHINE_VADP_PORT_CRTC_DATA) ||
        !core_machine_vadp_crtc_index_writable(adapter, adapter->data.crtc_index)) {
        return;
    }
    {
        lib_u8 value = port->data.ioByte &
            core_machine_vadp_crtc_mask(adapter, adapter->data.crtc_index);

        if (adapter->data.crtc[adapter->data.crtc_index] == value) return;
        adapter->data.crtc[adapter->data.crtc_index] = value;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_read_mode(t_port *port, lib_u16 port_id,
    void *owner)
{
    (void)port_id;
    if (port != LIB_NULL && owner != LIB_NULL) {
        port->data.ioByte = ((t_vadp *)owner)->data.mode_control;
    }
}

static void core_machine_vadp_write_mode(t_port *port, lib_u16 port_id,
    void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_u8 value;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL) return;
    value = port->data.ioByte & 0x3fu;
    if (adapter->data.mode_control != value) {
        adapter->data.mode_control = value;
        adapter->data.columns = (value & 0x01u) != 0u ? 80u : 40u;
        adapter->data.color_enabled = (value & 0x04u) != 0u;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_read_color(t_port *port,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (port != LIB_NULL && owner != LIB_NULL) {
        port->data.ioByte = ((t_vadp *)owner)->data.color_select;
    }
}

static void core_machine_vadp_write_color(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_u8 value;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL) return;
    value = port->data.ioByte & 0x3fu;

    if (adapter->data.color_select != value) {
        adapter->data.color_select = value;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_write_cga_lightpen(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port;
    if (adapter == LIB_NULL) return;
    if (adapter->data.ega_personality ==
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        if (port_id == (adapter->data.compaq_color_io_base ?
            CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET :
            CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_RESET)) {
            adapter->data.compaq_lightpen_latched = LIB_FALSE;
        } else if (port_id == (adapter->data.compaq_color_io_base ?
            CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET :
            CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_SET)) {
            adapter->data.compaq_lightpen_latched = LIB_TRUE;
        }
    } else if (!adapter->data.ega_controller_configured) {
        adapter->data.cga_lightpen_latched =
            port_id == CORE_MACHINE_VADP_PORT_CGA_LIGHTPEN_PRESET;
    }
}

static void core_machine_vadp_read_status(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (port == LIB_NULL || adapter == LIB_NULL) return;
    if (!core_machine_vadp_compaq_io_route_active(adapter, port_id,
        CORE_MACHINE_VADP_PORT_MONO_STATUS, CORE_MACHINE_VADP_PORT_STATUS)) {
        port->data.ioByte = 0u;
        return;
    }
    port->data.ioByte = core_machine_vadp_status(adapter);
    if (!adapter->data.ega_controller_configured &&
        adapter->data.cga_lightpen_latched) {
        port->data.ioByte |= CORE_MACHINE_VADP_STATUS_LIGHTPEN_TRIGGER;
    }
    if (adapter->data.ega_controller_configured) {
        if (!adapter->data.ega_status_diagnostic_high) {
            port->data.ioByte |= 0x30u;
        }
        adapter->data.ega_status_diagnostic_high =
            !adapter->data.ega_status_diagnostic_high;
    }
    if (adapter->data.ega_personality ==
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        if (adapter->data.compaq_lightpen_latched) {
            port->data.ioByte |= CORE_MACHINE_VADP_STATUS_LIGHTPEN_TRIGGER;
        }
        if (adapter->data.cecg.lightpen_switch_open) {
            port->data.ioByte |= CORE_MACHINE_VADP_STATUS_LIGHTPEN_SWITCH_OPEN;
        }
    }
    if (adapter->data.ega_controller_configured) {
        adapter->data.attribute_data_phase = LIB_FALSE;
    }
}

static void core_machine_vadp_read_compaq_control_mode(t_port *port,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (port != LIB_NULL && owner != LIB_NULL) {
        port->data.ioByte = ((const t_vadp *)owner)->data.compaq_control_mode;
    }
}

static void core_machine_vadp_write_compaq_control_mode(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_u8 value;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL) return;
    value = port->data.ioByte;
    if (adapter->data.compaq_control_mode != value) {
        adapter->data.compaq_control_mode = value;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_write_compaq_miscellaneous_output(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL) return;
    /* Compaq EGA miscellaneous-output bit 1 disables the CPU aperture.  The
     * clear state leaves the window decoded by the VADP provider. */
    adapter->data.compaq_cpu_video_memory_disabled =
        (port->data.ioByte & 0x02u) != 0u;
    adapter->data.compaq_color_io_base = (port->data.ioByte & 0x01u) != 0u;
    adapter->data.compaq_clock_switch_select = (port->data.ioByte >> 2u) & 0x03u;
    if (adapter->data.compaq_odd_even_high_page !=
        ((port->data.ioByte & 0x20u) != 0u)) {
        adapter->data.compaq_odd_even_high_page = (port->data.ioByte & 0x20u) != 0u;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_read_compaq_input_status_0(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;
    lib_u8 selected_switch;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL) return;
    selected_switch = 4u - adapter->data.compaq_clock_switch_select;
    port->data.ioByte = (adapter->data.cecg.sw1_closed_mask &
        (1u << (selected_switch - 1u))) != 0u ? 0u : 0x10u;
    if (!adapter->data.cecg.special_features_present) {
        port->data.ioByte |= 0x60u;
    }
    if (!adapter->data.cecg.vertical_retrace_irq_enabled) {
        port->data.ioByte |= 0x80u;
    }
}

static void core_machine_vadp_write_compaq_feature_control(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (port == LIB_NULL || adapter == LIB_NULL ||
        !core_machine_vadp_compaq_io_route_active(adapter, port_id,
        CORE_MACHINE_VADP_PORT_MONO_STATUS,
        CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL)) return;
    adapter->data.compaq_feature_control = port->data.ioByte & 0x03u;
}

static void core_machine_vadp_read_ega_input_status_0(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL) return;
    port->data.ioByte = (core_machine_vadp_status(adapter) &
        CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE) != 0u ? 0x80u : 0u;
}

static void core_machine_vadp_write_ega_miscellaneous_output(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL) return;
    if (adapter->data.ega_miscellaneous_output != port->data.ioByte) {
        adapter->data.ega_miscellaneous_output = port->data.ioByte;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_write_ega_feature_control(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL && adapter != LIB_NULL) {
        adapter->data.ega_feature_control = port->data.ioByte & 0x03u;
    }
}

static void core_machine_vadp_read_compaq_environment(t_port *port,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (port != LIB_NULL && owner != LIB_NULL) {
        const t_vadp *adapter = (const t_vadp *)owner;

        port->data.ioByte = (adapter->data.cecg.environment & 0xfcu) |
            adapter->data.compaq_feature_control;
    }
}

static void core_machine_vadp_read_compaq_display_type(t_port *port,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (port != LIB_NULL && owner != LIB_NULL) {
        port->data.ioByte = ((const t_vadp *)owner)->data.cecg.display_type;
    }
}

static void core_machine_vadp_read_compaq_initial_mode(t_port *port,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (port != LIB_NULL && owner != LIB_NULL) {
        port->data.ioByte = ((const t_vadp *)owner)->data.cecg.initial_mode;
    }
}

static void core_machine_vadp_read_graphics_index(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL && adapter != LIB_NULL) {
        port->data.ioByte = adapter->data.ega_controller_configured ?
            adapter->data.graphics_index : 0xffu;
    }
}

static void core_machine_vadp_write_graphics_index(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL && adapter != LIB_NULL &&
        adapter->data.ega_controller_configured) {
        adapter->data.graphics_index = port->data.ioByte;
    }
}

static void core_machine_vadp_read_graphics_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL) {
        port->data.ioByte = adapter != LIB_NULL && adapter->data.ega_controller_configured &&
            core_machine_vadp_graphics_index_supported(adapter->data.graphics_index) ?
            adapter->data.graphics[adapter->data.graphics_index] : 0u;
    }
}

static void core_machine_vadp_write_graphics_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_u8 index;
    lib_u8 value;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL ||
        !adapter->data.ega_controller_configured) return;
    index = adapter->data.graphics_index;
    if (!core_machine_vadp_graphics_index_supported(index)) return;
    value = port->data.ioByte & core_machine_vadp_graphics_mask(index);
    if (adapter->data.graphics[index] != value) {
        adapter->data.graphics[index] = value;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_read_attribute_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL) {
        port->data.ioByte = adapter != LIB_NULL && adapter->data.ega_controller_configured &&
            core_machine_vadp_attribute_index_supported(adapter->data.attribute_index) ?
            adapter->data.attribute[adapter->data.attribute_index] : 0u;
    }
}

static void core_machine_vadp_write_attribute(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_u8 value;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL ||
        !adapter->data.ega_controller_configured) return;
    value = port->data.ioByte;
    if (!adapter->data.attribute_data_phase) {
        adapter->data.attribute_index = value & 0x1fu;
        if (adapter->data.attribute_display_enabled != ((value & 0x20u) != 0u)) {
            adapter->data.attribute_display_enabled = (value & 0x20u) != 0u;
            core_machine_vadp_mark_dirty(adapter);
        }
        adapter->data.attribute_data_phase = LIB_TRUE;
        return;
    }
    if (core_machine_vadp_attribute_index_supported(adapter->data.attribute_index)) {
        lib_u8 index = adapter->data.attribute_index;
        lib_u8 masked = value & core_machine_vadp_attribute_mask(index);

        if (adapter->data.attribute[index] != masked) {
            adapter->data.attribute[index] = masked;
            core_machine_vadp_mark_dirty(adapter);
        }
    }
    adapter->data.attribute_data_phase = LIB_FALSE;
}

static void core_machine_vadp_read_sequencer_index(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL && adapter != LIB_NULL) {
        port->data.ioByte = adapter->data.ega_sequencer_configured ?
            adapter->data.sequencer_index : 0xffu;
    }
}

static void core_machine_vadp_write_sequencer_index(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL && adapter != LIB_NULL &&
        adapter->data.ega_sequencer_configured) {
        adapter->data.sequencer_index = port->data.ioByte;
    }
}

static void core_machine_vadp_read_sequencer_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL) return;
    port->data.ioByte = adapter->data.ega_sequencer_configured &&
        core_machine_vadp_sequencer_index_supported(adapter->data.sequencer_index) ?
        adapter->data.sequencer[adapter->data.sequencer_index] : 0xffu;
}

static void core_machine_vadp_write_sequencer_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_u8 index;
    lib_u8 value;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL ||
        !adapter->data.ega_sequencer_configured) return;
    index = adapter->data.sequencer_index;
    if (!core_machine_vadp_sequencer_index_supported(index)) return;
    value = port->data.ioByte & core_machine_vadp_sequencer_mask(index);
    if (adapter->data.sequencer[index] != value) {
        adapter->data.sequencer[index] = value;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_read_vga_dac_mask(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL) {
        port->data.ioByte = adapter != LIB_NULL && adapter->data.vga_configured ?
            adapter->data.vga_dac_mask : 0u;
    }
}

static void core_machine_vadp_write_vga_dac_mask(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    if (adapter->data.vga_dac_mask != port->data.ioByte) {
        adapter->data.vga_dac_mask = port->data.ioByte;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static void core_machine_vadp_read_vga_dac_read_index(t_port *port,
    lib_u16 port_id, void *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (void)port_id;
    if (port != LIB_NULL) {
        port->data.ioByte = adapter != LIB_NULL && adapter->data.vga_configured ?
            adapter->data.vga_dac_read_index : 0u;
    }
}

static void core_machine_vadp_write_vga_dac_read_index(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    adapter->data.vga_dac_read_index = port->data.ioByte;
    adapter->data.vga_dac_read_component = 0u;
}

static void core_machine_vadp_write_vga_dac_write_index(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    adapter->data.vga_dac_write_index = port->data.ioByte;
    adapter->data.vga_dac_write_component = 0u;
}

static void core_machine_vadp_read_vga_dac_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    port->data.ioByte = adapter->data.vga_dac[adapter->data.vga_dac_read_index]
        [adapter->data.vga_dac_read_component];
    if (++adapter->data.vga_dac_read_component == 3u) {
        adapter->data.vga_dac_read_component = 0u;
        ++adapter->data.vga_dac_read_index;
    }
}

static void core_machine_vadp_write_vga_dac_data(t_port *port,
    lib_u16 port_id, void *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    lib_u8 value;

    (void)port_id;
    if (port == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    value = port->data.ioByte & 0x3fu;
    if (adapter->data.vga_dac[adapter->data.vga_dac_write_index]
        [adapter->data.vga_dac_write_component] != value) {
        adapter->data.vga_dac[adapter->data.vga_dac_write_index]
            [adapter->data.vga_dac_write_component] = value;
        core_machine_vadp_mark_dirty(adapter);
    }
    if (++adapter->data.vga_dac_write_component == 3u) {
        adapter->data.vga_dac_write_component = 0u;
        ++adapter->data.vga_dac_write_index;
    }
}

static void core_machine_vadp_register_cga_ports(t_vadp *adapter, t_port *port)
{
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX,
        core_machine_vadp_write_crtc_index, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_CRTC_DATA,
        core_machine_vadp_read_crtc_data, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_CRTC_DATA,
        core_machine_vadp_write_crtc_data, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_MODE,
        core_machine_vadp_write_mode, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_COLOR,
        core_machine_vadp_write_color, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_STATUS,
        core_machine_vadp_read_status, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_CGA_LIGHTPEN_CLEAR,
        core_machine_vadp_write_cga_lightpen, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_CGA_LIGHTPEN_PRESET,
        core_machine_vadp_write_cga_lightpen, adapter);
}

void core_machine_vadp_initialize(t_vadp *adapter, t_port *port)
{
    if (adapter == LIB_NULL || port == LIB_NULL) return;
    lib_memory_set(adapter, 0u, sizeof(*adapter));
    core_machine_vadp_register_cga_ports(adapter, port);
    core_machine_vadp_reset(adapter);
}

void core_machine_vadp_set_allocate_zero(t_vadp *adapter,
    core_machine_vadp_allocate_zero callback, void *context)
{
    if (adapter == LIB_NULL) return;
    adapter->data.allocate_zero = callback;
    adapter->data.allocate_context = context;
}

void core_machine_vadp_configure_ega_ports(t_vadp *adapter, t_port *port)
{
    if (adapter == LIB_NULL || port == LIB_NULL) return;
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_MODE,
        core_machine_vadp_read_mode, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_COLOR,
        core_machine_vadp_read_color, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_ATTRIBUTE,
        core_machine_vadp_write_attribute, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ,
        core_machine_vadp_read_attribute_data, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX,
        core_machine_vadp_read_graphics_index, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX,
        core_machine_vadp_write_graphics_index, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA,
        core_machine_vadp_read_graphics_data, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA,
        core_machine_vadp_write_graphics_data, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX,
        core_machine_vadp_read_sequencer_index, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX,
        core_machine_vadp_write_sequencer_index, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_SEQUENCER_DATA,
        core_machine_vadp_read_sequencer_data, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_SEQUENCER_DATA,
        core_machine_vadp_write_sequencer_data, adapter);
}

lib_status core_machine_vadp_configure_ega_personality(t_vadp *adapter,
    t_port *port, core_machine_vadp_ega_personality personality)
{
    if (adapter == LIB_NULL || port == LIB_NULL ||
        adapter->data.ega_personality != CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC ||
        (personality != CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC &&
        personality != CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (personality == CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC) {
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0,
            core_machine_vadp_read_ega_input_status_0, adapter);
        core_machine_port_add_write(port,
            CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT,
            core_machine_vadp_write_ega_miscellaneous_output, adapter);
        core_machine_port_add_write(port,
            CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_MONO,
            core_machine_vadp_write_ega_feature_control, adapter);
        core_machine_port_add_write(port,
            CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR,
            core_machine_vadp_write_ega_feature_control, adapter);
        core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX,
            core_machine_vadp_write_crtc_index, adapter);
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA,
            core_machine_vadp_read_crtc_data, adapter);
        core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA,
            core_machine_vadp_write_crtc_data, adapter);
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_MONO_STATUS,
            core_machine_vadp_read_status, adapter);
        if (core_machine_port_registration_status(port) != LIB_STATUS_OK) {
            return core_machine_port_registration_status(port);
        }
        adapter->data.ega_external_configured = LIB_TRUE;
    } else {
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
            core_machine_vadp_read_compaq_input_status_0, adapter);
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE,
            core_machine_vadp_read_compaq_control_mode, adapter);
        core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE,
            core_machine_vadp_write_compaq_control_mode, adapter);
        core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
            core_machine_vadp_write_compaq_miscellaneous_output, adapter);
        core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL,
            core_machine_vadp_write_compaq_feature_control, adapter);
        core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX,
            core_machine_vadp_write_crtc_index, adapter);
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA,
            core_machine_vadp_read_crtc_data, adapter);
        core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA,
            core_machine_vadp_write_crtc_data, adapter);
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_MONO_STATUS,
            core_machine_vadp_read_status, adapter);
        core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_MONO_STATUS,
            core_machine_vadp_write_compaq_feature_control, adapter);
        core_machine_port_add_write(port,
            CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_RESET,
            core_machine_vadp_write_cga_lightpen, adapter);
        core_machine_port_add_write(port,
            CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_SET,
            core_machine_vadp_write_cga_lightpen, adapter);
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT,
            core_machine_vadp_read_compaq_environment, adapter);
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE,
            core_machine_vadp_read_compaq_display_type, adapter);
        core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE,
            core_machine_vadp_read_compaq_initial_mode, adapter);
        if (core_machine_port_registration_status(port) != LIB_STATUS_OK) {
            return core_machine_port_registration_status(port);
        }
    }
    adapter->data.ega_personality = personality;
    if (personality == CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        adapter->data.cecg = (core_machine_vadp_cecg_config) {
            0x40u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
            0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE };
        adapter->data.compaq_control_mode = adapter->data.cecg.control_mode;
        adapter->data.compaq_cpu_video_memory_disabled =
            adapter->data.cecg.cpu_video_memory_disabled;
        adapter->data.compaq_color_io_base = adapter->data.cecg.color_io_base;
        adapter->data.compaq_clock_switch_select =
            adapter->data.cecg.clock_switch_select;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_vadp_configure_vga(t_vadp *adapter, t_port *port)
{
    if (adapter == LIB_NULL || port == LIB_NULL || adapter->data.vga_configured ||
        adapter->data.ega_personality != CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC ||
        !adapter->data.ega_sequencer_configured ||
        !adapter->data.ega_controller_configured) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK,
        core_machine_vadp_read_vga_dac_mask, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK,
        core_machine_vadp_write_vga_dac_mask, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_READ_INDEX,
        core_machine_vadp_read_vga_dac_read_index, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_READ_INDEX,
        core_machine_vadp_write_vga_dac_read_index, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_WRITE_INDEX,
        core_machine_vadp_write_vga_dac_write_index, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA,
        core_machine_vadp_read_vga_dac_data, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA,
        core_machine_vadp_write_vga_dac_data, adapter);
    if (core_machine_port_registration_status(port) != LIB_STATUS_OK) {
        return core_machine_port_registration_status(port);
    }
    adapter->data.vga_configured = LIB_TRUE;
    adapter->data.vga_dac_mask = 0xffu;
    return LIB_STATUS_OK;
}

lib_i32 core_machine_vadp_cecg_config_is_valid(
    const core_machine_vadp_cecg_config *config)
{
    return config != LIB_NULL && (config->control_mode & 0xe0u) == 0x40u &&
        (config->display_type & 0x44u) == 0u && config->initial_mode == 0x01u &&
        (config->sw1_closed_mask & 0xf0u) == 0u && config->clock_switch_select <= 3u;
}

lib_status core_machine_vadp_configure_cecg(t_vadp *adapter,
    const core_machine_vadp_cecg_config *config)
{
    if (adapter == LIB_NULL ||
        adapter->data.ega_personality !=
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR ||
        !core_machine_vadp_cecg_config_is_valid(config)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    adapter->data.cecg = *config;
    adapter->data.compaq_control_mode = config->control_mode;
    adapter->data.compaq_cpu_video_memory_disabled =
        config->cpu_video_memory_disabled;
    adapter->data.compaq_color_io_base = config->color_io_base;
    adapter->data.compaq_clock_switch_select = config->clock_switch_select;
    adapter->data.compaq_odd_even_high_page = config->odd_even_high_page;
    adapter->data.compaq_feature_control = config->environment & 0x03u;
    return LIB_STATUS_OK;
}

void core_machine_vadp_reset(t_vadp *adapter)
{
    core_machine_vadp_text_timing timing;
    core_machine_vadp_ega_personality ega_personality;
    core_machine_vadp_cecg_config cecg;
    core_machine_vadp_ega_sequencer_config ega_sequencer;
    core_machine_vadp_ega_controller_config ega_controller;
    lib_u8 crtc[CORE_MACHINE_VADP_CRTC_REGISTER_COUNT];
    lib_u8 crtc_initialized;
    lib_u8 ega_sequencer_configured;
    lib_u8 ega_controller_configured;
    lib_u8 ega_external_configured;
    lib_u8 ega_planar_enabled;
    lib_u8 vga_configured;
    lib_u8 cga_memory_configured;
    lib_uptr ega_planar_vram;
    core_machine_vadp_text_glyph_config text_glyphs;

    if (adapter == LIB_NULL) return;
    timing = adapter->data.text_timing;
    if (!core_machine_vadp_valid_text_timing(&timing)) {
        timing.active_display_ticks = CORE_MACHINE_VADP_DEFAULT_ACTIVE_DISPLAY_TICKS;
        timing.horizontal_blank_ticks = CORE_MACHINE_VADP_DEFAULT_HORIZONTAL_BLANK_TICKS;
        timing.vertical_retrace_ticks = CORE_MACHINE_VADP_DEFAULT_VERTICAL_RETRACE_TICKS;
    }
    ega_personality = adapter->data.ega_personality;
    cecg = adapter->data.cecg;
    ega_sequencer = adapter->data.ega_sequencer;
    ega_sequencer_configured = adapter->data.ega_sequencer_configured;
    ega_external_configured = adapter->data.ega_external_configured;
    ega_controller = adapter->data.ega_controller;
    ega_controller_configured = adapter->data.ega_controller_configured;
    ega_planar_enabled = adapter->data.ega_planar_enabled;
    vga_configured = adapter->data.vga_configured;
    cga_memory_configured = adapter->data.cga_memory_configured;
    ega_planar_vram = adapter->data.ega_planar_vram;
    crtc_initialized = adapter->data.crtc_initialized;
    text_glyphs = adapter->data.text_glyphs;
    lib_memory_copy(crtc, adapter->data.crtc, sizeof(crtc));
    if (ega_planar_vram != 0u) {
        lib_memory_set((void *)ega_planar_vram, 0,
            CORE_MACHINE_VADP_EGA_PLANES * CORE_MACHINE_VADP_EGA_PLANE_BYTES);
    }
    lib_memory_set(&adapter->data, 0u, sizeof(adapter->data));
    adapter->data.mode_control = 0x05u;
    adapter->data.text_timing = timing;
    adapter->data.text_glyphs = text_glyphs;
    adapter->data.raster_phase = timing.vertical_retrace_ticks;
    adapter->data.columns = 80u;
    adapter->data.rows = 25u;
    adapter->data.color_enabled = LIB_TRUE;
    adapter->data.crtc_initialized = LIB_TRUE;
    if (!ega_controller_configured && crtc_initialized) {
        lib_memory_copy(adapter->data.crtc, crtc, sizeof(adapter->data.crtc));
    } else {
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_TOP] = 6u;
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM] = 7u;
    }
    adapter->data.ega_personality = ega_personality;
    adapter->data.cecg = cecg;
    adapter->data.compaq_control_mode = cecg.control_mode;
    adapter->data.compaq_feature_control = cecg.environment & 0x03u;
    adapter->data.compaq_cpu_video_memory_disabled =
        cecg.cpu_video_memory_disabled;
    adapter->data.compaq_color_io_base = cecg.color_io_base;
    adapter->data.compaq_clock_switch_select = cecg.clock_switch_select;
    adapter->data.compaq_odd_even_high_page = cecg.odd_even_high_page;
    adapter->data.ega_sequencer = ega_sequencer;
    adapter->data.ega_sequencer_configured = ega_sequencer_configured;
    adapter->data.ega_external_configured = ega_external_configured;
    core_machine_vadp_reset_sequencer(adapter);
    adapter->data.ega_controller = ega_controller;
    adapter->data.ega_controller_configured = ega_controller_configured;
    core_machine_vadp_reset_ega_controllers(adapter);
    adapter->data.ega_planar_enabled = ega_planar_enabled;
    adapter->data.vga_configured = vga_configured;
    adapter->data.cga_memory_configured = cga_memory_configured;
    if (vga_configured) adapter->data.vga_dac_mask = 0xffu;
    adapter->data.ega_planar_vram = ega_planar_vram;
    if (core_machine_vadp_cga_logical_raster_active(adapter)) {
        adapter->data.raster_phase = 0u;
        adapter->data.cga_logical_raster_started = LIB_FALSE;
    }
    adapter->data.dirty_generation = 1u;
}

lib_status core_machine_vadp_configure_text_glyphs(t_vadp *adapter,
    const core_machine_vadp_text_glyph_config *config)
{
    if (adapter == LIB_NULL || config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    adapter->data.text_glyphs = *config;
    return LIB_STATUS_OK;
}

void core_machine_vadp_advance(t_vadp *adapter, t_ram *memory,
    lib_u64 elapsed_ticks)
{
    lib_u32 period;

    (void)memory;
    if (adapter == LIB_NULL) return;
    if (core_machine_vadp_cga_logical_raster_active(adapter)) {
        lib_u32 phase;

        period = core_machine_vadp_cga_logical_raster_period(adapter);
        if (period == 0u) return;
        phase = adapter->data.raster_phase;
        if (phase >= period) phase %= period;
        if (!adapter->data.cga_logical_raster_started && elapsed_ticks >=
            period - phase) {
            adapter->data.cga_logical_raster_started = LIB_TRUE;
        }
        if (elapsed_ticks >= period) elapsed_ticks %= period;
        if ((lib_u32)elapsed_ticks >= period - phase) {
            adapter->data.raster_phase = (lib_u32)elapsed_ticks -
                (period - phase);
        } else {
            adapter->data.raster_phase = phase + (lib_u32)elapsed_ticks;
        }
        return;
    }
    period = core_machine_vadp_raster_period(&adapter->data.text_timing);
    if (period == 0u) return;
    if (adapter->data.raster_phase >= period) adapter->data.raster_phase %= period;
    if (elapsed_ticks >= period) elapsed_ticks %= period;
    if ((lib_u32)elapsed_ticks >= period - adapter->data.raster_phase) {
        adapter->data.raster_phase = (lib_u32)elapsed_ticks -
            (period - adapter->data.raster_phase);
    } else {
        adapter->data.raster_phase += (lib_u32)elapsed_ticks;
    }
}

void core_machine_vadp_finalize(t_vadp *adapter)
{
    if (adapter == LIB_NULL) return;
    lib_release((void *)adapter->data.ega_planar_vram);
    adapter->data.ega_planar_vram = 0u;
}

lib_status core_machine_vadp_configure_text_timing(t_vadp *adapter,
    const core_machine_vadp_text_timing *timing)
{
    if (adapter == LIB_NULL || !core_machine_vadp_valid_text_timing(timing)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    adapter->data.text_timing = *timing;
    adapter->data.raster_phase = timing->vertical_retrace_ticks;
    return LIB_STATUS_OK;
}

lib_status core_machine_vadp_configure_cga_memory(t_vadp *adapter, t_ram *memory)
{
    lib_status status;

    if (adapter == LIB_NULL || memory == LIB_NULL || adapter->data.cga_memory_configured) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_register_device_provider(memory,
        CORE_MACHINE_VADP_VIDEO_BASE, CORE_MACHINE_VADP_VIDEO_BYTES,
        core_machine_vadp_cga_read, core_machine_vadp_cga_write,
        core_machine_vadp_cga_query, adapter);
    if (status == LIB_STATUS_OK) adapter->data.cga_memory_configured = LIB_TRUE;
    return status;
}

lib_status core_machine_vadp_configure_ega_sequencer(t_vadp *adapter,
    t_ram *memory, const core_machine_vadp_ega_sequencer_config *config)
{
    lib_status status;
    lib_uptr planar_vram = 0u;

    if (adapter == LIB_NULL || memory == LIB_NULL || config == LIB_NULL ||
        config->aperture_base != CORE_MACHINE_VADP_EGA_APERTURE_BASE ||
        config->aperture_bytes != CORE_MACHINE_VADP_EGA_APERTURE_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (adapter->data.ega_sequencer_configured) return LIB_STATUS_INVALID_STATE;
    if (config->planar_ega) {
        planar_vram = (lib_uptr)(adapter->data.allocate_zero == LIB_NULL ?
            lib_allocate_zero(1u, CORE_MACHINE_VADP_EGA_PLANES *
                CORE_MACHINE_VADP_EGA_PLANE_BYTES) :
            adapter->data.allocate_zero(adapter->data.allocate_context, 1u,
                CORE_MACHINE_VADP_EGA_PLANES * CORE_MACHINE_VADP_EGA_PLANE_BYTES));
        if (planar_vram == 0u) return LIB_STATUS_NO_MEMORY;
        status = core_machine_memory_register_device_provider_and_write_observer(memory,
            CORE_MACHINE_VADP_EGA_APERTURE_BASE,
            CORE_MACHINE_VADP_EGA_CPU_DECODE_BYTES,
            core_machine_vadp_ega_planar_read, core_machine_vadp_ega_planar_write,
            core_machine_vadp_ega_planar_query, adapter,
            core_machine_vadp_ega_write_observer);
        if (status != LIB_STATUS_OK) {
            lib_release((void *)planar_vram);
            return status;
        }
        adapter->data.ega_planar_vram = planar_vram;
    } else {
        status = core_machine_memory_register_write_observer(memory,
            core_machine_vadp_ega_write_observer, adapter);
        if (status != LIB_STATUS_OK) return status;
    }
    adapter->data.ega_sequencer = *config;
    adapter->data.ega_sequencer_configured = LIB_TRUE;
    adapter->data.ega_planar_enabled = config->planar_ega;
    core_machine_vadp_reset_sequencer(adapter);
    return LIB_STATUS_OK;
}

lib_status core_machine_vadp_configure_ega_controllers(t_vadp *adapter,
    const core_machine_vadp_ega_controller_config *config)
{
    if (adapter == LIB_NULL || config == LIB_NULL ||
        !adapter->data.ega_sequencer_configured) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (adapter->data.ega_controller_configured) return LIB_STATUS_INVALID_STATE;
    adapter->data.ega_controller = *config;
    core_machine_vadp_normalize_ega_controllers(&adapter->data.ega_controller);
    adapter->data.ega_controller_configured = LIB_TRUE;
    core_machine_vadp_reset_ega_controllers(adapter);
    return LIB_STATUS_OK;
}

lib_i32 core_machine_vadp_ega_aperture_contains(const t_vadp *adapter,
    lib_u32 physical, lib_size bytes)
{
    lib_u64 aperture_end;
    lib_u64 request_end;

    if (adapter == LIB_NULL || !adapter->data.ega_sequencer_configured ||
        bytes == 0u) return LIB_FALSE;
    {
        lib_u32 aperture_base;
        lib_u32 aperture_bytes;

        core_machine_vadp_active_ega_aperture(adapter, &aperture_base,
            &aperture_bytes);
        aperture_end = (lib_u64)aperture_base + aperture_bytes;
        request_end = (lib_u64)physical + bytes;
        return physical >= aperture_base && request_end <= aperture_end;
    }
}

lib_i32 core_machine_vadp_capture_text_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot)
{
    lib_u16 row;
    lib_u16 column;
    lib_u16 start;
    lib_u16 cursor;
    lib_u16 relative_cursor;
    lib_u16 start_byte;
    lib_u16 columns;
    lib_u16 rows;
    lib_size visible_bytes;
    lib_size first_bytes;
    lib_u8 cells[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS * 2u];
    lib_i32 buffer_changed = LIB_FALSE;
    lib_i32 cursor_changed;
    lib_i32 cursor_visible;

    if (adapter == LIB_NULL || memory == LIB_NULL || out_snapshot == LIB_NULL) return LIB_FALSE;
    columns = core_machine_vadp_text_columns(adapter);
    rows = core_machine_vadp_text_rows(adapter);
    if (columns == 0u || rows == 0u) return LIB_FALSE;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = CORE_MACHINE_DISPLAY_KIND_TEXT;
    start = core_machine_vadp_crtc_word(adapter, CORE_MACHINE_VADP_CRTC_START_HIGH);
    cursor = core_machine_vadp_crtc_word(adapter, CORE_MACHINE_VADP_CRTC_CURSOR_HIGH);
    visible_bytes = (lib_size)columns * rows * 2u;
    start_byte = (lib_u16)((start % (CORE_MACHINE_VADP_TEXT_BYTES / 2u)) * 2u);
    first_bytes = CORE_MACHINE_VADP_TEXT_BYTES - start_byte;
    if (first_bytes > visible_bytes) first_bytes = visible_bytes;
    if (core_machine_memory_read_physical(memory,
            CORE_MACHINE_VADP_TEXT_BASE + start_byte,
            (lib_uptr)cells, first_bytes) != LIB_STATUS_OK ||
        (first_bytes < visible_bytes && core_machine_memory_read_physical(memory,
            CORE_MACHINE_VADP_TEXT_BASE, (lib_uptr)(cells + first_bytes),
            visible_bytes - first_bytes) != LIB_STATUS_OK)) {
        return LIB_FALSE;
    }
    if ((adapter->data.ega_controller_configured &&
        !core_machine_vadp_ega_output_active(adapter)) ||
        (!adapter->data.ega_controller_configured &&
        (adapter->data.mode_control & CORE_MACHINE_VADP_MODE_VIDEO_ENABLE) == 0u)) {
        for (row = 0u; row < visible_bytes; row += 2u) {
            cells[row] = 0x20u;
            cells[row + 1u] = 0u;
        }
    }
    out_snapshot->columns = columns;
    out_snapshot->rows = rows;
    out_snapshot->text_cell_height = (lib_u8)(adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_MAXIMUM_RASTER_ADDRESS] + 1u);
    for (column = 0u; column < CORE_MACHINE_DISPLAY_PALETTE_ENTRIES;
        ++column) {
        out_snapshot->palette_rgb[column] = core_machine_vadp_rgbi_color(
            (lib_u8)column);
    }
    out_snapshot->cursor_top = adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_CURSOR_TOP] & 0x1fu;
    out_snapshot->cursor_bottom = adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM] & 0x1fu;
    relative_cursor = (lib_u16)((cursor - start) %
        (CORE_MACHINE_VADP_TEXT_BYTES / 2u));
    cursor_visible = (adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_TOP] &
        0x20u) == 0u && out_snapshot->cursor_top <= out_snapshot->cursor_bottom &&
        relative_cursor < columns * rows;
    out_snapshot->cursor_visible = cursor_visible;
    if (cursor_visible) {
        out_snapshot->cursor_x = (lib_u8)(relative_cursor % columns);
        out_snapshot->cursor_y = (lib_u8)(relative_cursor / columns);
    }
    /* Geometry is part of a copied text frame.  Firmware legitimately
     * programs CRTC geometry before it writes a lower row; comparing only the
     * overlapping cell bytes would otherwise retain a stale short frame in
     * every presentation consumer. */
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind !=
        CORE_MACHINE_DISPLAY_KIND_TEXT ||
        adapter->data.captured_columns != columns ||
        adapter->data.captured_rows != rows ||
        adapter->data.captured_text_cell_height != out_snapshot->text_cell_height ||
        lib_memory_compare(adapter->data.text_cells, cells, visible_bytes) != 0;
    if (buffer_changed) {
        lib_memory_copy(adapter->data.text_cells, cells, visible_bytes);
        for (row = 0u; row < rows; ++row) {
            for (column = 0u; column < columns; ++column) {
                lib_u16 index = (lib_u16)(row * CORE_MACHINE_DISPLAY_MAX_COLUMNS + column);
                lib_u16 cell = (lib_u16)(row * columns + column);
                adapter->data.characters[index] = cells[(lib_size)cell * 2u];
                adapter->data.attributes[index] = cells[(lib_size)cell * 2u + 1u];
            }
        }
    }
    lib_memory_copy(out_snapshot->characters, adapter->data.characters,
        sizeof(out_snapshot->characters));
    lib_memory_copy(out_snapshot->attributes, adapter->data.attributes,
        sizeof(out_snapshot->attributes));
    out_snapshot->text_glyphs_present = adapter->data.text_glyphs.present;
    lib_memory_copy(out_snapshot->text_glyphs, adapter->data.text_glyphs.bytes,
        sizeof(out_snapshot->text_glyphs));
    cursor_changed = !adapter->data.captured ||
        adapter->data.captured_cursor_top != out_snapshot->cursor_top ||
        adapter->data.captured_cursor_bottom != out_snapshot->cursor_bottom ||
        adapter->data.captured_cursor_address != cursor ||
        adapter->data.captured_cursor_x != out_snapshot->cursor_x ||
        adapter->data.captured_cursor_y != out_snapshot->cursor_y ||
        adapter->data.captured_cursor_visible != out_snapshot->cursor_visible;
    if (buffer_changed || cursor_changed) core_machine_vadp_mark_dirty(adapter);
    adapter->data.captured_cursor_top = out_snapshot->cursor_top;
    adapter->data.captured_cursor_bottom = out_snapshot->cursor_bottom;
    adapter->data.captured_cursor_address = cursor;
    adapter->data.captured_cursor_x = out_snapshot->cursor_x;
    adapter->data.captured_cursor_y = out_snapshot->cursor_y;
    adapter->data.captured_columns = columns;
    adapter->data.captured_rows = rows;
    adapter->data.captured_text_cell_height = out_snapshot->text_cell_height;
    adapter->data.captured_cursor_visible = out_snapshot->cursor_visible;
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = CORE_MACHINE_DISPLAY_KIND_TEXT;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = cursor_changed;
    return LIB_TRUE;
}

void core_machine_vadp_observe_snapshot(const t_vadp *adapter,
    lib_u8 acknowledged_generation_valid,
    lib_u64 acknowledged_generation,
    core_machine_display_snapshot_observation *out_observation)
{
    lib_i32 reliable;

    if (out_observation == LIB_NULL) return;
    lib_memory_set(out_observation, 0, sizeof(*out_observation));
    if (adapter == LIB_NULL) {
        out_observation->capture_required = LIB_TRUE;
        return;
    }
    reliable = adapter->data.cga_memory_configured ||
        (adapter->data.ega_planar_enabled &&
        !core_machine_vadp_ega_output_active(adapter)) ||
        core_machine_vadp_vga_mode13_active(adapter) ||
        core_machine_vadp_ega_planar_display_active(adapter);
    if (!reliable) {
        out_observation->capture_required = LIB_TRUE;
        return;
    }
    out_observation->generation = adapter->data.dirty_generation;
    out_observation->generation_reliable = LIB_TRUE;
    out_observation->capture_required = !acknowledged_generation_valid ||
        acknowledged_generation != out_observation->generation;
}

static lib_i32 core_machine_vadp_capture_graphics_snapshot(t_vadp *adapter,
    t_ram *memory, core_machine_display_snapshot *out_snapshot)
{
    lib_u8 bytes[CORE_MACHINE_VADP_VIDEO_BYTES];
    lib_u16 y;
    lib_u16 x;
    lib_i32 buffer_changed;

    if (adapter == LIB_NULL || memory == LIB_NULL || out_snapshot == LIB_NULL ||
        !core_machine_vadp_is_graphics_mode(adapter)) {
        return LIB_FALSE;
    }
    if (core_machine_memory_read_physical(memory, CORE_MACHINE_VADP_VIDEO_BASE,
            (lib_uptr)bytes, sizeof(bytes)) != LIB_STATUS_OK) {
        return LIB_FALSE;
    }
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind !=
        CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4 ||
        adapter->data.captured_mode_control != adapter->data.mode_control ||
        adapter->data.captured_color_select != adapter->data.color_select || lib_memory_compare(
        adapter->data.graphics_bytes, bytes, sizeof(bytes)) != 0;
    if (buffer_changed) {
        lib_memory_copy(adapter->data.graphics_bytes, bytes, sizeof(bytes));
    }
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4;
    out_snapshot->pixel_width = CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH;
    out_snapshot->pixel_height = CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT;
    core_machine_vadp_graphics_palette(adapter, out_snapshot->palette_rgb);
    for (y = 0u; y < CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT; ++y) {
        lib_u32 row_offset = (lib_u32)(y & 1u) *
            CORE_MACHINE_VADP_GRAPHICS_ODD_ROW_OFFSET + (lib_u32)(y >> 1) *
            CORE_MACHINE_VADP_GRAPHICS_BYTES_PER_ROW;
        for (x = 0u; x < CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH; ++x) {
            lib_u8 byte = bytes[row_offset + (x >> 2)];
            out_snapshot->pixels[(lib_u32)y * CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH + x] =
                (lib_u8)((byte >> (6u - 2u * (x & 3u))) & 0x03u);
        }
    }
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4;
    adapter->data.captured_mode_control = adapter->data.mode_control;
    adapter->data.captured_color_select = adapter->data.color_select;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = LIB_FALSE;
    return LIB_TRUE;
}

static lib_i32 core_machine_vadp_capture_high_res_graphics_snapshot(t_vadp *adapter,
    t_ram *memory, core_machine_display_snapshot *out_snapshot)
{
    lib_u8 bytes[CORE_MACHINE_VADP_VIDEO_BYTES];
    lib_u16 y;
    lib_u16 x;
    lib_i32 buffer_changed;

    if (adapter == LIB_NULL || memory == LIB_NULL || out_snapshot == LIB_NULL ||
        !core_machine_vadp_is_high_res_graphics_mode(adapter)) return LIB_FALSE;
    if (core_machine_memory_read_physical(memory, CORE_MACHINE_VADP_VIDEO_BASE,
            (lib_uptr)bytes, sizeof(bytes)) != LIB_STATUS_OK) {
        return LIB_FALSE;
    }
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind !=
        CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2 ||
        adapter->data.captured_mode_control != adapter->data.mode_control ||
        adapter->data.captured_color_select != adapter->data.color_select ||
        lib_memory_compare(adapter->data.graphics_bytes, bytes, sizeof(bytes)) != 0;
    if (buffer_changed) lib_memory_copy(adapter->data.graphics_bytes, bytes, sizeof(bytes));
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2;
    out_snapshot->pixel_width = CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH;
    out_snapshot->pixel_height = CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT;
    core_machine_vadp_high_res_palette(adapter, out_snapshot->palette_rgb);
    for (y = 0u; y < CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT; ++y) {
        lib_u32 row_offset = (lib_u32)(y & 1u) *
            CORE_MACHINE_VADP_GRAPHICS_ODD_ROW_OFFSET + (lib_u32)(y >> 1) *
            CORE_MACHINE_VADP_GRAPHICS_BYTES_PER_ROW;

        for (x = 0u; x < CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH; ++x) {
            lib_u8 byte = bytes[row_offset + (x >> 3u)];

            out_snapshot->pixels[(lib_u32)y * CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH + x] =
                (lib_u8)((byte >> (7u - (x & 7u))) & 0x01u);
        }
    }
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2;
    adapter->data.captured_mode_control = adapter->data.mode_control;
    adapter->data.captured_color_select = adapter->data.color_select;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = LIB_FALSE;
    return LIB_TRUE;
}

static lib_i32 core_machine_vadp_capture_ega_planar_snapshot(t_vadp *adapter,
    core_machine_display_snapshot *out_snapshot)
{
    core_machine_display_kind kind;
    lib_u16 width;
    lib_u16 height;
    lib_u16 row_bytes;
    lib_u32 start_byte;
    lib_u16 y;
    lib_u16 x;
    lib_i32 buffer_changed;

    if (adapter == LIB_NULL || out_snapshot == LIB_NULL ||
        !core_machine_vadp_ega_planar_active(adapter)) {
        return LIB_FALSE;
    }
    if (!core_machine_vadp_ega_display_kind(adapter, &kind)) return LIB_FALSE;
    width = kind == CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ?
        CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH : CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH;
    height = kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ?
        CORE_MACHINE_DISPLAY_EGA_HIGH_RES_HEIGHT : CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT;
    row_bytes = kind == CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ?
        CORE_MACHINE_VADP_EGA_320X200_ROW_BYTES :
        CORE_MACHINE_VADP_EGA_640X200_ROW_BYTES;
    /* EGA CRTC start is a word address; 64 KiB plane addressing wraps. */
    start_byte = ((lib_u32)core_machine_vadp_crtc_word(adapter,
        CORE_MACHINE_VADP_CRTC_START_HIGH) * 2u) &
        (CORE_MACHINE_VADP_EGA_PLANE_BYTES - 1u);
    if (core_machine_vadp_compaq_odd_even_page_active(adapter)) {
        start_byte = (start_byte & (CORE_MACHINE_VADP_EGA_ODD_EVEN_PAGE_BYTES - 1u)) |
            (adapter->data.compaq_odd_even_high_page ?
            CORE_MACHINE_VADP_EGA_ODD_EVEN_PAGE_BYTES : 0u);
    }
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind != kind ||
        adapter->data.captured_ega_dirty_generation != adapter->data.dirty_generation;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = kind;
    out_snapshot->pixel_width = width;
    out_snapshot->pixel_height = height;
    for (x = 0u; x < CORE_MACHINE_DISPLAY_PALETTE_ENTRIES; ++x) {
        lib_u8 enabled_index = (lib_u8)(x & adapter->data.attribute[18]);
        out_snapshot->palette_rgb[x] = core_machine_vadp_ega_palette_color(adapter,
            adapter->data.attribute[enabled_index]);
    }
    for (y = 0u; y < height; ++y) {
        for (x = 0u; x < width; ++x) {
            lib_u32 offset = (start_byte + (lib_u32)y * row_bytes +
                (x >> 3)) & (CORE_MACHINE_VADP_EGA_PLANE_BYTES - 1u);
            lib_u8 bit = (lib_u8)(0x80u >> (x & 7u));
            lib_u8 plane;
            lib_u8 pixel = 0u;

            for (plane = 0u; plane < CORE_MACHINE_VADP_EGA_PLANES; ++plane) {
                const lib_u8 *source = (const lib_u8 *)adapter->data.ega_planar_vram +
                    (lib_size)plane * CORE_MACHINE_VADP_EGA_PLANE_BYTES;
                if ((source[offset] & bit) != 0u) pixel |= (lib_u8)(1u << plane);
            }
            out_snapshot->pixels[(lib_u32)y * width + x] = pixel;
        }
    }
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = kind;
    adapter->data.captured_ega_dirty_generation = adapter->data.dirty_generation;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = LIB_FALSE;
    return LIB_TRUE;
}

static lib_i32 core_machine_vadp_capture_vga_mode13_snapshot(t_vadp *adapter,
    core_machine_display_snapshot *out_snapshot)
{
    lib_u32 pixel;
    lib_u16 index;

    if (adapter == LIB_NULL || out_snapshot == LIB_NULL ||
        !core_machine_vadp_vga_mode13_active(adapter)) return LIB_FALSE;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = CORE_MACHINE_DISPLAY_KIND_VGA_320X200X256;
    out_snapshot->pixel_width = CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH;
    out_snapshot->pixel_height = CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT;
    for (index = 0u; index < CORE_MACHINE_DISPLAY_PALETTE_ENTRIES; ++index) {
        const lib_u8 *entry = adapter->data.vga_dac[index &
            adapter->data.vga_dac_mask];
        out_snapshot->palette_rgb[index] =
            ((lib_u32)((entry[0] << 2u) | (entry[0] >> 4u)) << 16u) |
            ((lib_u32)((entry[1] << 2u) | (entry[1] >> 4u)) << 8u) |
            (lib_u32)((entry[2] << 2u) | (entry[2] >> 4u));
    }
    for (pixel = 0u; pixel < CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH *
        CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT; ++pixel) {
        const lib_u8 *plane = (const lib_u8 *)adapter->data.ega_planar_vram +
            (lib_size)(pixel & 3u) * CORE_MACHINE_VADP_EGA_PLANE_BYTES;
        out_snapshot->pixels[pixel] = plane[pixel >> 2u];
    }
    out_snapshot->buffer_changed = !adapter->data.captured ||
        adapter->data.captured_kind != CORE_MACHINE_DISPLAY_KIND_VGA_320X200X256 ||
        adapter->data.captured_ega_dirty_generation != adapter->data.dirty_generation;
    out_snapshot->cursor_changed = LIB_FALSE;
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = CORE_MACHINE_DISPLAY_KIND_VGA_320X200X256;
    adapter->data.captured_ega_dirty_generation = adapter->data.dirty_generation;
    return LIB_TRUE;
}

static lib_i32 core_machine_vadp_capture_blank_ega_snapshot(t_vadp *adapter,
    core_machine_display_snapshot *out_snapshot)
{
    core_machine_display_kind kind;

    if (adapter == LIB_NULL || out_snapshot == LIB_NULL ||
        !core_machine_vadp_ega_display_kind(adapter, &kind)) return LIB_FALSE;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = kind;
    out_snapshot->pixel_width = kind == CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ?
        CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH : CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH;
    out_snapshot->pixel_height = kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ?
        CORE_MACHINE_DISPLAY_EGA_HIGH_RES_HEIGHT : CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT;
    out_snapshot->buffer_changed = !adapter->data.captured ||
        adapter->data.captured_kind != kind ||
        adapter->data.captured_ega_dirty_generation != adapter->data.dirty_generation;
    out_snapshot->cursor_changed = LIB_FALSE;
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = kind;
    adapter->data.captured_ega_dirty_generation = adapter->data.dirty_generation;
    return LIB_TRUE;
}

lib_i32 core_machine_vadp_capture_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot)
{
    if (adapter != LIB_NULL && adapter->data.ega_planar_enabled &&
        !core_machine_vadp_ega_output_active(adapter) &&
        core_machine_vadp_capture_blank_ega_snapshot(adapter, out_snapshot)) {
        return LIB_TRUE;
    }
    if (core_machine_vadp_vga_mode13_active(adapter)) {
        return core_machine_vadp_capture_vga_mode13_snapshot(adapter, out_snapshot);
    }
    if (core_machine_vadp_ega_planar_display_active(adapter)) {
        return core_machine_vadp_capture_ega_planar_snapshot(adapter, out_snapshot);
    }
    if (core_machine_vadp_is_high_res_graphics_mode(adapter)) {
        return core_machine_vadp_capture_high_res_graphics_snapshot(adapter, memory,
            out_snapshot);
    }
    return core_machine_vadp_is_graphics_mode(adapter) ?
        core_machine_vadp_capture_graphics_snapshot(adapter, memory, out_snapshot) :
        core_machine_vadp_capture_text_snapshot(adapter, memory, out_snapshot);
}
