/* Copyright 2012-2026 Neko. */
#include "core/chips/video/video.h"
#define X86_VIDEO_STATUS_DISPLAY_ENABLE 0x01u
#define X86_VIDEO_STATUS_LIGHTPEN_TRIGGER 0x02u
#define X86_VIDEO_STATUS_LIGHTPEN_SWITCH_OPEN 0x04u
#define X86_VIDEO_STATUS_VERTICAL_RETRACE 0x08u
#define X86_VIDEO_DEFAULT_ACTIVE_DISPLAY_TICKS 48u
#define X86_VIDEO_DEFAULT_HORIZONTAL_BLANK_TICKS 8u
#define X86_VIDEO_DEFAULT_VERTICAL_RETRACE_TICKS 8u
#define X86_VIDEO_CRTC_HORIZONTAL_TOTAL 0x00u
#define X86_VIDEO_CRTC_HORIZONTAL_DISPLAYED 0x01u
#define X86_VIDEO_CRTC_HORIZONTAL_SYNC_POSITION 0x02u
#define X86_VIDEO_CRTC_SYNC_WIDTH 0x03u
#define X86_VIDEO_CRTC_VERTICAL_TOTAL 0x04u
#define X86_VIDEO_EGA_CRTC_VERTICAL_TOTAL 0x06u
#define X86_VIDEO_CRTC_VERTICAL_TOTAL_ADJUST 0x05u
#define X86_VIDEO_CRTC_VERTICAL_DISPLAYED 0x06u
#define X86_VIDEO_CRTC_OVERFLOW 0x07u
#define X86_VIDEO_CRTC_VERTICAL_SYNC_POSITION 0x07u
#define X86_VIDEO_CRT_INTERLACE_SKEW 0x08u
#define X86_VIDEO_CRTC_MAXIMUM_RASTER_ADDRESS 0x09u
#define X86_VIDEO_CRTC_CURSOR_TOP 0x0au
#define X86_VIDEO_CRTC_CURSOR_BOTTOM 0x0bu
#define X86_VIDEO_CRTC_START_HIGH 0x0cu
#define X86_VIDEO_CRTC_START_LOW 0x0du
#define X86_VIDEO_CRTC_CURSOR_HIGH 0x0eu
#define X86_VIDEO_CRTC_CURSOR_LOW 0x0fu
#define X86_VIDEO_CRTC_OFFSET 0x13u
#define X86_VIDEO_CRTC_VERTICAL_DISPLAY_END 0x12u
#define X86_VIDEO_CRTC_VERTICAL_RETRACE_END 0x11u
#define X86_VIDEO_CRTC_VERTICAL_RETRACE_START 0x10u
#define X86_VIDEO_CRTC_UNDERLINE_LOCATION 0x14u
#define X86_VIDEO_CRTC_START_VERTICAL_BLANK 0x15u
#define X86_VIDEO_CRTC_END_VERTICAL_BLANK 0x16u
#define X86_VIDEO_CRTC_MODE_CONTROL 0x17u
#define X86_VIDEO_CRTC_LINE_COMPARE 0x18u
#define X86_VIDEO_MODE_GRAPHICS 0x02u
#define X86_VIDEO_MODE_80_COLUMN 0x01u
#define X86_VIDEO_MODE_VIDEO_ENABLE 0x08u
#define X86_VIDEO_MODE_HIGH_RES 0x10u
#define X86_VIDEO_MODE_BLINK 0x20u
#define X86_VIDEO_COLOR_PALETTE_SELECT 0x20u
#define X86_VIDEO_GRAPHICS_BYTES_PER_ROW 80u
#define X86_VIDEO_GRAPHICS_ODD_ROW_OFFSET 0x2000u

_Static_assert(X86_VIDEO_CRTC_CURSOR_TOP <
        X86_VIDEO_CRTC_REGISTER_COUNT &&
    X86_VIDEO_CRTC_CURSOR_BOTTOM <
        X86_VIDEO_CRTC_REGISTER_COUNT &&
    X86_VIDEO_CRTC_START_HIGH + 1u <
        X86_VIDEO_CRTC_REGISTER_COUNT &&
    X86_VIDEO_CRTC_CURSOR_HIGH + 1u <
        X86_VIDEO_CRTC_REGISTER_COUNT &&
    X86_VIDEO_CRTC_OFFSET < X86_VIDEO_CRTC_REGISTER_COUNT,
    "CRTC constant indices must fit the VADP CRTC register bank");

static void x86_video_mark_dirty(x86_video *adapter);
static lib_i32 x86_video_ega_display_kind(const x86_video *adapter,
    x86_video_kind *out_kind);

static lib_bool x86_video_memory_range_contains(lib_u32 base, lib_u32 capacity,
    lib_u32 physical, lib_size bytes)
{
    lib_u32 offset;

    if (physical < base) return LIB_FALSE;
    offset = physical - base;
    return offset <= capacity && bytes <= capacity - offset;
}

static lib_status x86_video_cga_read(x86_video *adapter,
    lib_u32 physical, lib_u8 *destination,
    lib_size bytes)
{
    if (adapter == LIB_NULL || destination == LIB_NULL ||
        !x86_video_memory_range_contains(X86_VIDEO_VIDEO_BASE,
            X86_VIDEO_VIDEO_BYTES, physical, bytes)) return LIB_STATUS_UNSUPPORTED;
    lib_memory_copy(destination, adapter->data.cga_vram +
        physical - X86_VIDEO_VIDEO_BASE, bytes);
    return LIB_STATUS_OK;
}

static lib_status x86_video_cga_write(x86_video *adapter,
    lib_u32 physical, const lib_u8 *source,
    lib_size bytes)
{
    if (adapter == LIB_NULL || source == LIB_NULL ||
        !x86_video_memory_range_contains(X86_VIDEO_VIDEO_BASE,
            X86_VIDEO_VIDEO_BYTES, physical, bytes)) return LIB_STATUS_UNSUPPORTED;
    lib_memory_copy(adapter->data.cga_vram + physical - X86_VIDEO_VIDEO_BASE,
        source, bytes);
    x86_video_mark_dirty(adapter);
    return LIB_STATUS_OK;
}

static lib_status x86_video_cga_query(x86_video *owner,
    lib_u32 physical, lib_size bytes,
    lib_bool write)
{
    (void)owner;
    return (write == LIB_FALSE || write == LIB_TRUE) &&
        x86_video_memory_range_contains(X86_VIDEO_VIDEO_BASE,
            X86_VIDEO_VIDEO_BYTES, physical, bytes) ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_i32 x86_video_is_graphics_mode(const x86_video *adapter)
{
    return adapter != LIB_NULL &&
        (adapter->data.mode_control & (X86_VIDEO_MODE_GRAPHICS |
            X86_VIDEO_MODE_HIGH_RES)) == X86_VIDEO_MODE_GRAPHICS;
}

static lib_i32 x86_video_is_high_res_graphics_mode(const x86_video *adapter)
{
    return adapter != LIB_NULL && (adapter->data.mode_control &
        (X86_VIDEO_MODE_GRAPHICS | X86_VIDEO_MODE_HIGH_RES)) ==
        (X86_VIDEO_MODE_GRAPHICS | X86_VIDEO_MODE_HIGH_RES);
}

static lib_u32 x86_video_rgbi_color(lib_u8 index)
{
    static const lib_u32 colors[16] = {
        0x000000u, 0x0000aau, 0x00aa00u, 0x00aaaau,
        0xaa0000u, 0xaa00aau, 0xaa5500u, 0xaaaaaau,
        0x555555u, 0x5555ffu, 0x55ff55u, 0x55ffffu,
        0xff5555u, 0xff55ffu, 0xffff55u, 0xffffffu
    };

    return colors[index & 0x0fu];
}

/* EGA/CECG's six digital palette bits are r g b R G B,
 * from bit 5 through bit 0. Snapshot RGB expands each primary/secondary
 * pair to the project 0x00..0xff capture range; it is not a monitor model. */
static lib_u32 x86_video_ega_digital_color(lib_u8 value)
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

static lib_u32 x86_video_ega_palette_color(const x86_video *adapter,
    lib_u8 value)
{
    return adapter != LIB_NULL && adapter->data.vga_configured ?
        x86_video_rgbi_color(value & 0x0fu) : x86_video_ega_digital_color(value & 0x3fu);
}

static lib_i32 x86_video_ega_output_active(const x86_video *adapter);
static lib_u8 x86_video_ega_graphics_diagnostic(const x86_video *adapter);

static void x86_video_graphics_palette(const x86_video *adapter,
    lib_u32 palette[4])
{
    lib_i32 alternate;
    lib_i32 intensified;

    if (adapter == LIB_NULL || palette == LIB_NULL) return;
    alternate = (adapter->data.color_select &
        X86_VIDEO_COLOR_PALETTE_SELECT) != 0u;
    intensified = (adapter->data.color_select & 0x10u) != 0u;
    palette[0] = x86_video_rgbi_color(adapter->data.color_select);
    palette[1] = x86_video_rgbi_color((alternate ? 3u : 2u) |
        (intensified ? 8u : 0u));
    palette[2] = x86_video_rgbi_color((alternate ? 5u : 4u) |
        (intensified ? 8u : 0u));
    palette[3] = x86_video_rgbi_color((alternate ? 7u : 6u) |
        (intensified ? 8u : 0u));
    if ((adapter->data.ega_controller_configured &&
        !x86_video_ega_output_active(adapter)) ||
        (!adapter->data.ega_controller_configured &&
        (adapter->data.mode_control & X86_VIDEO_MODE_VIDEO_ENABLE) == 0u)) {
        palette[0] = 0u;
        palette[1] = 0u;
        palette[2] = 0u;
        palette[3] = 0u;
    }
}

static void x86_video_high_res_palette(const x86_video *adapter,
    lib_u32 palette[X86_VIDEO_PALETTE_ENTRIES])
{
    if (adapter == LIB_NULL || palette == LIB_NULL) return;
    palette[0] = 0u;
    palette[1] = (adapter->data.mode_control & X86_VIDEO_MODE_VIDEO_ENABLE) != 0u ?
        x86_video_rgbi_color(adapter->data.color_select & 0x0fu) : 0u;
}

static lib_bool x86_video_active_ega_aperture(const x86_video *adapter,
    lib_u32 *out_base, lib_u32 *out_bytes);

static lib_i32 x86_video_ega_output_active(const x86_video *adapter)
{
    return adapter != LIB_NULL && adapter->data.ega_planar_enabled &&
        adapter->data.ega_planar_vram != LIB_NULL &&
        adapter->data.ega_sequencer_configured &&
        (adapter->data.sequencer[0] & 0x03u) == 0x03u &&
        adapter->data.ega_controller_configured &&
        (adapter->data.graphics[5] & 0x04u) == 0u &&
        adapter->data.attribute_display_enabled &&
        (adapter->data.ega_personality != X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR ||
         (adapter->data.compaq_control_mode &
            ((adapter->data.compaq_control_mode & 8u) != 0u ? 0x20u : 0x10u)) == 0u);
}

/* CPU access to the EGA aperture is a mapping decision, not a presentation
 * decision.  In particular, firmware may clear text memory while display
 * output is disabled.  Letting that access fall through to ordinary RAM
 * creates a second owner for video state and corrupts board aliases which
 * legitimately reuse the conventional video-hole backing. */
static lib_i32 x86_video_ega_aperture_mapped(const x86_video *adapter)
{
    return adapter != LIB_NULL && adapter->data.ega_planar_enabled &&
        adapter->data.ega_planar_vram != LIB_NULL && adapter->data.ega_sequencer_configured &&
        (adapter->data.sequencer[0] & 0x03u) == 0x03u &&
        adapter->data.ega_controller_configured;
}

static lib_i32 x86_video_ega_cpu_aperture_active(const x86_video *adapter)
{
    return x86_video_ega_aperture_mapped(adapter) &&
        (adapter->data.graphics[5] & 0x04u) == 0u &&
        (adapter->data.ega_personality !=
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR ||
         !adapter->data.compaq_cpu_video_memory_disabled);
}

static lib_i32 x86_video_ega_planar_active(const x86_video *adapter)
{
    return x86_video_ega_output_active(adapter) &&
        (adapter->data.graphics[6] & 0x01u) != 0u;
}

static lib_i32 x86_video_vga_chain4_active(const x86_video *adapter)
{
    return x86_video_ega_cpu_aperture_active(adapter) &&
        adapter->data.vga_configured && (adapter->data.sequencer[4] & 0x08u) != 0u;
}

static lib_i32 x86_video_vga_mode13_active(const x86_video *adapter)
{
    return x86_video_vga_chain4_active(adapter) &&
        (adapter->data.graphics[5] & 0x40u) != 0u;
}

static lib_i32 x86_video_ega_planar_display_active(const x86_video *adapter)
{
    x86_video_kind kind;

    return x86_video_ega_planar_active(adapter) &&
        x86_video_ega_display_kind(adapter, &kind);
}

static lib_u8 x86_video_rotate_right(lib_u8 value, lib_u8 count)
{
    count &= 7u;
    return count == 0u ? value : (lib_u8)((value >> count) |
        (value << (8u - count)));
}

static lib_u8 x86_video_logical_operation(lib_u8 operation,
    lib_u8 source, lib_u8 latch)
{
    switch (operation & 0x03u) {
    case 1u: return source & latch;
    case 2u: return source | latch;
    case 3u: return source ^ latch;
    default: return source;
    }
}

static lib_u8 x86_video_ega_color_compare(
    const x86_video *adapter, const lib_u8 *latches)
{
    lib_u8 value = 0xffu;
    lib_u8 plane;

    if (adapter == LIB_NULL) return 0u;
    for (plane = 0u; plane < X86_VIDEO_EGA_PLANES; ++plane) {
        if ((adapter->data.graphics[7] & (1u << plane)) != 0u) continue;
        value &= (adapter->data.graphics[2] & (1u << plane)) != 0u ?
            latches[plane] : (lib_u8)~latches[plane];
    }
    return value;
}

static lib_u8 x86_video_ega_write_source(const x86_video *adapter,
    lib_u8 input, lib_u8 plane)
{
    lib_u8 mode = adapter->data.graphics[5] & 0x03u;

    if (mode == 1u) return adapter->data.ega_latches[plane];
    if (mode == 2u) return (input & (1u << plane)) != 0u ? 0xffu : 0u;
    return (adapter->data.graphics[1] & (1u << plane)) != 0u ?
        (adapter->data.graphics[0] & (1u << plane)) != 0u ? 0xffu : 0u :
        x86_video_rotate_right(input, adapter->data.graphics[3]);
}

/* The selected CECG model retains its established L2 B0000h compatibility
 * alias when map 3 selects B8000h. Both use one planar store; this is not
 * primary-source proof of the physical board's alias decode. */
static lib_i32 x86_video_compaq_b000_compatibility_contains(
    const x86_video *adapter, lib_u32 physical, lib_size bytes)
{
    return adapter != LIB_NULL && bytes != 0u && adapter->data.ega_personality ==
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR &&
        x86_video_ega_cpu_aperture_active(adapter) &&
        ((adapter->data.graphics[6] >> 2u) & 0x03u) == 3u &&
        x86_video_memory_range_contains(0x000b0000u, 0x00008000u,
            physical, bytes);
}

static lib_u32 x86_video_ega_planar_offset(const x86_video *adapter,
    lib_u32 physical)
{
    lib_u32 aperture_base;
    lib_u32 aperture_bytes;
    lib_u32 offset;

    if (x86_video_compaq_b000_compatibility_contains(adapter, physical, 1u)) {
        physical += 0x00008000u;
    }
    if (!x86_video_active_ega_aperture(adapter, &aperture_base,
        &aperture_bytes)) return 0u;
    offset = physical - aperture_base;
    if (aperture_bytes == 0x00020000u) offset &= 0x0000ffffu;
    else if (aperture_bytes == 0x00008000u) offset &= 0x00007fffu;
    if (!adapter->data.vga_configured && (adapter->data.graphics[6] & 0x02u) != 0u) {
        const lib_bool high_page = adapter->data.ega_personality ==
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR ?
            adapter->data.compaq_odd_even_high_page :
            (adapter->data.ega_miscellaneous_output & 0x20u) != 0u;
        /* Retained 256-KiB store: 128-KiB mapping selects A16, other
         * mappings use the programmed CPU page. Replace A0, not all bits. */
        offset = (offset & ~1u) | (aperture_bytes == 0x00020000u ?
            ((physical - aperture_base) >> 16u) & 1u : high_page);
    }
    if ((adapter->data.sequencer[4] & 0x02u) == 0u) offset &= 0x00003fffu;
    return offset;
}

static lib_i32 x86_video_ega_cpu_aperture_contains(const x86_video *adapter,
    lib_u32 physical, lib_size bytes)
{
    return x86_video_ega_cpu_aperture_active(adapter) &&
        (x86_video_ega_aperture_contains(adapter, physical, bytes) ||
        x86_video_compaq_b000_compatibility_contains(adapter, physical, bytes));
}

static lib_status x86_video_ega_planar_read(x86_video *adapter,
    lib_u32 physical, lib_u8 *destination,
    lib_size bytes, lib_bool observe_only)
{
    lib_size index;

    if (adapter == LIB_NULL || destination == LIB_NULL ||
        !x86_video_ega_aperture_mapped(adapter)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if (!x86_video_ega_cpu_aperture_contains(adapter, physical, bytes)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    for (index = 0u; index < bytes; ++index) {
        lib_u32 address = physical + (lib_u32)index;
        lib_u32 offset = x86_video_vga_chain4_active(adapter) ?
            x86_video_ega_planar_offset(adapter, address) >> 2u :
            x86_video_ega_planar_offset(adapter, address);
        lib_u8 plane;
        lib_u8 latches[X86_VIDEO_EGA_PLANES];

        for (plane = 0u; plane < X86_VIDEO_EGA_PLANES; ++plane) {
            latches[plane] = adapter->data.ega_planar_vram
                [(lib_size)plane * X86_VIDEO_EGA_PLANE_BYTES + offset];
        }
        if (!observe_only)
            lib_memory_copy(adapter->data.ega_latches, latches, sizeof(latches));
        if ((adapter->data.graphics[5] & 0x08u) != 0u) {
            destination[index] = x86_video_ega_color_compare(adapter, latches);
        } else {
            lib_u8 map = adapter->data.graphics[4];

            if (x86_video_vga_chain4_active(adapter)) map = (lib_u8)(address & 3u);
            else if (!adapter->data.vga_configured && map < X86_VIDEO_EGA_PLANES &&
                (adapter->data.graphics[5] & 0x10u) != 0u)
                map = (lib_u8)((map & 2u) | (address & 1u));

            destination[index] = map < X86_VIDEO_EGA_PLANES ?
                latches[map] : 0u;
        }
    }
    return LIB_STATUS_OK;
}

static lib_status x86_video_ega_planar_write(x86_video *adapter,
    lib_u32 physical, const lib_u8 *source,
    lib_size bytes)
{
    lib_size index;

    if (adapter == LIB_NULL || source == LIB_NULL ||
        !x86_video_ega_aperture_mapped(adapter)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if (!x86_video_ega_cpu_aperture_contains(adapter, physical, bytes)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if ((adapter->data.graphics[5] & 0x03u) == 0x03u) {
        return LIB_STATUS_UNSUPPORTED;
    }
    for (index = 0u; index < bytes; ++index) {
        lib_u32 address = physical + (lib_u32)index;
        lib_u32 offset = x86_video_vga_chain4_active(adapter) ?
            x86_video_ega_planar_offset(adapter, address) >> 2u :
            x86_video_ega_planar_offset(adapter, address);
        lib_u8 write_mask = adapter->data.sequencer[2];
        lib_u8 plane;

        if (x86_video_vga_chain4_active(adapter)) write_mask &= (lib_u8)(1u << (address & 3u));
        else if (!adapter->data.vga_configured && (adapter->data.sequencer[4] & 4u) == 0u)
            write_mask &= (address & 1u) != 0u ? 0x0au : 0x05u;
        for (plane = 0u; plane < X86_VIDEO_EGA_PLANES; ++plane) {
            lib_u8 *target = adapter->data.ega_planar_vram +
                (lib_size)plane * X86_VIDEO_EGA_PLANE_BYTES + offset;
            lib_u8 source_byte = x86_video_ega_write_source(adapter,
                source[index], plane);
            lib_u8 merged = x86_video_logical_operation(
                adapter->data.graphics[3] >> 3, source_byte,
                adapter->data.ega_latches[plane]);

            if ((write_mask & (1u << plane)) != 0u) {
                *target = (adapter->data.graphics[5] & 0x03u) == 1u ?
                    adapter->data.ega_latches[plane] :
                    (lib_u8)((merged & adapter->data.graphics[8]) |
                    (adapter->data.ega_latches[plane] &
                    (lib_u8)~adapter->data.graphics[8]));
            }
        }
    }
    x86_video_mark_dirty(adapter);
    return LIB_STATUS_OK;
}

static lib_status x86_video_ega_planar_query(x86_video *adapter,
    lib_u32 physical, lib_size bytes,
    lib_bool write)
{
    if (adapter == LIB_NULL ||
        (write != LIB_FALSE && write != LIB_TRUE) ||
        !x86_video_ega_cpu_aperture_contains(adapter, physical, bytes)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static lib_i32 x86_video_cga_logical_raster_active(const x86_video *adapter)
{
    return adapter != LIB_NULL && !adapter->data.ega_controller_configured &&
        adapter->data.crtc[X86_VIDEO_CRTC_HORIZONTAL_DISPLAYED] != 0u &&
        adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_DISPLAYED] != 0u;
}

static lib_u32 x86_video_cga_scanlines_per_row(const x86_video *adapter)
{
    return (lib_u32)adapter->data.crtc[
        X86_VIDEO_CRTC_MAXIMUM_RASTER_ADDRESS] + 1u;
}

static lib_u32 x86_video_cga_clocks_per_character(const x86_video *adapter)
{
    return (adapter->data.mode_control & X86_VIDEO_MODE_80_COLUMN) != 0u ? 1u : 2u;
}

static lib_u32 x86_video_cga_logical_raster_period(const x86_video *adapter)
{
    lib_u32 rows = (lib_u32)adapter->data.crtc[
        X86_VIDEO_CRTC_VERTICAL_TOTAL] + 1u;
    lib_u32 scanlines = rows * x86_video_cga_scanlines_per_row(adapter) +
        adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_TOTAL_ADJUST];

    return scanlines * ((lib_u32)adapter->data.crtc[
        X86_VIDEO_CRTC_HORIZONTAL_TOTAL] + 1u) * x86_video_cga_clocks_per_character(adapter);
}

static lib_u16 x86_video_text_columns(const x86_video *adapter)
{
    lib_u16 columns;

    if (!x86_video_cga_logical_raster_active(adapter)) return adapter->data.columns;
    columns = adapter->data.crtc[X86_VIDEO_CRTC_HORIZONTAL_DISPLAYED];
    return columns > X86_VIDEO_MAX_COLUMNS ? X86_VIDEO_MAX_COLUMNS :
        columns;
}

static lib_u16 x86_video_text_rows(const x86_video *adapter)
{
    lib_u16 rows;

    if (!x86_video_cga_logical_raster_active(adapter)) return adapter->data.rows;
    rows = adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_DISPLAYED];
    return rows > X86_VIDEO_CGA_TEXT_MAX_ROWS ? X86_VIDEO_CGA_TEXT_MAX_ROWS : rows;
}

static lib_i32 x86_video_supported_crtc_index(const x86_video *adapter,
    lib_u8 index)
{
    if (adapter == LIB_NULL || index >= X86_VIDEO_CRTC_REGISTER_COUNT) return LIB_FALSE;
    if (!adapter->data.ega_controller_configured) return index <= 0x11u;
    return index <= X86_VIDEO_CRTC_EGA_LAST;
}

static lib_i32 x86_video_crtc_index_readable(const x86_video *adapter,
    lib_u8 index)
{
    if (!x86_video_supported_crtc_index(adapter, index)) return LIB_FALSE;
    if (!adapter->data.ega_controller_configured) {
        return index >= X86_VIDEO_CRTC_CURSOR_HIGH;
    }
    return index >= X86_VIDEO_CRTC_START_HIGH &&
        index <= X86_VIDEO_CRTC_CURSOR_LOW;
}

static lib_i32 x86_video_crtc_index_writable(const x86_video *adapter,
    lib_u8 index)
{
    if (!x86_video_supported_crtc_index(adapter, index)) return LIB_FALSE;
    return adapter->data.ega_controller_configured ||
        index <= X86_VIDEO_CRTC_CURSOR_LOW;
}

static lib_u8 x86_video_crtc_mask(const x86_video *adapter,
    lib_u8 index)
{
    if (adapter != LIB_NULL && adapter->data.ega_controller_configured) {
        switch (index) {
        case 0x03u: return 0x7fu;
        case 0x07u: return 0x3fu;
        case X86_VIDEO_CRT_INTERLACE_SKEW:
        case X86_VIDEO_CRTC_MAXIMUM_RASTER_ADDRESS:
        case X86_VIDEO_CRTC_CURSOR_TOP:
        case X86_VIDEO_CRTC_UNDERLINE_LOCATION:
        case X86_VIDEO_CRTC_END_VERTICAL_BLANK:
            return 0x1fu;
        case X86_VIDEO_CRTC_CURSOR_BOTTOM: return 0x7fu;
        case X86_VIDEO_CRTC_VERTICAL_RETRACE_END: return 0x3fu;
        default: return 0xffu;
        }
    }
    switch (index) {
    case X86_VIDEO_CRTC_VERTICAL_TOTAL:
    case X86_VIDEO_CRTC_VERTICAL_DISPLAYED:
    case X86_VIDEO_CRTC_VERTICAL_SYNC_POSITION:
        return 0x7fu;
    case X86_VIDEO_CRTC_VERTICAL_TOTAL_ADJUST:
    case X86_VIDEO_CRTC_MAXIMUM_RASTER_ADDRESS:
        return 0x1fu;
    case X86_VIDEO_CRTC_CURSOR_TOP:
        return 0x7fu;
    case X86_VIDEO_CRTC_CURSOR_BOTTOM:
        return 0x1fu;
    case X86_VIDEO_CRTC_START_HIGH:
    case X86_VIDEO_CRTC_CURSOR_HIGH:
        return 0x3fu;
    default:
        return 0xffu;
    }
}

static lib_u16 x86_video_crtc_word(const x86_video *adapter,
    lib_u8 high_index)
{
    lib_u8 low_index = (lib_u8)(high_index + 1u);

    if (adapter == LIB_NULL || !x86_video_supported_crtc_index(adapter,
            high_index) || !x86_video_supported_crtc_index(adapter, low_index)) {
        return 0u;
    }
    return (lib_u16)(((lib_u16)adapter->data.crtc[high_index] << 8) |
        adapter->data.crtc[low_index]);
}

static lib_u16 x86_video_ega_vertical_displayed(
    const x86_video *adapter)
{
    return adapter == LIB_NULL ? 0u : (lib_u16)(
        adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_DISPLAY_END] +
        ((adapter->data.crtc[X86_VIDEO_CRTC_OVERFLOW] & 0x02u) << 7u) + 1u);
}

static lib_u32 x86_video_ega_horizontal_scale(const x86_video *adapter)
{
    return (adapter->data.sequencer[1] & 8u) != 0u ? 2u : 1u;
}

static lib_u32 x86_video_ega_vertical_scale(const x86_video *adapter)
{
    return (adapter->data.crtc[X86_VIDEO_CRTC_MODE_CONTROL] & 4u) != 0u ? 2u : 1u;
}

static lib_i32 x86_video_ega_display_kind(const x86_video *adapter,
    x86_video_kind *out_kind)
{
    lib_u16 horizontal;
    lib_u16 vertical;

    if (adapter == LIB_NULL || out_kind == LIB_NULL) return LIB_FALSE;
    horizontal = (lib_u16)adapter->data.crtc[
        X86_VIDEO_CRTC_HORIZONTAL_DISPLAYED] + 1u;
    vertical = (lib_u16)(x86_video_ega_vertical_displayed(adapter) *
        x86_video_ega_vertical_scale(adapter));
    if (horizontal == 40u && vertical == 200u) {
        *out_kind = X86_VIDEO_KIND_EGA_320X200X16;
        return LIB_TRUE;
    }
    if (horizontal == 80u && vertical == 200u) {
        *out_kind = X86_VIDEO_KIND_EGA_640X200X16;
        return LIB_TRUE;
    }
    if (horizontal == 80u && vertical == 350u) {
        *out_kind = X86_VIDEO_KIND_EGA_640X350X16;
        return LIB_TRUE;
    }
    return LIB_FALSE;
}

static void x86_video_mark_dirty(x86_video *adapter)
{
    if (adapter != LIB_NULL) ++adapter->data.dirty_generation;
}

static lib_i32 x86_video_sequencer_index_supported(lib_u8 index)
{
    return index == 0u || index == 1u || index == 2u || index == 3u || index == 4u;
}

static lib_u8 x86_video_sequencer_mask(const x86_video *adapter, lib_u8 index)
{
    switch (index) {
    case 0u: return 0x03u;
    case 1u: return adapter->data.vga_configured ? 0x3du : 0x0fu;
    case 2u: return 0x0fu;
    case 3u: return adapter->data.vga_configured ? 0x3fu : 0x0fu;
    case 4u: return adapter->data.vga_configured ? 0x0eu : 0x07u;
    default: return 0u;
    }
}

static lib_i32 x86_video_graphics_index_supported(lib_u8 index)
{
    return index < X86_VIDEO_GRAPHICS_REGISTER_COUNT;
}

static lib_u8 x86_video_graphics_mask(lib_u8 index)
{
    static const lib_u8 masks[X86_VIDEO_GRAPHICS_REGISTER_COUNT] = {
        0x0fu, 0x0fu, 0x0fu, 0x1fu, 0x07u, 0x7fu, 0x0fu, 0x0fu, 0xffu
    };

    return x86_video_graphics_index_supported(index) ? masks[index] : 0u;
}

static lib_i32 x86_video_attribute_index_supported(lib_u8 index)
{
    return index < 20u;
}

static lib_u8 x86_video_attribute_mask(lib_u8 index)
{
    if (index < 16u) return 0x3fu;
    switch (index) {
    case 16u: return 0x0fu;
    case 17u: return 0x3fu;
    case 18u: return 0x3fu;
    case 19u: return 0x0fu;
    case 20u: return 0x0fu;
    default: return 0u;
    }
}

static lib_bool x86_video_active_ega_aperture(const x86_video *adapter,
    lib_u32 *out_base, lib_u32 *out_bytes)
{
    lib_u8 map_select;

    if (adapter == LIB_NULL || out_base == LIB_NULL || out_bytes == LIB_NULL) {
        return LIB_FALSE;
    }
    *out_base = adapter->data.ega_sequencer.aperture_base;
    *out_bytes = adapter->data.ega_sequencer.aperture_bytes;
    if (!adapter->data.ega_controller_configured) return LIB_TRUE;
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
    return LIB_TRUE;
}

static void x86_video_reset_sequencer(x86_video *adapter)
{
    if (adapter == LIB_NULL || !adapter->data.ega_sequencer_configured) return;
    adapter->data.sequencer_index = 0u;
    adapter->data.sequencer[0] = adapter->data.ega_sequencer.reset &
        x86_video_sequencer_mask(adapter, 0u);
    adapter->data.sequencer[1] = adapter->data.ega_sequencer.clocking_mode &
        x86_video_sequencer_mask(adapter, 1u);
    adapter->data.sequencer[2] = adapter->data.ega_sequencer.map_mask &
        x86_video_sequencer_mask(adapter, 2u);
    adapter->data.sequencer[4] = adapter->data.ega_sequencer.memory_mode &
        x86_video_sequencer_mask(adapter, 4u);
}

static void x86_video_reset_ega_controllers(x86_video *adapter)
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

static void x86_video_normalize_ega_controllers(
    x86_video_ega_controller_config *config)
{
    lib_u8 index;

    if (config == LIB_NULL) return;
    for (index = 0u; index < X86_VIDEO_GRAPHICS_REGISTER_COUNT; ++index) {
        config->graphics[index] &= x86_video_graphics_mask(index);
    }
    for (index = 0u; index < X86_VIDEO_ATTRIBUTE_REGISTER_COUNT; ++index) {
        config->attribute[index] &= x86_video_attribute_mask(index);
    }
}

static void x86_video_ega_write_observer(x86_video *adapter,
    lib_u32 physical, lib_size bytes)
{
    lib_u64 write_end;
    lib_u64 aperture_end;

    /* This observer owns presentation freshness for both planar EGA and the
     * non-planar memory-backed EGA configuration.  CPU mapping still belongs
     * to the planar provider when present; requiring that provider here made
     * the non-planar path silently miss real writes. */
    if (adapter == LIB_NULL || !adapter->data.ega_sequencer_configured ||
        bytes == 0u || !x86_video_ega_aperture_contains(adapter,
            physical, bytes)) return;
    if (x86_video_compaq_b000_compatibility_contains(adapter, physical, bytes)) {
        x86_video_mark_dirty(adapter);
        return;
    }
    write_end = (lib_u64)physical + bytes;
    {
        lib_u32 aperture_base;
        lib_u32 aperture_bytes;

        if (!x86_video_active_ega_aperture(adapter, &aperture_base,
            &aperture_bytes)) return;
        aperture_end = (lib_u64)aperture_base + aperture_bytes;
        if ((lib_u64)physical < aperture_end &&
            (lib_u64)aperture_base < write_end) {
            x86_video_mark_dirty(adapter);
        }
    }
}

static lib_u32 x86_video_raster_period(
    const x86_video_text_timing *timing)
{
    return timing->active_display_ticks + timing->horizontal_blank_ticks +
        timing->vertical_retrace_ticks;
}

static lib_i32 x86_video_valid_text_timing(
    const x86_video_text_timing *timing)
{
    lib_u64 period;

    if (timing == LIB_NULL || timing->active_display_ticks == 0u ||
        timing->vertical_retrace_ticks == 0u) {
        return LIB_FALSE;
    }
    period = (lib_u64)timing->active_display_ticks +
        timing->horizontal_blank_ticks + timing->vertical_retrace_ticks;
    return period <= LIB_UINT32_MAX;
}

static lib_bool x86_video_ega_raster_geometry(const x86_video *adapter,
    lib_u32 *out_horizontal, lib_u32 *out_period, lib_u32 *out_blank)
{
    lib_u32 horizontal;
    lib_u32 vertical;
    lib_u32 blank;

    if (!adapter->data.ega_controller_configured || adapter->data.vga_configured ||
        adapter->data.sequencer[0] != 3u ||
        adapter->data.crtc[X86_VIDEO_CRTC_HORIZONTAL_TOTAL] == 0u ||
        adapter->data.crtc[X86_VIDEO_EGA_CRTC_VERTICAL_TOTAL] == 0u)
        return LIB_FALSE;
    horizontal = ((lib_u32)adapter->data.crtc[X86_VIDEO_CRTC_HORIZONTAL_TOTAL] + 2u) *
        x86_video_ega_horizontal_scale(adapter);
    /* EGA horizontal n-2 is sourced; vertical terminal count +2 is L2. */
    vertical = ((lib_u32)adapter->data.crtc[X86_VIDEO_EGA_CRTC_VERTICAL_TOTAL] |
        ((lib_u32)(adapter->data.crtc[X86_VIDEO_CRTC_OVERFLOW] & 1u) << 8u)) + 2u;
    blank = (lib_u32)adapter->data.crtc[X86_VIDEO_CRTC_START_VERTICAL_BLANK] |
        ((lib_u32)(adapter->data.crtc[X86_VIDEO_CRTC_OVERFLOW] & 8u) << 5u);
    if (blank >= vertical) return LIB_FALSE;
    *out_horizontal = horizontal;
    *out_period = horizontal * vertical * x86_video_ega_vertical_scale(adapter);
    *out_blank = horizontal * blank * x86_video_ega_vertical_scale(adapter);
    return LIB_TRUE;
}

lib_status x86_video_ticks_until_event(const x86_video *video, lib_u64 *out_ticks)
{
    lib_u32 horizontal;
    lib_u32 period;
    lib_u32 blank;
    lib_u32 phase;

    if (video == LIB_NULL || out_ticks == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if ((video->data.crtc[X86_VIDEO_CRTC_VERTICAL_RETRACE_END] & 0x30u) != 0x10u ||
        video->data.vertical_interrupt_pending ||
        !x86_video_ega_raster_geometry(video, &horizontal, &period, &blank))
        return LIB_STATUS_UNSUPPORTED;
    phase = video->data.raster_phase % period;
    *out_ticks = blank > phase ? blank - phase : period - phase + blank;
    return LIB_STATUS_OK;
}

static lib_u8 x86_video_status(const x86_video *adapter)
{
    lib_u32 vertical_end;
    lib_u32 display_end;
    lib_u8 status = 0u;

    if (adapter == LIB_NULL) return 0u;
    if (adapter->data.ega_personality == X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR &&
        (adapter->data.compaq_control_mode & 0x80u) != 0u)
        status |= X86_VIDEO_STATUS_DISPLAY_ENABLE;
    {
        lib_u32 horizontal;
        lib_u32 period;
        lib_u32 blank;

        if (x86_video_ega_raster_geometry(adapter, &horizontal, &period, &blank)) {
            const lib_u32 phase = adapter->data.raster_phase % period;
            const lib_u32 scanline = phase / horizontal / x86_video_ega_vertical_scale(adapter);
            const lib_u32 sync = (lib_u32)adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_RETRACE_START] |
                ((lib_u32)(adapter->data.crtc[X86_VIDEO_CRTC_OVERFLOW] & 4u) << 6u);
            lib_u32 width = (adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_RETRACE_END] - sync) & 0x0fu;

            if (width == 0u) width = 16u;
            if (scanline >= sync && scanline - sync < width)
                status |= X86_VIDEO_STATUS_VERTICAL_RETRACE;
            if (scanline < x86_video_ega_vertical_displayed(adapter) &&
                phase < blank && phase % horizontal <
                ((lib_u32)adapter->data.crtc[X86_VIDEO_CRTC_HORIZONTAL_DISPLAYED] + 1u) *
                    x86_video_ega_horizontal_scale(adapter))
                status |= X86_VIDEO_STATUS_DISPLAY_ENABLE;
            return status;
        }
    }
    if (x86_video_cga_logical_raster_active(adapter)) {
        lib_u32 horizontal_total = (lib_u32)adapter->data.crtc[
            X86_VIDEO_CRTC_HORIZONTAL_TOTAL] + 1u;
        lib_u32 period = x86_video_cga_logical_raster_period(adapter);
        lib_u32 scanline;
        lib_u32 character;
        lib_u32 phase;
        lib_u32 display_scanlines = (lib_u32)adapter->data.crtc[
            X86_VIDEO_CRTC_VERTICAL_DISPLAYED] *
            x86_video_cga_scanlines_per_row(adapter);
        lib_u32 vertical_sync_start = (lib_u32)adapter->data.crtc[
            X86_VIDEO_CRTC_VERTICAL_SYNC_POSITION] *
            x86_video_cga_scanlines_per_row(adapter);

        if (!adapter->data.cga_logical_raster_started || period == 0u) {
            return X86_VIDEO_STATUS_DISPLAY_ENABLE;
        }
        phase = (adapter->data.raster_phase % period) / x86_video_cga_clocks_per_character(adapter);
        scanline = phase / horizontal_total;
        character = phase % horizontal_total;
        if (scanline >= vertical_sync_start && scanline - vertical_sync_start < 16u) {
            status |= X86_VIDEO_STATUS_VERTICAL_RETRACE;
        }
        if (scanline >= display_scanlines || character >= adapter->data.crtc[
                X86_VIDEO_CRTC_HORIZONTAL_DISPLAYED]) {
            status |= X86_VIDEO_STATUS_DISPLAY_ENABLE;
        }
        return status;
    }
    vertical_end = adapter->data.text_timing.vertical_retrace_ticks;
    display_end = vertical_end + adapter->data.text_timing.active_display_ticks;
    if (adapter->data.raster_phase < vertical_end) {
        status |= X86_VIDEO_STATUS_VERTICAL_RETRACE;
    }
    /* CGA status bit 0 reports that buffer access can proceed without
     * display interference. EGA retains its existing display-enable view. */
    if (adapter->data.ega_controller_configured) {
        if (adapter->data.raster_phase >= vertical_end &&
            adapter->data.raster_phase < display_end) {
            status |= X86_VIDEO_STATUS_DISPLAY_ENABLE;
        }
    } else if (adapter->data.raster_phase < vertical_end ||
        adapter->data.raster_phase >= display_end) {
        status |= X86_VIDEO_STATUS_DISPLAY_ENABLE;
    }
    return status;
}

static lib_i32 x86_video_compaq_io_route_active(const x86_video *adapter,
    lib_u16 port_id, lib_u16 monochrome_port,
    lib_u16 color_port)
{
    if (adapter == LIB_NULL) return LIB_FALSE;
    if (adapter->data.ega_personality ==
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
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

static void x86_video_write_crtc_index(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    if (io_byte != LIB_NULL && adapter != LIB_NULL &&
        x86_video_compaq_io_route_active(adapter, port_id,
        X86_VIDEO_REGISTER_MONO_CRTC_INDEX,
        X86_VIDEO_REGISTER_COLOR_CRTC_INDEX)) {
        adapter->data.crtc_index = *io_byte & 0x1fu;
    }
}

static void x86_video_read_crtc_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    if (!x86_video_compaq_io_route_active(adapter, port_id,
        X86_VIDEO_REGISTER_MONO_CRTC_DATA,
        X86_VIDEO_REGISTER_COLOR_CRTC_DATA) ||
        !x86_video_crtc_index_readable(adapter, adapter->data.crtc_index)) {
        return;
    }
    *io_byte = adapter->data.crtc[adapter->data.crtc_index];
}

static void x86_video_write_crtc_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    if (io_byte == LIB_NULL || adapter == LIB_NULL ||
        !x86_video_compaq_io_route_active(adapter, port_id,
        X86_VIDEO_REGISTER_MONO_CRTC_DATA,
        X86_VIDEO_REGISTER_COLOR_CRTC_DATA) ||
        !x86_video_crtc_index_writable(adapter, adapter->data.crtc_index)) {
        return;
    }
    {
        lib_u8 value = *io_byte &
            x86_video_crtc_mask(adapter, adapter->data.crtc_index);

        if (adapter->data.ega_controller_configured &&
            adapter->data.crtc_index == X86_VIDEO_CRTC_VERTICAL_RETRACE_END &&
            (value & 0x10u) == 0u) adapter->data.vertical_interrupt_pending = LIB_FALSE;
        if (adapter->data.crtc[adapter->data.crtc_index] == value) return;
        adapter->data.crtc[adapter->data.crtc_index] = value;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_read_mode(lib_u8 *io_byte, lib_u16 port_id,
    void *owner)
{
    (void)port_id;
    if (io_byte != LIB_NULL && owner != LIB_NULL) {
        *io_byte = ((x86_video *)owner)->data.mode_control;
    }
}

static void x86_video_write_mode(lib_u8 *io_byte, lib_u16 port_id,
    void *owner)
{
    x86_video *adapter = (x86_video *)owner;
    lib_u8 value;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    value = *io_byte & 0x3fu;
    if (adapter->data.mode_control != value) {
        adapter->data.mode_control = value;
        adapter->data.columns = (value & 0x01u) != 0u ? 80u : 40u;
        adapter->data.color_enabled = (value & 0x04u) != 0u;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_read_color(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (io_byte != LIB_NULL && owner != LIB_NULL) {
        *io_byte = ((x86_video *)owner)->data.color_select;
    }
}

static void x86_video_write_color(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;
    lib_u8 value;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    value = *io_byte & 0x3fu;

    if (adapter->data.color_select != value) {
        adapter->data.color_select = value;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_write_cga_lightpen(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)io_byte;
    if (adapter == LIB_NULL) return;
    if (adapter->data.ega_personality ==
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        if (port_id == (adapter->data.compaq_color_io_base ?
            X86_VIDEO_REGISTER_COLOR_LIGHTPEN_RESET :
            X86_VIDEO_REGISTER_MONO_LIGHTPEN_RESET)) {
            adapter->data.compaq_lightpen_latched = LIB_FALSE;
        } else if (port_id == (adapter->data.compaq_color_io_base ?
            X86_VIDEO_REGISTER_COLOR_LIGHTPEN_SET :
            X86_VIDEO_REGISTER_MONO_LIGHTPEN_SET)) {
            adapter->data.compaq_lightpen_latched = LIB_TRUE;
        }
    } else if (!adapter->data.ega_controller_configured) {
        adapter->data.cga_lightpen_latched =
            port_id == X86_VIDEO_REGISTER_COLOR_LIGHTPEN_SET;
    }
}

static void x86_video_read_status(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    if (!x86_video_compaq_io_route_active(adapter, port_id,
        X86_VIDEO_REGISTER_MONO_STATUS, X86_VIDEO_REGISTER_COLOR_STATUS)) {
        return;
    }
    *io_byte = x86_video_status(adapter);
    if (adapter->data.ega_controller_configured && !x86_video_ega_output_active(adapter))
        *io_byte &= (lib_u8)~X86_VIDEO_STATUS_DISPLAY_ENABLE;
    if (!adapter->data.ega_controller_configured &&
        adapter->data.cga_lightpen_latched) {
        *io_byte |= X86_VIDEO_STATUS_LIGHTPEN_TRIGGER;
    }
    if (adapter->data.vga_configured) {
        if (!adapter->data.vga_status_diagnostic_high) {
            *io_byte |= 0x30u;
        }
        adapter->data.vga_status_diagnostic_high =
            !adapter->data.vga_status_diagnostic_high;
    } else if (adapter->data.ega_controller_configured) {
        *io_byte |= x86_video_ega_graphics_diagnostic(adapter);
    }
    if (adapter->data.ega_personality ==
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        if (adapter->data.compaq_lightpen_latched) {
            *io_byte |= X86_VIDEO_STATUS_LIGHTPEN_TRIGGER;
        }
        if (adapter->data.cecg.lightpen_switch_open) {
            *io_byte |= X86_VIDEO_STATUS_LIGHTPEN_SWITCH_OPEN;
        }
    }
    if (adapter->data.ega_controller_configured) {
        adapter->data.attribute_data_phase = LIB_FALSE;
    }
}

static void x86_video_read_compaq_control_mode(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (io_byte != LIB_NULL && owner != LIB_NULL) {
        *io_byte = ((const x86_video *)owner)->data.compaq_control_mode;
    }
}

static void x86_video_write_compaq_control_mode(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;
    lib_u8 value;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    value = *io_byte & 0xfbu;
    if (adapter->data.compaq_control_mode != value) {
        adapter->data.compaq_control_mode = value;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_write_compaq_miscellaneous_output(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    /* Compaq EGA miscellaneous-output bit 1 disables the CPU aperture.  The
     * clear state leaves the window decoded by the VADP provider. */
    adapter->data.compaq_cpu_video_memory_disabled =
        (*io_byte & 0x02u) != 0u;
    adapter->data.compaq_color_io_base = (*io_byte & 0x01u) != 0u;
    adapter->data.compaq_clock_switch_select = (*io_byte >> 2u) & 0x03u;
    if (adapter->data.compaq_odd_even_high_page !=
        ((*io_byte & 0x20u) != 0u)) {
        adapter->data.compaq_odd_even_high_page = (*io_byte & 0x20u) != 0u;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_read_compaq_input_status_0(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;
    lib_u8 selected_switch;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    selected_switch = 4u - adapter->data.compaq_clock_switch_select;
    *io_byte = (adapter->data.cecg.sw1_closed_mask &
        (1u << (selected_switch - 1u))) != 0u ? 0u : 0x10u;
    if (!adapter->data.cecg.special_features_present) {
        *io_byte |= 0x60u;
    }
    if ((adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_RETRACE_END] & 0x20u) != 0u ||
        adapter->data.vertical_interrupt_pending) {
        *io_byte |= 0x80u;
    }
}

static void x86_video_write_compaq_feature_control(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    if (io_byte == LIB_NULL || adapter == LIB_NULL ||
        !x86_video_compaq_io_route_active(adapter, port_id,
        X86_VIDEO_REGISTER_MONO_STATUS,
        X86_VIDEO_REGISTER_COLOR_STATUS)) return;
    adapter->data.compaq_feature_control = *io_byte & 0x03u;
}

static void x86_video_read_ega_input_status_0(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    *io_byte = adapter->data.vga_configured ?
        ((x86_video_status(adapter) & X86_VIDEO_STATUS_DISPLAY_ENABLE) != 0u ? 0x80u : 0u) :
        (adapter->data.vertical_interrupt_pending ? 0x80u : 0u);
}

static void x86_video_write_ega_miscellaneous_output(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    if (adapter->data.ega_miscellaneous_output != *io_byte) {
        adapter->data.ega_miscellaneous_output = *io_byte;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_write_ega_feature_control(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL && adapter != LIB_NULL) {
        adapter->data.ega_feature_control = *io_byte & 0x03u;
    }
}

static void x86_video_read_compaq_environment(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (io_byte != LIB_NULL && owner != LIB_NULL) {
        const x86_video *adapter = (const x86_video *)owner;

        *io_byte = (adapter->data.cecg.environment & 0x02u) | 0x0du;
    }
}

static void x86_video_read_compaq_display_type(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (io_byte != LIB_NULL && owner != LIB_NULL) {
        *io_byte = ((const x86_video *)owner)->data.cecg.display_type;
    }
}

static void x86_video_read_compaq_initial_mode(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    (void)port_id;
    if (io_byte != LIB_NULL && owner != LIB_NULL) {
        *io_byte = ((const x86_video *)owner)->data.cecg.initial_mode;
    }
}

static void x86_video_read_graphics_index(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL && adapter != LIB_NULL) {
        *io_byte = adapter->data.ega_controller_configured ?
            adapter->data.graphics_index : 0xffu;
    }
}

static void x86_video_write_graphics_index(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL && adapter != LIB_NULL &&
        adapter->data.ega_controller_configured) {
        adapter->data.graphics_index = *io_byte;
    }
}

static void x86_video_read_graphics_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL) {
        *io_byte = adapter != LIB_NULL && adapter->data.ega_controller_configured &&
            x86_video_graphics_index_supported(adapter->data.graphics_index) ?
            adapter->data.graphics[adapter->data.graphics_index] : 0u;
    }
}

static void x86_video_write_graphics_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;
    lib_u8 index;
    lib_u8 value;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL ||
        !adapter->data.ega_controller_configured) return;
    index = adapter->data.graphics_index;
    if (!x86_video_graphics_index_supported(index)) return;
    value = *io_byte & x86_video_graphics_mask(index);
    if (adapter->data.graphics[index] != value) {
        adapter->data.graphics[index] = value;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_read_attribute_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL) {
        *io_byte = adapter != LIB_NULL && adapter->data.ega_controller_configured &&
            x86_video_attribute_index_supported(adapter->data.attribute_index) ?
            adapter->data.attribute[adapter->data.attribute_index] : 0u;
    }
}

static void x86_video_write_attribute(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;
    lib_u8 value;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL ||
        !adapter->data.ega_controller_configured) return;
    value = *io_byte;
    if (!adapter->data.attribute_data_phase) {
        adapter->data.attribute_index = value & 0x1fu;
        if (adapter->data.attribute_display_enabled != ((value & 0x20u) != 0u)) {
            adapter->data.attribute_display_enabled = (value & 0x20u) != 0u;
            x86_video_mark_dirty(adapter);
        }
        adapter->data.attribute_data_phase = LIB_TRUE;
        return;
    }
    if (x86_video_attribute_index_supported(adapter->data.attribute_index)) {
        lib_u8 index = adapter->data.attribute_index;
        lib_u8 masked = value & x86_video_attribute_mask(index);

        if (adapter->data.attribute[index] != masked) {
            adapter->data.attribute[index] = masked;
            x86_video_mark_dirty(adapter);
        }
    }
    adapter->data.attribute_data_phase = LIB_FALSE;
}

static void x86_video_read_sequencer_index(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL && adapter != LIB_NULL) {
        *io_byte = adapter->data.ega_sequencer_configured ?
            adapter->data.sequencer_index : 0xffu;
    }
}

static void x86_video_write_sequencer_index(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL && adapter != LIB_NULL &&
        adapter->data.ega_sequencer_configured) {
        adapter->data.sequencer_index = *io_byte;
    }
}

static void x86_video_read_sequencer_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL) return;
    *io_byte = adapter->data.ega_sequencer_configured &&
        x86_video_sequencer_index_supported(adapter->data.sequencer_index) ?
        adapter->data.sequencer[adapter->data.sequencer_index] : 0xffu;
}

static void x86_video_write_sequencer_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;
    lib_u8 index;
    lib_u8 value;
    lib_bool clear_character_map;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL ||
        !adapter->data.ega_sequencer_configured) return;
    index = adapter->data.sequencer_index;
    if (!x86_video_sequencer_index_supported(index)) return;
    value = *io_byte & x86_video_sequencer_mask(adapter, index);
    clear_character_map = !adapter->data.vga_configured &&
        adapter->data.ega_personality == X86_VIDEO_EGA_PERSONALITY_GENERIC && index == 0u &&
        (value & 0x01u) == 0u && adapter->data.sequencer[3] != 0u;
    if (adapter->data.sequencer[index] != value || clear_character_map) {
        adapter->data.sequencer[index] = value;
        if (clear_character_map) adapter->data.sequencer[3] = 0u;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_read_vga_dac_mask(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL) {
        *io_byte = adapter != LIB_NULL && adapter->data.vga_configured ?
            adapter->data.vga_dac_mask : 0u;
    }
}

static void x86_video_write_vga_dac_mask(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    if (adapter->data.vga_dac_mask != *io_byte) {
        adapter->data.vga_dac_mask = *io_byte;
        x86_video_mark_dirty(adapter);
    }
}

static void x86_video_read_vga_dac_read_index(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    const x86_video *adapter = (const x86_video *)owner;

    (void)port_id;
    if (io_byte != LIB_NULL) {
        *io_byte = adapter != LIB_NULL && adapter->data.vga_configured ?
            adapter->data.vga_dac_read_index : 0u;
    }
}

static void x86_video_write_vga_dac_read_index(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    adapter->data.vga_dac_read_index = *io_byte;
    adapter->data.vga_dac_read_component = 0u;
}

static void x86_video_write_vga_dac_write_index(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    adapter->data.vga_dac_write_index = *io_byte;
    adapter->data.vga_dac_write_component = 0u;
}

static void x86_video_read_vga_dac_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    *io_byte = adapter->data.vga_dac[adapter->data.vga_dac_read_index]
        [adapter->data.vga_dac_read_component];
    if (++adapter->data.vga_dac_read_component == 3u) {
        adapter->data.vga_dac_read_component = 0u;
        ++adapter->data.vga_dac_read_index;
    }
}

static void x86_video_write_vga_dac_data(lib_u8 *io_byte,
    lib_u16 port_id, void *owner)
{
    x86_video *adapter = (x86_video *)owner;
    lib_u8 value;

    (void)port_id;
    if (io_byte == LIB_NULL || adapter == LIB_NULL || !adapter->data.vga_configured) return;
    value = *io_byte & 0x3fu;
    if (adapter->data.vga_dac[adapter->data.vga_dac_write_index]
        [adapter->data.vga_dac_write_component] != value) {
        adapter->data.vga_dac[adapter->data.vga_dac_write_index]
            [adapter->data.vga_dac_write_component] = value;
        x86_video_mark_dirty(adapter);
    }
    if (++adapter->data.vga_dac_write_component == 3u) {
        adapter->data.vga_dac_write_component = 0u;
        ++adapter->data.vga_dac_write_index;
    }
}

lib_status x86_video_register_read(x86_video *adapter,
    x86_video_register port_id, lib_u8 *out_value)
{
    lib_u8 value;

    if (adapter == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (adapter->data.ega_sequencer_configured && !adapter->data.vga_configured) {
        /* EGA/CECG indexed controls are write-only; the board leaves those
         * read directions unregistered instead of driving an idle byte. */
        switch (port_id) {
        case X86_VIDEO_REGISTER_ATTRIBUTE:
        case X86_VIDEO_REGISTER_ATTRIBUTE_DATA:
        case X86_VIDEO_REGISTER_SEQUENCER_INDEX:
        case X86_VIDEO_REGISTER_SEQUENCER_DATA:
        case X86_VIDEO_REGISTER_GRAPHICS_INDEX:
        case X86_VIDEO_REGISTER_GRAPHICS_DATA:
            return LIB_STATUS_UNSUPPORTED;
        default: break;
        }
    }
    value = *out_value;
    switch (port_id) {
    case X86_VIDEO_REGISTER_MONO_CRTC_DATA:
        x86_video_read_crtc_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_MONO_STATUS:
        x86_video_read_status(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_ATTRIBUTE_DATA:
        x86_video_read_attribute_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_EXTERNAL_CONTROL:
        if (adapter->data.ega_personality ==
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
            x86_video_read_compaq_input_status_0(&value, port_id, adapter);
        } else {
            x86_video_read_ega_input_status_0(&value, port_id, adapter);
        }
        break;
    case X86_VIDEO_REGISTER_SEQUENCER_INDEX:
        x86_video_read_sequencer_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_SEQUENCER_DATA:
        x86_video_read_sequencer_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_AUXILIARY_CONTROL:
        if (adapter->data.vga_configured) {
            x86_video_read_vga_dac_mask(&value, port_id, adapter);
        } else {
            x86_video_read_compaq_control_mode(&value, port_id, adapter);
        }
        break;
    case X86_VIDEO_REGISTER_DAC_READ_INDEX:
        x86_video_read_vga_dac_read_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_DAC_DATA:
        x86_video_read_vga_dac_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_GRAPHICS_INDEX:
        x86_video_read_graphics_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_GRAPHICS_DATA:
        x86_video_read_graphics_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_COLOR_CRTC_DATA:
        x86_video_read_crtc_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_MODE:
        x86_video_read_mode(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_COLOR:
        x86_video_read_color(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_COLOR_STATUS:
        x86_video_read_status(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_ENVIRONMENT:
        x86_video_read_compaq_environment(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_DISPLAY_TYPE:
        x86_video_read_compaq_display_type(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_INITIAL_MODE:
        x86_video_read_compaq_initial_mode(&value, port_id, adapter);
        break;
    default: return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_value = value;
    return LIB_STATUS_OK;
}

lib_status x86_video_register_write(x86_video *adapter,
    x86_video_register port_id, lib_u8 value)
{
    if (adapter == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (port_id) {
    case X86_VIDEO_REGISTER_MONO_CRTC_INDEX:
        x86_video_write_crtc_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_MONO_CRTC_DATA:
        x86_video_write_crtc_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_MONO_STATUS:
        if (adapter->data.ega_personality ==
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
            x86_video_write_compaq_feature_control(&value, port_id, adapter);
        } else {
            x86_video_write_ega_feature_control(&value, port_id, adapter);
        }
        break;
    case X86_VIDEO_REGISTER_MONO_LIGHTPEN_RESET:
        x86_video_write_cga_lightpen(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_MONO_LIGHTPEN_SET:
        x86_video_write_cga_lightpen(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_ATTRIBUTE:
        x86_video_write_attribute(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_EXTERNAL_CONTROL:
        if (adapter->data.ega_personality ==
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
            x86_video_write_compaq_miscellaneous_output(&value, port_id, adapter);
        } else {
            x86_video_write_ega_miscellaneous_output(&value, port_id, adapter);
        }
        break;
    case X86_VIDEO_REGISTER_SEQUENCER_INDEX:
        x86_video_write_sequencer_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_SEQUENCER_DATA:
        x86_video_write_sequencer_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_AUXILIARY_CONTROL:
        if (adapter->data.vga_configured) {
            x86_video_write_vga_dac_mask(&value, port_id, adapter);
        } else {
            x86_video_write_compaq_control_mode(&value, port_id, adapter);
        }
        break;
    case X86_VIDEO_REGISTER_DAC_READ_INDEX:
        x86_video_write_vga_dac_read_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_DAC_WRITE_INDEX:
        x86_video_write_vga_dac_write_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_DAC_DATA:
        x86_video_write_vga_dac_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_GRAPHICS_INDEX:
        x86_video_write_graphics_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_GRAPHICS_DATA:
        x86_video_write_graphics_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_COLOR_CRTC_INDEX:
        x86_video_write_crtc_index(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_COLOR_CRTC_DATA:
        x86_video_write_crtc_data(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_MODE:
        x86_video_write_mode(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_COLOR:
        x86_video_write_color(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_COLOR_STATUS:
        if (adapter->data.ega_personality ==
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
            x86_video_write_compaq_feature_control(&value, port_id, adapter);
        } else {
            x86_video_write_ega_feature_control(&value, port_id, adapter);
        }
        break;
    case X86_VIDEO_REGISTER_COLOR_LIGHTPEN_RESET:
        x86_video_write_cga_lightpen(&value, port_id, adapter);
        break;
    case X86_VIDEO_REGISTER_COLOR_LIGHTPEN_SET:
        x86_video_write_cga_lightpen(&value, port_id, adapter);
        break;
    default: return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}





void x86_video_set_allocate_zero(x86_video *adapter,
    x86_video_allocate_zero callback, void *context)
{
    if (adapter == LIB_NULL) return;
    adapter->data.allocate_zero = callback;
    adapter->data.allocate_context = context;
}



lib_status x86_video_configure_ega_personality(x86_video *adapter,
    x86_video_ega_personality personality)
{
    if (adapter == LIB_NULL ||
        adapter->data.ega_personality != X86_VIDEO_EGA_PERSONALITY_GENERIC ||
        (personality != X86_VIDEO_EGA_PERSONALITY_GENERIC &&
        personality != X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    adapter->data.ega_external_configured = personality == X86_VIDEO_EGA_PERSONALITY_GENERIC;
    adapter->data.ega_personality = personality;
    if (personality == X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        adapter->data.cecg = (x86_video_cecg_config) {
            0x41u, 0x0du, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
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

lib_status x86_video_configure_vga(x86_video *adapter)
{
    if (adapter == LIB_NULL || adapter->data.vga_configured ||
        adapter->data.ega_personality != X86_VIDEO_EGA_PERSONALITY_GENERIC ||
        !adapter->data.ega_sequencer_configured ||
        !adapter->data.ega_controller_configured) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    adapter->data.vga_configured = LIB_TRUE;
    adapter->data.vga_dac_mask = 0xffu;
    /* Capability selection precedes runtime; replay the retained construction
     * values with VGA masks rather than retaining EGA-filtered defaults. */
    x86_video_reset_sequencer(adapter);
    return LIB_STATUS_OK;
}

lib_i32 x86_video_cecg_config_is_valid(
    const x86_video_cecg_config *config)
{
    return config != LIB_NULL && (config->control_mode & 0x07u) == 0x01u &&
        (config->display_type & 0x44u) == 0u && config->initial_mode == 0x01u &&
        (config->sw1_closed_mask & 0xf0u) == 0u && config->clock_switch_select <= 3u;
}

lib_status x86_video_configure_cecg(x86_video *adapter,
    const x86_video_cecg_config *config)
{
    if (adapter == LIB_NULL ||
        adapter->data.ega_personality !=
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR ||
        !x86_video_cecg_config_is_valid(config)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    adapter->data.cecg = *config;
    adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_RETRACE_END] =
        config->vertical_retrace_irq_enabled ? 0x10u : 0x20u;
    adapter->data.compaq_control_mode = config->control_mode;
    adapter->data.compaq_cpu_video_memory_disabled =
        config->cpu_video_memory_disabled;
    adapter->data.compaq_color_io_base = config->color_io_base;
    adapter->data.compaq_clock_switch_select = config->clock_switch_select;
    adapter->data.compaq_odd_even_high_page = config->odd_even_high_page;
    return LIB_STATUS_OK;
}

void x86_video_reset(x86_video *adapter)
{
    x86_video_text_timing timing;
    x86_video_ega_personality ega_personality;
    x86_video_cecg_config cecg;
    x86_video_ega_sequencer_config ega_sequencer;
    x86_video_ega_controller_config ega_controller;
    lib_u8 crtc[X86_VIDEO_CRTC_REGISTER_COUNT];
    lib_u8 crtc_initialized;
    lib_u8 ega_sequencer_configured;
    lib_u8 ega_controller_configured;
    lib_u8 ega_external_configured;
    lib_u8 ega_planar_enabled;
    lib_u8 vga_configured;
    lib_u8 cga_memory_configured;
    lib_u8 *ega_planar_vram;
    x86_video_text_glyph_config text_glyphs;

    if (adapter == LIB_NULL) return;
    timing = adapter->data.text_timing;
    if (!x86_video_valid_text_timing(&timing)) {
        timing.active_display_ticks = X86_VIDEO_DEFAULT_ACTIVE_DISPLAY_TICKS;
        timing.horizontal_blank_ticks = X86_VIDEO_DEFAULT_HORIZONTAL_BLANK_TICKS;
        timing.vertical_retrace_ticks = X86_VIDEO_DEFAULT_VERTICAL_RETRACE_TICKS;
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
    if (ega_planar_vram != LIB_NULL) {
        lib_memory_set(ega_planar_vram, 0,
            X86_VIDEO_EGA_PLANES * X86_VIDEO_EGA_PLANE_BYTES);
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
        adapter->data.crtc[X86_VIDEO_CRTC_CURSOR_TOP] = 6u;
        adapter->data.crtc[X86_VIDEO_CRTC_CURSOR_BOTTOM] = 7u;
    }
    adapter->data.ega_personality = ega_personality;
    adapter->data.cecg = cecg;
    adapter->data.compaq_control_mode = cecg.control_mode;
    adapter->data.compaq_cpu_video_memory_disabled =
        cecg.cpu_video_memory_disabled;
    adapter->data.compaq_color_io_base = cecg.color_io_base;
    adapter->data.compaq_clock_switch_select = cecg.clock_switch_select;
    adapter->data.compaq_odd_even_high_page = cecg.odd_even_high_page;
    adapter->data.ega_sequencer = ega_sequencer;
    adapter->data.ega_sequencer_configured = ega_sequencer_configured;
    adapter->data.ega_external_configured = ega_external_configured;
    adapter->data.vga_configured = vga_configured;
    x86_video_reset_sequencer(adapter);
    adapter->data.ega_controller = ega_controller;
    adapter->data.ega_controller_configured = ega_controller_configured;
    if (ega_controller_configured || ega_personality ==
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR)
        adapter->data.crtc[X86_VIDEO_CRTC_VERTICAL_RETRACE_END] =
            ega_personality == X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR &&
            cecg.vertical_retrace_irq_enabled ? 0x10u : 0x20u;
    x86_video_reset_ega_controllers(adapter);
    adapter->data.ega_planar_enabled = ega_planar_enabled;
    adapter->data.cga_memory_configured = cga_memory_configured;
    if (vga_configured) adapter->data.vga_dac_mask = 0xffu;
    adapter->data.ega_planar_vram = ega_planar_vram;
    if (x86_video_cga_logical_raster_active(adapter)) {
        adapter->data.raster_phase = 0u;
        adapter->data.cga_logical_raster_started = LIB_FALSE;
    }
    adapter->data.dirty_generation = 1u;
}

lib_status x86_video_configure_text_glyphs(x86_video *adapter,
    const x86_video_text_glyph_config *config)
{
    if (adapter == LIB_NULL || config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    adapter->data.text_glyphs = *config;
    return LIB_STATUS_OK;
}

static void x86_video_advance_raster(x86_video *adapter,
    lib_u64 elapsed_ticks, lib_u32 period)
{
    const lib_u32 phase = adapter->data.raster_phase % period;
    const lib_u32 remainder = (lib_u32)(elapsed_ticks % period);

    if (!adapter->data.ega_controller_configured) {
        /* Count complete fields modulo the longest MC6845 cursor cycle.
         * Reduce before adding so even LIB_UINT64_MAX ticks remain bounded. */
        const lib_u8 fields = (lib_u8)((elapsed_ticks / period) % 32u +
            (remainder >= period - phase ? 1u : 0u));
        const lib_u8 previous = adapter->data.cga_field_phase;
        const lib_u8 mode = (lib_u8)((adapter->data.crtc[
            X86_VIDEO_CRTC_CURSOR_TOP] >> 5u) & 3u);

        adapter->data.cga_field_phase = (lib_u8)(
            (adapter->data.cga_field_phase + fields) & 31u);
        if (!x86_video_is_graphics_mode(adapter) &&
            (adapter->data.mode_control & X86_VIDEO_MODE_VIDEO_ENABLE) != 0u &&
            ((mode >= 2u && ((previous ^ adapter->data.cga_field_phase) &
                (mode == 2u ? 8u : 16u)) != 0u) ||
            ((adapter->data.mode_control & X86_VIDEO_MODE_BLINK) != 0u &&
                ((previous ^ adapter->data.cga_field_phase) & 8u) != 0u)))
            x86_video_mark_dirty(adapter);
    }
    adapter->data.raster_phase = remainder >= period - phase ?
        remainder - (period - phase) : phase + remainder;
}

void x86_video_advance(x86_video *adapter, lib_u64 elapsed_ticks)
{
    lib_u32 period;

    if (adapter == LIB_NULL) return;
    {
        lib_u32 horizontal;
        lib_u32 blank;

        if (adapter->data.ega_controller_configured && !adapter->data.vga_configured &&
            adapter->data.sequencer[0] != 3u) return;
        if (x86_video_ega_raster_geometry(adapter, &horizontal, &period, &blank)) {
            lib_u64 due;
            const lib_u32 phase = adapter->data.raster_phase % period;

            if (x86_video_ticks_until_event(adapter, &due) == LIB_STATUS_OK &&
                elapsed_ticks >= due) adapter->data.vertical_interrupt_pending = LIB_TRUE;
            adapter->data.raster_phase = (lib_u32)((phase + elapsed_ticks % period) % period);
            return;
        }
    }
    if (x86_video_cga_logical_raster_active(adapter)) {
        lib_u32 phase;

        period = x86_video_cga_logical_raster_period(adapter);
        if (period == 0u) return;
        phase = adapter->data.raster_phase;
        if (phase >= period) phase %= period;
        if (!adapter->data.cga_logical_raster_started && elapsed_ticks >=
            period - phase) {
            adapter->data.cga_logical_raster_started = LIB_TRUE;
        }
        x86_video_advance_raster(adapter, elapsed_ticks, period);
        return;
    }
    period = x86_video_raster_period(&adapter->data.text_timing);
    if (period == 0u) return;
    x86_video_advance_raster(adapter, elapsed_ticks, period);
}

void x86_video_finalize(x86_video *adapter)
{
    if (adapter == LIB_NULL) return;
    lib_release(adapter->data.ega_planar_vram);
    adapter->data.ega_planar_vram = LIB_NULL;
}

lib_status x86_video_configure_text_timing(x86_video *adapter,
    const x86_video_text_timing *timing)
{
    if (adapter == LIB_NULL || !x86_video_valid_text_timing(timing)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    adapter->data.text_timing = *timing;
    adapter->data.raster_phase = timing->vertical_retrace_ticks;
    return LIB_STATUS_OK;
}

lib_status x86_video_configure_cga_memory(x86_video *adapter)
{
    if (adapter == LIB_NULL || adapter->data.cga_memory_configured)
        return LIB_STATUS_INVALID_ARGUMENT;
    adapter->data.cga_memory_configured = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status x86_video_configure_ega_sequencer(x86_video *adapter,
    const x86_video_ega_sequencer_config *config)
{
    lib_u8 *planar_vram = LIB_NULL;

    if (adapter == LIB_NULL || config == LIB_NULL ||
        config->aperture_base != X86_VIDEO_EGA_APERTURE_BASE ||
        config->aperture_bytes != X86_VIDEO_EGA_APERTURE_BYTES)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (adapter->data.ega_sequencer_configured) return LIB_STATUS_INVALID_STATE;
    if (config->planar_ega) {
        planar_vram = (lib_u8 *)(adapter->data.allocate_zero == LIB_NULL ?
            lib_allocate_zero(1u, X86_VIDEO_EGA_PLANES * X86_VIDEO_EGA_PLANE_BYTES) :
            adapter->data.allocate_zero(adapter->data.allocate_context, 1u,
                X86_VIDEO_EGA_PLANES * X86_VIDEO_EGA_PLANE_BYTES));
        if (planar_vram == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    }
    adapter->data.ega_planar_vram = planar_vram;
    adapter->data.ega_sequencer = *config;
    adapter->data.ega_sequencer_configured = LIB_TRUE;
    adapter->data.ega_planar_enabled = config->planar_ega;
    x86_video_reset_sequencer(adapter);
    return LIB_STATUS_OK;
}

lib_status x86_video_configure_ega_controllers(x86_video *adapter,
    const x86_video_ega_controller_config *config)
{
    if (adapter == LIB_NULL || config == LIB_NULL ||
        !adapter->data.ega_sequencer_configured) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (adapter->data.ega_controller_configured) return LIB_STATUS_INVALID_STATE;
    adapter->data.ega_controller = *config;
    x86_video_normalize_ega_controllers(&adapter->data.ega_controller);
    adapter->data.ega_controller_configured = LIB_TRUE;
    x86_video_reset_ega_controllers(adapter);
    return LIB_STATUS_OK;
}

lib_i32 x86_video_ega_aperture_contains(const x86_video *adapter,
    lib_u32 physical, lib_size bytes)
{
    if (adapter == LIB_NULL || !adapter->data.ega_sequencer_configured ||
        bytes == 0u) return LIB_FALSE;
    {
        lib_u32 aperture_base;
        lib_u32 aperture_bytes;

        if (!x86_video_active_ega_aperture(adapter, &aperture_base,
            &aperture_bytes)) return LIB_FALSE;
        return x86_video_memory_range_contains(aperture_base, aperture_bytes,
            physical, bytes);
    }
}

static lib_status x86_video_capture_text_snapshot_from(x86_video *adapter, const x86_video_memory_reader *memory,
    x86_video_snapshot *out_snapshot)
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
    lib_u8 cells[X86_VIDEO_MAX_COLUMNS *
        X86_VIDEO_MAX_ROWS * 2u];
    lib_i32 buffer_changed = LIB_FALSE;
    lib_i32 cursor_changed;
    lib_i32 cursor_visible;
    lib_bool enhanced_text;
    lib_bool output_active;
    lib_status status;

    if (memory == LIB_NULL || memory->read == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    enhanced_text = adapter->data.ega_controller_configured && !adapter->data.vga_configured;
    output_active = adapter->data.ega_controller_configured ?
        x86_video_ega_output_active(adapter) :
        (adapter->data.mode_control & X86_VIDEO_MODE_VIDEO_ENABLE) != 0u;
    columns = x86_video_text_columns(adapter);
    rows = x86_video_text_rows(adapter);
    if (columns == 0u || rows == 0u) return LIB_STATUS_UNSUPPORTED;
    start = x86_video_crtc_word(adapter, X86_VIDEO_CRTC_START_HIGH);
    cursor = x86_video_crtc_word(adapter, X86_VIDEO_CRTC_CURSOR_HIGH);
    visible_bytes = (lib_size)columns * rows * 2u;
    start_byte = (lib_u16)((start % (X86_VIDEO_TEXT_BYTES / 2u)) * 2u);
    first_bytes = X86_VIDEO_TEXT_BYTES - start_byte;
    if (first_bytes > visible_bytes) first_bytes = visible_bytes;
    status = memory->read(memory->context, X86_VIDEO_TEXT_BASE + start_byte,
        cells, first_bytes);
    if (status != LIB_STATUS_OK) return status;
    if (first_bytes < visible_bytes) {
        status = memory->read(memory->context, X86_VIDEO_TEXT_BASE,
            cells + first_bytes, visible_bytes - first_bytes);
        if (status != LIB_STATUS_OK) return status;
    }
    if (!output_active) {
        for (row = 0u; row < visible_bytes; row += 2u) {
            cells[row] = 0x20u;
            cells[row + 1u] = 0u;
        }
    }
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = X86_VIDEO_KIND_TEXT;
    out_snapshot->columns = columns;
    out_snapshot->rows = rows;
    out_snapshot->text_cell_height = (lib_u8)(adapter->data.crtc[
        X86_VIDEO_CRTC_MAXIMUM_RASTER_ADDRESS] + 1u);
    for (column = 0u; column < X86_VIDEO_PALETTE_ENTRIES;
        ++column) {
        if (enhanced_text) {
            out_snapshot->palette_rgb[column] = output_active ?
                x86_video_ega_palette_color(adapter,
                    adapter->data.attribute[column & adapter->data.attribute[18] & 0x0fu]) : 0u;
        } else {
            out_snapshot->palette_rgb[column] = x86_video_rgbi_color((lib_u8)column);
        }
    }
    out_snapshot->cursor_top = adapter->data.crtc[
        X86_VIDEO_CRTC_CURSOR_TOP] & 0x1fu;
    out_snapshot->cursor_bottom = adapter->data.crtc[
        X86_VIDEO_CRTC_CURSOR_BOTTOM] & 0x1fu;
    relative_cursor = (lib_u16)((cursor - start) %
        (X86_VIDEO_TEXT_BYTES / 2u));
    cursor_visible = output_active && relative_cursor < columns * rows;
    if (adapter->data.ega_personality ==
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
        lib_u8 end = out_snapshot->cursor_bottom;

        /* CECG encodes the first scanline after the cursor, with zero
         * extending a nonzero start to the cell bottom (guide 4-29). */
        if (end == 0u && out_snapshot->cursor_top != 0u)
            end = out_snapshot->text_cell_height;
        if (end > out_snapshot->text_cell_height)
            end = out_snapshot->text_cell_height;
        cursor_visible &= out_snapshot->cursor_top < end;
        out_snapshot->cursor_bottom = end != 0u ? (lib_u8)(end - 1u) : 0u;
    } else if (!adapter->data.ega_controller_configured) {
        const lib_u8 mode = (lib_u8)((adapter->data.crtc[
            X86_VIDEO_CRTC_CURSOR_TOP] >> 5u) & 3u);

        cursor_visible &= out_snapshot->cursor_top <= out_snapshot->cursor_bottom &&
            mode != 1u && (mode < 2u || (adapter->data.cga_field_phase &
                (mode == 2u ? 8u : 16u)) == 0u);
    } else if (!adapter->data.vga_configured) {
        /* IBM EGA encodes one less than the first cursor scanline (33). */
        ++out_snapshot->cursor_top;
        cursor_visible &= out_snapshot->cursor_top < out_snapshot->text_cell_height &&
            out_snapshot->cursor_top <= out_snapshot->cursor_bottom;
    } else {
        cursor_visible &= (adapter->data.crtc[X86_VIDEO_CRTC_CURSOR_TOP] &
            0x20u) == 0u && out_snapshot->cursor_top <= out_snapshot->cursor_bottom;
    }
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
        X86_VIDEO_KIND_TEXT ||
        adapter->data.captured_generation != adapter->data.dirty_generation ||
        adapter->data.captured_columns != columns ||
        adapter->data.captured_rows != rows ||
        adapter->data.captured_text_cell_height != out_snapshot->text_cell_height ||
        lib_memory_compare(adapter->data.text_cells, cells, visible_bytes) != 0;
    if (buffer_changed) {
        lib_memory_copy(adapter->data.text_cells, cells, visible_bytes);
        for (row = 0u; row < rows; ++row) {
            for (column = 0u; column < columns; ++column) {
                lib_u16 index = (lib_u16)(row * X86_VIDEO_MAX_COLUMNS + column);
                lib_u16 cell = (lib_u16)(row * columns + column);
                adapter->data.characters[index] = cells[(lib_size)cell * 2u];
                adapter->data.attributes[index] = cells[(lib_size)cell * 2u + 1u];
                if (!adapter->data.ega_controller_configured &&
                    (adapter->data.mode_control & X86_VIDEO_MODE_BLINK) != 0u) {
                    const lib_u8 attribute = adapter->data.attributes[index];

                    adapter->data.attributes[index] &= 0x7fu;
                    if ((attribute & 0x80u) != 0u &&
                        (adapter->data.cga_field_phase & 8u) != 0u)
                        adapter->data.attributes[index] = (lib_u8)(
                            (attribute & 0x70u) | ((attribute >> 4u) & 7u));
                }
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
    if (buffer_changed || cursor_changed) x86_video_mark_dirty(adapter);
    adapter->data.captured_generation = adapter->data.dirty_generation;
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
    adapter->data.captured_kind = X86_VIDEO_KIND_TEXT;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = cursor_changed;
    return LIB_STATUS_OK;
}

void x86_video_observe_snapshot(const x86_video *adapter,
    lib_u8 acknowledged_generation_valid,
    lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation)
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
        !x86_video_ega_output_active(adapter)) ||
        x86_video_vga_mode13_active(adapter) ||
        x86_video_ega_planar_display_active(adapter);
    /* Enhanced text still borrows system backing until live planar text is
     * qualified. Graphics-shaped CRTC values do not confer its ownership. */
    if (adapter->data.ega_controller_configured && !adapter->data.vga_configured &&
        (adapter->data.graphics[6] & 1u) == 0u && !adapter->data.cga_memory_configured)
        reliable = LIB_FALSE;
    if (!reliable) {
        out_observation->capture_required = LIB_TRUE;
        return;
    }
    out_observation->generation = adapter->data.dirty_generation;
    out_observation->generation_reliable = LIB_TRUE;
    out_observation->capture_required = !acknowledged_generation_valid ||
        acknowledged_generation != out_observation->generation;
}

static lib_status x86_video_capture_graphics_snapshot(x86_video *adapter,
    const x86_video_memory_reader *memory, x86_video_snapshot *out_snapshot)
{
    lib_u8 bytes[X86_VIDEO_VIDEO_BYTES];
    const lib_u32 start_byte = ((lib_u32)x86_video_crtc_word(adapter,
        X86_VIDEO_CRTC_START_HIGH) * 2u) & (X86_VIDEO_GRAPHICS_ODD_ROW_OFFSET - 1u);
    lib_u16 y;
    lib_u16 x;
    lib_i32 buffer_changed;
    lib_status status;

    if (memory == LIB_NULL || memory->read == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = memory->read(memory->context, X86_VIDEO_VIDEO_BASE, bytes, sizeof(bytes));
    if (status != LIB_STATUS_OK) return status;
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind !=
        X86_VIDEO_KIND_CGA_320X200X4 ||
        adapter->data.captured_generation != adapter->data.dirty_generation ||
        adapter->data.captured_mode_control != adapter->data.mode_control ||
        adapter->data.captured_color_select != adapter->data.color_select || lib_memory_compare(
        adapter->data.graphics_bytes, bytes, sizeof(bytes)) != 0;
    if (buffer_changed) {
        lib_memory_copy(adapter->data.graphics_bytes, bytes, sizeof(bytes));
    }
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = X86_VIDEO_KIND_CGA_320X200X4;
    out_snapshot->pixel_width = X86_VIDEO_GRAPHICS_WIDTH;
    out_snapshot->pixel_height = X86_VIDEO_GRAPHICS_HEIGHT;
    x86_video_graphics_palette(adapter, out_snapshot->palette_rgb);
    for (y = 0u; y < X86_VIDEO_GRAPHICS_HEIGHT; ++y) {
        const lib_u32 bank = (lib_u32)(y & 1u) * X86_VIDEO_GRAPHICS_ODD_ROW_OFFSET;
        const lib_u32 row_offset = start_byte + (lib_u32)(y >> 1) *
            X86_VIDEO_GRAPHICS_BYTES_PER_ROW;
        for (x = 0u; x < X86_VIDEO_GRAPHICS_WIDTH; ++x) {
            lib_u8 byte = bytes[bank + ((row_offset + (x >> 2)) &
                (X86_VIDEO_GRAPHICS_ODD_ROW_OFFSET - 1u))];
            out_snapshot->pixels[(lib_u32)y * X86_VIDEO_GRAPHICS_WIDTH + x] =
                (lib_u8)((byte >> (6u - 2u * (x & 3u))) & 0x03u);
        }
    }
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = X86_VIDEO_KIND_CGA_320X200X4;
    adapter->data.captured_generation = adapter->data.dirty_generation;
    adapter->data.captured_mode_control = adapter->data.mode_control;
    adapter->data.captured_color_select = adapter->data.color_select;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_status x86_video_capture_high_res_graphics_snapshot(x86_video *adapter,
    const x86_video_memory_reader *memory, x86_video_snapshot *out_snapshot)
{
    lib_u8 bytes[X86_VIDEO_VIDEO_BYTES];
    const lib_u32 start_byte = ((lib_u32)x86_video_crtc_word(adapter,
        X86_VIDEO_CRTC_START_HIGH) * 2u) & (X86_VIDEO_GRAPHICS_ODD_ROW_OFFSET - 1u);
    lib_u16 y;
    lib_u16 x;
    lib_i32 buffer_changed;
    lib_status status;

    if (memory == LIB_NULL || memory->read == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = memory->read(memory->context, X86_VIDEO_VIDEO_BASE, bytes, sizeof(bytes));
    if (status != LIB_STATUS_OK) return status;
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind !=
        X86_VIDEO_KIND_CGA_640X200X2 ||
        adapter->data.captured_generation != adapter->data.dirty_generation ||
        adapter->data.captured_mode_control != adapter->data.mode_control ||
        adapter->data.captured_color_select != adapter->data.color_select ||
        lib_memory_compare(adapter->data.graphics_bytes, bytes, sizeof(bytes)) != 0;
    if (buffer_changed) lib_memory_copy(adapter->data.graphics_bytes, bytes, sizeof(bytes));
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = X86_VIDEO_KIND_CGA_640X200X2;
    out_snapshot->pixel_width = X86_VIDEO_CGA_HIGH_RES_WIDTH;
    out_snapshot->pixel_height = X86_VIDEO_GRAPHICS_HEIGHT;
    x86_video_high_res_palette(adapter, out_snapshot->palette_rgb);
    for (y = 0u; y < X86_VIDEO_GRAPHICS_HEIGHT; ++y) {
        const lib_u32 bank = (lib_u32)(y & 1u) * X86_VIDEO_GRAPHICS_ODD_ROW_OFFSET;
        const lib_u32 row_offset = start_byte + (lib_u32)(y >> 1) *
            X86_VIDEO_GRAPHICS_BYTES_PER_ROW;

        for (x = 0u; x < X86_VIDEO_CGA_HIGH_RES_WIDTH; ++x) {
            lib_u8 byte = bytes[bank + ((row_offset + (x >> 3u)) &
                (X86_VIDEO_GRAPHICS_ODD_ROW_OFFSET - 1u))];

            out_snapshot->pixels[(lib_u32)y * X86_VIDEO_CGA_HIGH_RES_WIDTH + x] =
                (lib_u8)((byte >> (7u - (x & 7u))) & 0x01u);
        }
    }
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = X86_VIDEO_KIND_CGA_640X200X2;
    adapter->data.captured_generation = adapter->data.dirty_generation;
    adapter->data.captured_mode_control = adapter->data.mode_control;
    adapter->data.captured_color_select = adapter->data.color_select;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_u32 x86_video_ega_display_address(const x86_video *adapter,
    lib_u16 address, lib_u8 row_scan)
{
    const lib_u8 mode = adapter->data.crtc[X86_VIDEO_CRTC_MODE_CONTROL];
    lib_u32 offset = address;

    if ((mode & 0x40u) == 0u) offset = ((offset << 1u) & 0xffffu) |
        ((address >> ((mode & 0x20u) != 0u ? 15u : 13u)) & 1u);
    if ((mode & 1u) == 0u) offset = (offset & ~0x2000u) | ((lib_u32)(row_scan & 1u) << 13u);
    if ((mode & 2u) == 0u) offset = (offset & ~0x4000u) | ((lib_u32)(row_scan & 2u) << 13u);
    return offset;
}

static lib_u32 x86_video_ega_graphics_offset(const x86_video *adapter,
    lib_u16 x, lib_u16 y)
{
    const lib_u32 scanline = y / x86_video_ega_vertical_scale(adapter);
    const lib_u32 compare = adapter->data.crtc[X86_VIDEO_CRTC_LINE_COMPARE] |
        ((lib_u32)(adapter->data.crtc[X86_VIDEO_CRTC_OVERFLOW] & 0x10u) << 4u);
    const lib_u32 height = adapter->data.crtc[X86_VIDEO_CRTC_MAXIMUM_RASTER_ADDRESS] + 1u;
    const lib_u32 raster = scanline > compare ? scanline - compare - 1u :
        scanline + adapter->data.crtc[X86_VIDEO_CRT_INTERLACE_SKEW];
    const lib_u32 start = scanline > compare ? 0u :
        x86_video_crtc_word(adapter, X86_VIDEO_CRTC_START_HIGH);
    const lib_u32 column = ((lib_u32)x + adapter->data.attribute[19]) >> 3u;
    const lib_u32 address = start + (raster / height) *
        adapter->data.crtc[X86_VIDEO_CRTC_OFFSET] * 2u +
        (column >> ((adapter->data.crtc[X86_VIDEO_CRTC_MODE_CONTROL] & 8u) != 0u ? 1u : 0u));

    return x86_video_ega_display_address(adapter, (lib_u16)address,
        (lib_u8)(raster % height));
}

static lib_u8 x86_video_ega_plane_pixel(const lib_u8 samples[X86_VIDEO_EGA_PLANES], lib_u8 bit)
{
    lib_u8 pixel = 0u;

    for (lib_u8 plane = 0u; plane < X86_VIDEO_EGA_PLANES; ++plane)
        if ((samples[plane] & bit) != 0u) pixel |= (lib_u8)(1u << plane);
    return pixel;
}

static lib_u8 x86_video_ega_graphics_diagnostic(const x86_video *adapter)
{
    static const lib_u8 mux_bits[2][4][2] = {
        {{0u, 2u}, {1u, 3u}, {4u, 5u}, {0u, 0u}},
        {{0u, 2u}, {4u, 5u}, {1u, 3u}, {4u, 5u}}
    };
    const lib_u8 personality = adapter->data.ega_personality ==
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR ? 1u : 0u;
    const lib_u8 select = (adapter->data.attribute[18] >> 4u) & 3u;
    lib_u32 horizontal, period, blank, phase, x, y, offset;
    lib_u8 samples[X86_VIDEO_EGA_PLANES], color, bit;
    x86_video_kind kind;

    if (!x86_video_ega_planar_active(adapter) ||
        (adapter->data.sequencer[1] & 1u) == 0u ||
        !x86_video_ega_display_kind(adapter, &kind) ||
        !x86_video_ega_raster_geometry(adapter, &horizontal, &period, &blank) ||
        (x86_video_status(adapter) & X86_VIDEO_STATUS_DISPLAY_ENABLE) == 0u ||
        adapter->data.crtc[X86_VIDEO_CRT_INTERLACE_SKEW] >
            adapter->data.crtc[X86_VIDEO_CRTC_MAXIMUM_RASTER_ADDRESS] ||
        adapter->data.attribute[19] > 7u || (personality == 0u && select == 3u)) return 0u;
    phase = adapter->data.raster_phase % period;
    x = (phase % horizontal) / x86_video_ega_horizontal_scale(adapter) * 8u;
    y = phase / horizontal;
    offset = x86_video_ega_graphics_offset(adapter, (lib_u16)x, (lib_u16)y);
    bit = (lib_u8)(0x80u >> ((x + adapter->data.attribute[19]) & 7u));
    for (lib_u8 plane = 0u; plane < X86_VIDEO_EGA_PLANES; ++plane)
        samples[plane] = adapter->data.ega_planar_vram[(lib_size)plane *
            X86_VIDEO_EGA_PLANE_BYTES + offset];
    color = adapter->data.attribute[x86_video_ega_plane_pixel(samples, bit) &
        adapter->data.attribute[18] & 15u];
    return (lib_u8)(((color >> mux_bits[personality][select][0]) & 1u) << 4u) |
        (lib_u8)(((color >> mux_bits[personality][select][1]) & 1u) << 5u);
}

static lib_status x86_video_capture_ega_planar_snapshot(x86_video *adapter,
    x86_video_snapshot *out_snapshot)
{
    x86_video_kind kind;
    lib_u16 width;
    lib_u16 height;
    lib_u16 y;
    lib_u16 x;
    lib_i32 buffer_changed;

    if (adapter == LIB_NULL || out_snapshot == LIB_NULL ||
        !x86_video_ega_planar_active(adapter)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if (!x86_video_ega_display_kind(adapter, &kind) ||
        adapter->data.crtc[X86_VIDEO_CRT_INTERLACE_SKEW] >
            adapter->data.crtc[X86_VIDEO_CRTC_MAXIMUM_RASTER_ADDRESS] ||
        adapter->data.attribute[19] > 7u) return LIB_STATUS_UNSUPPORTED;
    width = kind == X86_VIDEO_KIND_EGA_320X200X16 ?
        X86_VIDEO_GRAPHICS_WIDTH : X86_VIDEO_CGA_HIGH_RES_WIDTH;
    height = kind == X86_VIDEO_KIND_EGA_640X350X16 ?
        X86_VIDEO_EGA_HIGH_RES_HEIGHT : X86_VIDEO_GRAPHICS_HEIGHT;
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind != kind ||
        adapter->data.captured_generation != adapter->data.dirty_generation;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = kind;
    out_snapshot->pixel_width = width;
    out_snapshot->pixel_height = height;
    for (x = 0u; x < X86_VIDEO_PALETTE_ENTRIES; ++x) {
        lib_u8 enabled_index = (lib_u8)(x & adapter->data.attribute[18]);
        out_snapshot->palette_rgb[x] = x86_video_ega_palette_color(adapter,
            adapter->data.attribute[enabled_index]);
    }
    for (y = 0u; y < height; ++y) {
        for (x = 0u; x < width;) {
            const lib_u32 offset = x86_video_ega_graphics_offset(adapter, x, y);
            lib_u8 bit = (lib_u8)(0x80u >> ((x + adapter->data.attribute[19]) & 7u));
            lib_u8 samples[X86_VIDEO_EGA_PLANES];

            for (lib_u8 plane = 0u; plane < X86_VIDEO_EGA_PLANES; ++plane) {
                const lib_u8 *source = adapter->data.ega_planar_vram +
                    (lib_size)plane * X86_VIDEO_EGA_PLANE_BYTES;
                samples[plane] = source[offset];
            }
            while (bit != 0u && x < width) {
                out_snapshot->pixels[(lib_u32)y * width + x] = x86_video_ega_plane_pixel(samples, bit);
                ++x;
                bit >>= 1u;
            }
        }
    }
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = kind;
    adapter->data.captured_generation = adapter->data.dirty_generation;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_status x86_video_capture_vga_mode13_snapshot(x86_video *adapter,
    x86_video_snapshot *out_snapshot)
{
    lib_u32 pixel;
    lib_u16 index;

    if (adapter == LIB_NULL || out_snapshot == LIB_NULL ||
        !x86_video_vga_mode13_active(adapter)) return LIB_STATUS_UNSUPPORTED;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = X86_VIDEO_KIND_VGA_320X200X256;
    out_snapshot->pixel_width = X86_VIDEO_GRAPHICS_WIDTH;
    out_snapshot->pixel_height = X86_VIDEO_GRAPHICS_HEIGHT;
    for (index = 0u; index < X86_VIDEO_PALETTE_ENTRIES; ++index) {
        const lib_u8 *entry = adapter->data.vga_dac[index &
            adapter->data.vga_dac_mask];
        out_snapshot->palette_rgb[index] =
            ((lib_u32)((entry[0] << 2u) | (entry[0] >> 4u)) << 16u) |
            ((lib_u32)((entry[1] << 2u) | (entry[1] >> 4u)) << 8u) |
            (lib_u32)((entry[2] << 2u) | (entry[2] >> 4u));
    }
    for (pixel = 0u; pixel < X86_VIDEO_GRAPHICS_WIDTH *
        X86_VIDEO_GRAPHICS_HEIGHT; ++pixel) {
        const lib_u8 *plane = adapter->data.ega_planar_vram +
            (lib_size)(pixel & 3u) * X86_VIDEO_EGA_PLANE_BYTES;
        out_snapshot->pixels[pixel] = plane[pixel >> 2u];
    }
    out_snapshot->buffer_changed = !adapter->data.captured ||
        adapter->data.captured_kind != X86_VIDEO_KIND_VGA_320X200X256 ||
        adapter->data.captured_generation != adapter->data.dirty_generation;
    out_snapshot->cursor_changed = LIB_FALSE;
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = X86_VIDEO_KIND_VGA_320X200X256;
    adapter->data.captured_generation = adapter->data.dirty_generation;
    return LIB_STATUS_OK;
}

static lib_status x86_video_capture_blank_ega_snapshot(x86_video *adapter,
    x86_video_snapshot *out_snapshot)
{
    x86_video_kind kind;

    if (adapter == LIB_NULL || out_snapshot == LIB_NULL ||
        !x86_video_ega_display_kind(adapter, &kind)) return LIB_STATUS_UNSUPPORTED;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = kind;
    out_snapshot->pixel_width = kind == X86_VIDEO_KIND_EGA_320X200X16 ?
        X86_VIDEO_GRAPHICS_WIDTH : X86_VIDEO_CGA_HIGH_RES_WIDTH;
    out_snapshot->pixel_height = kind == X86_VIDEO_KIND_EGA_640X350X16 ?
        X86_VIDEO_EGA_HIGH_RES_HEIGHT : X86_VIDEO_GRAPHICS_HEIGHT;
    out_snapshot->buffer_changed = !adapter->data.captured ||
        adapter->data.captured_kind != kind ||
        adapter->data.captured_generation != adapter->data.dirty_generation;
    out_snapshot->cursor_changed = LIB_FALSE;
    adapter->data.captured = LIB_TRUE;
    adapter->data.captured_kind = kind;
    adapter->data.captured_generation = adapter->data.dirty_generation;
    return LIB_STATUS_OK;
}

lib_status x86_video_capture_snapshot_from(x86_video *adapter, const x86_video_memory_reader *memory,
    x86_video_snapshot *out_snapshot)
{
    if (adapter == LIB_NULL || out_snapshot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (adapter->data.ega_controller_configured && !adapter->data.vga_configured) {
        /* Graphics Miscellaneous selects text/graphics scanout (CECG 4-19).
         * Attribute Mode controls interpretation of the serialized output. */
        if ((adapter->data.graphics[6] & 1u) != 0u)
            return x86_video_ega_output_active(adapter) ?
                x86_video_capture_ega_planar_snapshot(adapter, out_snapshot) :
                x86_video_capture_blank_ega_snapshot(adapter, out_snapshot);
        /* Retain the existing logical CGA compatibility view. This borrowed-
         * memory model is not physical EGA shift-register qualification. */
        if (x86_video_is_high_res_graphics_mode(adapter))
            return x86_video_capture_high_res_graphics_snapshot(adapter, memory, out_snapshot);
        if (x86_video_is_graphics_mode(adapter))
            return x86_video_capture_graphics_snapshot(adapter, memory, out_snapshot);
        return x86_video_capture_text_snapshot_from(adapter, memory, out_snapshot);
    }
    if (x86_video_vga_mode13_active(adapter)) {
        return x86_video_capture_vga_mode13_snapshot(adapter, out_snapshot);
    }
    if (x86_video_ega_planar_display_active(adapter)) {
        return x86_video_capture_ega_planar_snapshot(adapter, out_snapshot);
    }
    if (x86_video_is_high_res_graphics_mode(adapter)) {
        return x86_video_capture_high_res_graphics_snapshot(adapter, memory,
            out_snapshot);
    }
    return x86_video_is_graphics_mode(adapter) ?
        x86_video_capture_graphics_snapshot(adapter, memory, out_snapshot) :
        x86_video_capture_text_snapshot_from(adapter, memory, out_snapshot);
}






lib_status x86_video_create(x86_video **out_video)
{
    x86_video *video;

    if (out_video == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_video = LIB_NULL;
    video = lib_allocate_zero(1u, sizeof(*video));
    if (video == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_video_reset(video);
    *out_video = video;
    return LIB_STATUS_OK;
}

lib_status x86_video_observe_bus(const x86_video *video,
    x86_video_bus_observation *out_observation)
{
    if (video == LIB_NULL || out_observation == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_observation = (x86_video_bus_observation) {
        .cpu_memory_disabled = video->data.compaq_cpu_video_memory_disabled != 0u,
        .graphics_miscellaneous = video->data.graphics[6u],
        .sequencer_reset = video->data.sequencer[0u],
        .vertical_interrupt = video->data.vertical_interrupt_pending &&
            (video->data.crtc[X86_VIDEO_CRTC_VERTICAL_RETRACE_END] & 0x20u) == 0u
    };
    return LIB_STATUS_OK;
}

void x86_video_destroy(x86_video *video)
{
    if (video == LIB_NULL) return;
    x86_video_finalize(video);
    lib_release(video);
}

static lib_status x86_video_memory_read_selected(x86_video *video,
    x86_video_memory_region region, lib_u32 address, lib_u8 *destination,
    lib_size bytes, lib_bool observe_only)
{
    if (region == X86_VIDEO_MEMORY_CGA)
        return x86_video_cga_read(video, address, destination, bytes);
    if (region == X86_VIDEO_MEMORY_PLANAR)
        return x86_video_ega_planar_read(video, address, destination, bytes,
            observe_only);
    return LIB_STATUS_INVALID_ARGUMENT;
}

lib_status x86_video_memory_read(x86_video *video, x86_video_memory_region region,
    lib_u32 address, lib_u8 *destination, lib_size bytes)
{
    return x86_video_memory_read_selected(video, region, address, destination,
        bytes, LIB_FALSE);
}

lib_status x86_video_memory_inspect(x86_video *video, x86_video_memory_region region,
    lib_u32 address, lib_u8 *destination, lib_size bytes)
{
    return x86_video_memory_read_selected(video, region, address, destination,
        bytes, LIB_TRUE);
}

lib_status x86_video_memory_write(x86_video *video, x86_video_memory_region region,
    lib_u32 address, const lib_u8 *source, lib_size bytes)
{
    if (region == X86_VIDEO_MEMORY_CGA)
        return x86_video_cga_write(video, address, source, bytes);
    if (region == X86_VIDEO_MEMORY_PLANAR)
        return x86_video_ega_planar_write(video, address, source, bytes);
    return LIB_STATUS_INVALID_ARGUMENT;
}

lib_status x86_video_memory_query(x86_video *video, x86_video_memory_region region,
    lib_u32 address, lib_size bytes, lib_bool write)
{
    if (video == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (region == X86_VIDEO_MEMORY_CGA)
        return x86_video_cga_query(video, address, bytes, write);
    if (region == X86_VIDEO_MEMORY_PLANAR)
        return x86_video_ega_planar_query(video, address, bytes, write);
    return LIB_STATUS_INVALID_ARGUMENT;
}

void x86_video_notify_memory_write(x86_video *video, lib_u32 address, lib_size bytes)
{
    x86_video_ega_write_observer(video, address, bytes);
}
