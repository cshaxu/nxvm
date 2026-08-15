/* Copyright 2012-2014 Neko. */

/* Core-owned CGA text-controller state. Host presentation is outside core. */

#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

#define CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE 0x01u
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
#define CORE_MACHINE_VADP_CRTC_VERTICAL_SYNC_POSITION 0x07u
#define CORE_MACHINE_VADP_CRTC_INTERLACE_SKEW 0x08u
#define CORE_MACHINE_VADP_CRTC_MAXIMUM_RASTER_ADDRESS 0x09u
#define CORE_MACHINE_VADP_CRTC_CURSOR_TOP 0x0au
#define CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM 0x0bu
#define CORE_MACHINE_VADP_CRTC_START_HIGH 0x0cu
#define CORE_MACHINE_VADP_CRTC_START_LOW 0x0du
#define CORE_MACHINE_VADP_CRTC_CURSOR_HIGH 0x0eu
#define CORE_MACHINE_VADP_CRTC_CURSOR_LOW 0x0fu
#define CORE_MACHINE_VADP_CRTC_OFFSET 0x13u
#define CORE_MACHINE_VADP_MODE_GRAPHICS 0x02u
#define CORE_MACHINE_VADP_MODE_VIDEO_ENABLE 0x08u
#define CORE_MACHINE_VADP_MODE_HIGH_RES 0x10u
#define CORE_MACHINE_VADP_COLOR_PALETTE_SELECT 0x20u
#define CORE_MACHINE_VADP_GRAPHICS_BYTES_PER_ROW 80u
#define CORE_MACHINE_VADP_GRAPHICS_ODD_ROW_OFFSET 0x2000u
#define CORE_MACHINE_VADP_EGA_320X200_ROW_BYTES 40u
#define CORE_MACHINE_VADP_EGA_640X350_ROW_BYTES 80u
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

static C_VOID core_machine_vadp_mark_dirty(t_vadp *adapter);

static type_status core_machine_vadp_cga_read(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (adapter == STD_NULL || destination == 0u || physical < CORE_MACHINE_VADP_VIDEO_BASE ||
        (type_unsigned_64)physical - CORE_MACHINE_VADP_VIDEO_BASE + bytes >
        CORE_MACHINE_VADP_VIDEO_BYTES) return TYPE_STATUS_UNSUPPORTED;
    STD_MEMCPY((C_VOID *)destination, adapter->data.cga_vram +
        physical - CORE_MACHINE_VADP_VIDEO_BASE, bytes);
    return TYPE_STATUS_OK;
}

static type_status core_machine_vadp_cga_write(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (adapter == STD_NULL || source == 0u || physical < CORE_MACHINE_VADP_VIDEO_BASE ||
        (type_unsigned_64)physical - CORE_MACHINE_VADP_VIDEO_BASE + bytes >
        CORE_MACHINE_VADP_VIDEO_BYTES) return TYPE_STATUS_UNSUPPORTED;
    STD_MEMCPY(adapter->data.cga_vram + physical - CORE_MACHINE_VADP_VIDEO_BASE,
        (const C_VOID *)source, bytes);
    core_machine_vadp_mark_dirty(adapter);
    return TYPE_STATUS_OK;
}

static type_status core_machine_vadp_cga_query(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    (C_VOID)owner;
    return (access == CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == CORE_MACHINE_MEMORY_ACCESS_WRITE) &&
        physical >= CORE_MACHINE_VADP_VIDEO_BASE &&
        (type_unsigned_64)physical - CORE_MACHINE_VADP_VIDEO_BASE + bytes <=
        CORE_MACHINE_VADP_VIDEO_BYTES ? TYPE_STATUS_OK : TYPE_STATUS_UNSUPPORTED;
}

static C_INT core_machine_vadp_is_graphics_mode(const t_vadp *adapter)
{
    return adapter != STD_NULL &&
        (adapter->data.mode_control & (CORE_MACHINE_VADP_MODE_GRAPHICS |
            CORE_MACHINE_VADP_MODE_HIGH_RES)) == CORE_MACHINE_VADP_MODE_GRAPHICS;
}

static C_INT core_machine_vadp_is_high_res_graphics_mode(const t_vadp *adapter)
{
    return adapter != STD_NULL && adapter->data.mode_control == 0x1au;
}

static type_unsigned_32 core_machine_vadp_rgbi_color(type_unsigned_8 index)
{
    static const type_unsigned_32 colors[16] = {
        0x000000u, 0x0000aau, 0x00aa00u, 0x00aaaau,
        0xaa0000u, 0xaa00aau, 0xaa5500u, 0xaaaaaau,
        0x555555u, 0x5555ffu, 0x55ff55u, 0x55ffffu,
        0xff5555u, 0xff55ffu, 0xffff55u, 0xffffffu
    };

    return colors[index & 0x0fu];
}

static C_VOID core_machine_vadp_graphics_palette(const t_vadp *adapter,
    type_unsigned_32 palette[4])
{
    C_INT alternate;

    if (adapter == STD_NULL || palette == STD_NULL) return;
    alternate = (adapter->data.color_select &
        CORE_MACHINE_VADP_COLOR_PALETTE_SELECT) != 0u;
    palette[0] = core_machine_vadp_rgbi_color(adapter->data.color_select);
    palette[1] = core_machine_vadp_rgbi_color(alternate ? 3u : 2u);
    palette[2] = core_machine_vadp_rgbi_color(alternate ? 5u : 4u);
    palette[3] = core_machine_vadp_rgbi_color(alternate ? 7u : 6u);
    if ((adapter->data.mode_control & CORE_MACHINE_VADP_MODE_VIDEO_ENABLE) == 0u) {
        palette[0] = 0u;
        palette[1] = 0u;
        palette[2] = 0u;
        palette[3] = 0u;
    }
}

static C_VOID core_machine_vadp_high_res_palette(const t_vadp *adapter,
    type_unsigned_32 palette[CORE_MACHINE_DISPLAY_PALETTE_ENTRIES])
{
    if (adapter == STD_NULL || palette == STD_NULL) return;
    palette[0] = 0u;
    palette[1] = core_machine_vadp_rgbi_color(adapter->data.color_select);
    if ((adapter->data.mode_control & CORE_MACHINE_VADP_MODE_VIDEO_ENABLE) == 0u) {
        palette[1] = 0u;
    }
}

static C_INT core_machine_vadp_ega_planar_active(const t_vadp *adapter)
{
    return adapter != STD_NULL && adapter->data.ega_planar_enabled &&
        adapter->data.ega_planar_armed &&
        adapter->data.ega_planar_vram != 0u &&
        adapter->data.ega_sequencer_configured &&
        adapter->data.ega_controller_configured &&
        (adapter->data.graphics[6] & 0x0cu) == 0x04u &&
        (adapter->data.graphics[5] & 0x0bu) == 0u &&
        (adapter->data.attribute[16] & 0x01u) != 0u &&
        adapter->data.attribute_display_enabled;
}

static type_unsigned_8 core_machine_vadp_rotate_right(type_unsigned_8 value, type_unsigned_8 count)
{
    count &= 7u;
    return count == 0u ? value : (type_unsigned_8)((value >> count) |
        (value << (8u - count)));
}

static type_unsigned_8 core_machine_vadp_logical_operation(type_unsigned_8 operation,
    type_unsigned_8 source, type_unsigned_8 latch)
{
    switch (operation & 0x03u) {
    case 1u: return source & latch;
    case 2u: return source | latch;
    case 3u: return source ^ latch;
    default: return source;
    }
}

static type_status core_machine_vadp_ega_planar_read(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    t_vadp *adapter = (t_vadp *)owner;
    type_native_unsigned index;
    type_unsigned_8 *out = (type_unsigned_8 *)destination;

    if (adapter == STD_NULL || destination == 0u ||
        !core_machine_vadp_ega_planar_active(adapter)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    if (physical < CORE_MACHINE_VADP_EGA_APERTURE_BASE ||
        (type_unsigned_64)physical - CORE_MACHINE_VADP_EGA_APERTURE_BASE + bytes >
            CORE_MACHINE_VADP_EGA_APERTURE_BYTES) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    for (index = 0u; index < bytes; ++index) {
        type_unsigned_32 offset = physical - CORE_MACHINE_VADP_EGA_APERTURE_BASE + index;
        type_unsigned_8 plane;

        for (plane = 0u; plane < CORE_MACHINE_VADP_EGA_PLANES; ++plane) {
            adapter->data.ega_latches[plane] = ((type_unsigned_8 *)adapter->data.ega_planar_vram)
                [(STD_SIZE_T)plane * CORE_MACHINE_VADP_EGA_PLANE_BYTES + offset];
        }
        out[index] = adapter->data.ega_latches[adapter->data.graphics[4] & 0x03u];
    }
    return TYPE_STATUS_OK;
}

static type_status core_machine_vadp_ega_planar_write(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    t_vadp *adapter = (t_vadp *)owner;
    const type_unsigned_8 *input = (const type_unsigned_8 *)source;
    type_native_unsigned index;

    if (adapter == STD_NULL || source == 0u ||
        !core_machine_vadp_ega_planar_active(adapter)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    if (physical < CORE_MACHINE_VADP_EGA_APERTURE_BASE ||
        (type_unsigned_64)physical - CORE_MACHINE_VADP_EGA_APERTURE_BASE + bytes >
            CORE_MACHINE_VADP_EGA_APERTURE_BYTES) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    for (index = 0u; index < bytes; ++index) {
        type_unsigned_32 offset = physical - CORE_MACHINE_VADP_EGA_APERTURE_BASE + index;
        type_unsigned_8 rotated = core_machine_vadp_rotate_right(input[index],
            adapter->data.graphics[3]);
        type_unsigned_8 plane;

        for (plane = 0u; plane < CORE_MACHINE_VADP_EGA_PLANES; ++plane) {
            type_unsigned_8 *target = (type_unsigned_8 *)adapter->data.ega_planar_vram +
                (STD_SIZE_T)plane * CORE_MACHINE_VADP_EGA_PLANE_BYTES + offset;
            type_unsigned_8 source_byte = (adapter->data.graphics[1] & (1u << plane)) != 0u ?
                (adapter->data.graphics[0] & (1u << plane)) != 0u ? 0xffu : 0u :
                rotated;
            type_unsigned_8 merged = core_machine_vadp_logical_operation(
                adapter->data.graphics[3] >> 3, source_byte,
                adapter->data.ega_latches[plane]);

            if ((adapter->data.sequencer[2] & (1u << plane)) != 0u) {
                *target = (type_unsigned_8)((merged & adapter->data.graphics[8]) |
                    (adapter->data.ega_latches[plane] &
                    (type_unsigned_8)~adapter->data.graphics[8]));
            }
        }
    }
    core_machine_vadp_mark_dirty(adapter);
    return TYPE_STATUS_OK;
}

static type_status core_machine_vadp_ega_planar_query(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    t_vadp *adapter = (t_vadp *)owner;

    if (adapter == STD_NULL || !core_machine_vadp_ega_planar_active(adapter) ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != CORE_MACHINE_MEMORY_ACCESS_WRITE) ||
        physical < CORE_MACHINE_VADP_EGA_APERTURE_BASE ||
        (type_unsigned_64)physical - CORE_MACHINE_VADP_EGA_APERTURE_BASE + bytes >
            CORE_MACHINE_VADP_EGA_APERTURE_BYTES) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    return TYPE_STATUS_OK;
}

static C_INT core_machine_vadp_cga_logical_raster_active(const t_vadp *adapter)
{
    return adapter != STD_NULL && !adapter->data.ega_controller_configured &&
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_HORIZONTAL_DISPLAYED] != 0u &&
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAYED] != 0u;
}

static type_unsigned_32 core_machine_vadp_cga_scanlines_per_row(const t_vadp *adapter)
{
    return (type_unsigned_32)adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_MAXIMUM_RASTER_ADDRESS] + 1u;
}

static type_unsigned_32 core_machine_vadp_cga_logical_raster_period(const t_vadp *adapter)
{
    type_unsigned_32 rows = (type_unsigned_32)adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_VERTICAL_TOTAL] + 1u;
    type_unsigned_32 scanlines = rows * core_machine_vadp_cga_scanlines_per_row(adapter) +
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_VERTICAL_TOTAL_ADJUST];

    return scanlines * ((type_unsigned_32)adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_HORIZONTAL_TOTAL] + 1u);
}

static type_unsigned_16 core_machine_vadp_text_columns(const t_vadp *adapter)
{
    type_unsigned_16 columns;

    if (!core_machine_vadp_cga_logical_raster_active(adapter)) return adapter->data.columns;
    columns = adapter->data.crtc[CORE_MACHINE_VADP_CRTC_HORIZONTAL_DISPLAYED];
    return columns > CORE_MACHINE_DISPLAY_MAX_COLUMNS ? CORE_MACHINE_DISPLAY_MAX_COLUMNS :
        columns;
}

static type_unsigned_16 core_machine_vadp_text_rows(const t_vadp *adapter)
{
    type_unsigned_16 rows;

    if (!core_machine_vadp_cga_logical_raster_active(adapter)) return adapter->data.rows;
    rows = adapter->data.crtc[CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAYED];
    return rows > CORE_MACHINE_DISPLAY_MAX_ROWS ? CORE_MACHINE_DISPLAY_MAX_ROWS : rows;
}

static C_INT core_machine_vadp_supported_crtc_index(const t_vadp *adapter,
    type_unsigned_8 index)
{
    if (adapter == STD_NULL || index >= CORE_MACHINE_VADP_CRTC_REGISTER_COUNT) return TYPE_FALSE;
    if (!adapter->data.ega_controller_configured) {
        return (index <= CORE_MACHINE_VADP_CRTC_VERTICAL_SYNC_POSITION ||
            index == CORE_MACHINE_VADP_CRTC_MAXIMUM_RASTER_ADDRESS ||
            (index >= CORE_MACHINE_VADP_CRTC_CURSOR_TOP &&
            index <= CORE_MACHINE_VADP_CRTC_CURSOR_LOW));
    }
    return (index >= CORE_MACHINE_VADP_CRTC_CURSOR_TOP &&
        index <= CORE_MACHINE_VADP_CRTC_CURSOR_LOW) || index == CORE_MACHINE_VADP_CRTC_OFFSET;
}

static type_unsigned_8 core_machine_vadp_crtc_mask(type_unsigned_8 index)
{
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

static type_unsigned_16 core_machine_vadp_crtc_word(const t_vadp *adapter,
    type_unsigned_8 high_index)
{
    type_unsigned_8 low_index = (type_unsigned_8)(high_index + 1u);

    if (adapter == STD_NULL || !core_machine_vadp_supported_crtc_index(adapter,
            high_index) || !core_machine_vadp_supported_crtc_index(adapter, low_index)) {
        return 0u;
    }
    return (type_unsigned_16)(((type_unsigned_16)adapter->data.crtc[high_index] << 8) |
        adapter->data.crtc[low_index]);
}

static core_machine_display_kind core_machine_vadp_ega_display_kind(
    const t_vadp *adapter)
{
    return adapter != STD_NULL && adapter->data.crtc[CORE_MACHINE_VADP_CRTC_OFFSET] ==
        CORE_MACHINE_VADP_EGA_640X350_CRTC_OFFSET ?
        CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 :
        CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16;
}

static C_VOID core_machine_vadp_mark_dirty(t_vadp *adapter)
{
    if (adapter != STD_NULL) ++adapter->data.dirty_generation;
}

static C_INT core_machine_vadp_sequencer_index_supported(type_unsigned_8 index)
{
    return index == 0u || index == 1u || index == 2u || index == 4u;
}

static type_unsigned_8 core_machine_vadp_sequencer_mask(type_unsigned_8 index)
{
    switch (index) {
    case 0u: return 0x03u;
    case 1u: return 0x3du;
    case 2u: return 0x0fu;
    case 4u: return 0x0eu;
    default: return 0u;
    }
}

static C_INT core_machine_vadp_graphics_index_supported(type_unsigned_8 index)
{
    return index < CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT;
}

static type_unsigned_8 core_machine_vadp_graphics_mask(type_unsigned_8 index)
{
    static const type_unsigned_8 masks[CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT] = {
        0x0fu, 0x0fu, 0x0fu, 0x1fu, 0x03u, 0x7bu, 0x0fu, 0x0fu, 0xffu
    };

    return core_machine_vadp_graphics_index_supported(index) ? masks[index] : 0u;
}

static C_INT core_machine_vadp_attribute_index_supported(type_unsigned_8 index)
{
    return index < CORE_MACHINE_VADP_ATTRIBUTE_REGISTER_COUNT;
}

static type_unsigned_8 core_machine_vadp_attribute_mask(type_unsigned_8 index)
{
    if (index < 16u) return 0x3fu;
    switch (index) {
    case 16u: return 0xffu;
    case 17u: return 0x3fu;
    case 18u: return 0x0fu;
    case 19u: return 0x0fu;
    case 20u: return 0x0fu;
    default: return 0u;
    }
}

static C_VOID core_machine_vadp_active_ega_aperture(const t_vadp *adapter,
    type_unsigned_32 *out_base, type_unsigned_32 *out_bytes)
{
    type_unsigned_8 map_select;

    if (adapter == STD_NULL || out_base == STD_NULL || out_bytes == STD_NULL) {
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

static C_VOID core_machine_vadp_reset_sequencer(t_vadp *adapter)
{
    if (adapter == STD_NULL || !adapter->data.ega_sequencer_configured) return;
    adapter->data.sequencer_index = 0u;
    adapter->data.sequencer[0] = adapter->data.ega_sequencer.reset & 0x03u;
    adapter->data.sequencer[1] = adapter->data.ega_sequencer.clocking_mode & 0x3du;
    adapter->data.sequencer[2] = adapter->data.ega_sequencer.map_mask & 0x0fu;
    adapter->data.sequencer[4] = adapter->data.ega_sequencer.memory_mode & 0x0eu;
}

static C_VOID core_machine_vadp_reset_ega_controllers(t_vadp *adapter)
{
    if (adapter == STD_NULL || !adapter->data.ega_controller_configured) return;
    adapter->data.graphics_index = 0u;
    STD_MEMCPY(adapter->data.graphics, adapter->data.ega_controller.graphics,
        sizeof(adapter->data.graphics));
    adapter->data.attribute_index = 0u;
    STD_MEMCPY(adapter->data.attribute, adapter->data.ega_controller.attribute,
        sizeof(adapter->data.attribute));
    adapter->data.attribute_data_phase = TYPE_FALSE;
    adapter->data.attribute_display_enabled = TYPE_TRUE;
}

static C_VOID core_machine_vadp_normalize_ega_controllers(
    core_machine_vadp_ega_controller_config *config)
{
    type_unsigned_8 index;

    if (config == STD_NULL) return;
    for (index = 0u; index < CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT; ++index) {
        config->graphics[index] &= core_machine_vadp_graphics_mask(index);
    }
    for (index = 0u; index < CORE_MACHINE_VADP_ATTRIBUTE_REGISTER_COUNT; ++index) {
        config->attribute[index] &= core_machine_vadp_attribute_mask(index);
    }
}

static C_VOID core_machine_vadp_ega_write_observer(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes)
{
    t_vadp *adapter = (t_vadp *)owner;
    type_unsigned_64 write_end;
    type_unsigned_64 aperture_end;

    if (adapter == STD_NULL || !adapter->data.ega_sequencer_configured ||
        bytes == 0u) return;
    write_end = (type_unsigned_64)physical + bytes;
    {
        type_unsigned_32 aperture_base;
        type_unsigned_32 aperture_bytes;

        core_machine_vadp_active_ega_aperture(adapter, &aperture_base,
            &aperture_bytes);
        aperture_end = (type_unsigned_64)aperture_base + aperture_bytes;
        if ((type_unsigned_64)physical < aperture_end &&
            (type_unsigned_64)aperture_base < write_end) {
            core_machine_vadp_mark_dirty(adapter);
        }
    }
}

static type_unsigned_32 core_machine_vadp_raster_period(
    const core_machine_vadp_text_timing *timing)
{
    return timing->active_display_ticks + timing->horizontal_blank_ticks +
        timing->vertical_retrace_ticks;
}

static C_INT core_machine_vadp_valid_text_timing(
    const core_machine_vadp_text_timing *timing)
{
    type_unsigned_32 period;

    if (timing == STD_NULL || timing->active_display_ticks == 0u ||
        timing->vertical_retrace_ticks == 0u) {
        return TYPE_FALSE;
    }
    period = core_machine_vadp_raster_period(timing);
    return period >= timing->active_display_ticks &&
        period >= timing->horizontal_blank_ticks &&
        period >= timing->vertical_retrace_ticks;
}

static type_unsigned_8 core_machine_vadp_status(const t_vadp *adapter)
{
    type_unsigned_32 vertical_end;
    type_unsigned_32 display_end;
    type_unsigned_8 status = 0u;

    if (adapter == STD_NULL) return 0u;
    if (core_machine_vadp_cga_logical_raster_active(adapter)) {
        type_unsigned_32 horizontal_total = (type_unsigned_32)adapter->data.crtc[
            CORE_MACHINE_VADP_CRTC_HORIZONTAL_TOTAL] + 1u;
        type_unsigned_32 period = core_machine_vadp_cga_logical_raster_period(adapter);
        type_unsigned_32 scanline;
        type_unsigned_32 character;
        type_unsigned_32 display_scanlines = (type_unsigned_32)adapter->data.crtc[
            CORE_MACHINE_VADP_CRTC_VERTICAL_DISPLAYED] *
            core_machine_vadp_cga_scanlines_per_row(adapter);
        type_unsigned_32 vertical_sync_start = (type_unsigned_32)adapter->data.crtc[
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

static C_VOID core_machine_vadp_write_crtc_index(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    (C_VOID)port_id;
    if (port != STD_NULL && owner != STD_NULL) {
        ((t_vadp *)owner)->data.crtc_index = port->data.ioByte;
    }
}

static C_VOID core_machine_vadp_read_crtc_data(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL) return;
    port->data.ioByte = core_machine_vadp_supported_crtc_index(adapter,
        adapter->data.crtc_index) ?
        adapter->data.crtc[adapter->data.crtc_index] : 0u;
}

static C_VOID core_machine_vadp_write_crtc_data(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL ||
        !core_machine_vadp_supported_crtc_index(adapter, adapter->data.crtc_index)) {
        return;
    }
    {
        type_unsigned_8 value = port->data.ioByte &
            core_machine_vadp_crtc_mask(adapter->data.crtc_index);

        if (adapter->data.crtc[adapter->data.crtc_index] == value) return;
        adapter->data.crtc[adapter->data.crtc_index] = value;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static C_VOID core_machine_vadp_read_mode(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    (C_VOID)port_id;
    if (port != STD_NULL && owner != STD_NULL) {
        port->data.ioByte = ((t_vadp *)owner)->data.mode_control;
    }
}

static C_VOID core_machine_vadp_write_mode(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    type_unsigned_8 value;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL) return;
    value = port->data.ioByte;
    /* T254 admits exactly 1Ah for the digital 640x200x2 CGA slice. */
    if ((value & (CORE_MACHINE_VADP_MODE_GRAPHICS |
        CORE_MACHINE_VADP_MODE_HIGH_RES)) ==
        (CORE_MACHINE_VADP_MODE_GRAPHICS | CORE_MACHINE_VADP_MODE_HIGH_RES) &&
        value != 0x1au) {
        return;
    }
    if (adapter->data.mode_control != value) {
        adapter->data.mode_control = value;
        adapter->data.columns = (value & 0x01u) != 0u ? 80u : 40u;
        adapter->data.color_enabled = (value & 0x04u) != 0u;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static C_VOID core_machine_vadp_read_color(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    (C_VOID)port_id;
    if (port != STD_NULL && owner != STD_NULL) {
        port->data.ioByte = ((t_vadp *)owner)->data.color_select;
    }
}

static C_VOID core_machine_vadp_write_color(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL) return;
    if (adapter->data.color_select != port->data.ioByte) {
        adapter->data.color_select = port->data.ioByte;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static C_VOID core_machine_vadp_read_status(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (C_VOID)port_id;
    if (port != STD_NULL && adapter != STD_NULL) {
        port->data.ioByte = core_machine_vadp_status(adapter);
        if (adapter->data.ega_controller_configured) {
            adapter->data.attribute_data_phase = TYPE_FALSE;
        }
    }
}

static C_VOID core_machine_vadp_read_graphics_index(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (C_VOID)port_id;
    if (port != STD_NULL && adapter != STD_NULL) {
        port->data.ioByte = adapter->data.ega_controller_configured ?
            adapter->data.graphics_index : 0xffu;
    }
}

static C_VOID core_machine_vadp_write_graphics_index(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (C_VOID)port_id;
    if (port != STD_NULL && adapter != STD_NULL &&
        adapter->data.ega_controller_configured) {
        adapter->data.graphics_index = port->data.ioByte;
    }
}

static C_VOID core_machine_vadp_read_graphics_data(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL) return;
    port->data.ioByte = adapter->data.ega_controller_configured &&
        core_machine_vadp_graphics_index_supported(adapter->data.graphics_index) ?
        adapter->data.graphics[adapter->data.graphics_index] : 0xffu;
}

static C_VOID core_machine_vadp_write_graphics_data(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    type_unsigned_8 index;
    type_unsigned_8 value;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL ||
        !adapter->data.ega_controller_configured) return;
    index = adapter->data.graphics_index;
    if (!core_machine_vadp_graphics_index_supported(index)) return;
    value = port->data.ioByte & core_machine_vadp_graphics_mask(index);
    if (index == 6u && adapter->data.ega_planar_enabled) {
        type_bool armed = (value & 0x0cu) == 0x04u;

        if (adapter->data.ega_planar_armed != armed) {
            adapter->data.ega_planar_armed = armed;
            core_machine_vadp_mark_dirty(adapter);
        }
    }
    if (adapter->data.graphics[index] != value) {
        adapter->data.graphics[index] = value;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static C_VOID core_machine_vadp_read_attribute_data(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL) return;
    port->data.ioByte = adapter->data.ega_controller_configured &&
        core_machine_vadp_attribute_index_supported(adapter->data.attribute_index) ?
        adapter->data.attribute[adapter->data.attribute_index] : 0xffu;
}

static C_VOID core_machine_vadp_write_attribute(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    type_unsigned_8 value;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL ||
        !adapter->data.ega_controller_configured) return;
    value = port->data.ioByte;
    if (!adapter->data.attribute_data_phase) {
        adapter->data.attribute_index = value & 0x1fu;
        adapter->data.attribute_display_enabled = (value & 0x20u) != 0u;
        adapter->data.attribute_data_phase = TYPE_TRUE;
        return;
    }
    if (core_machine_vadp_attribute_index_supported(adapter->data.attribute_index)) {
        type_unsigned_8 index = adapter->data.attribute_index;
        type_unsigned_8 masked = value & core_machine_vadp_attribute_mask(index);

        if (adapter->data.attribute[index] != masked) {
            adapter->data.attribute[index] = masked;
            core_machine_vadp_mark_dirty(adapter);
        }
    }
    adapter->data.attribute_data_phase = TYPE_FALSE;
}

static C_VOID core_machine_vadp_read_sequencer_index(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (C_VOID)port_id;
    if (port != STD_NULL && adapter != STD_NULL) {
        port->data.ioByte = adapter->data.ega_sequencer_configured ?
            adapter->data.sequencer_index : 0xffu;
    }
}

static C_VOID core_machine_vadp_write_sequencer_index(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (C_VOID)port_id;
    if (port != STD_NULL && adapter != STD_NULL &&
        adapter->data.ega_sequencer_configured) {
        adapter->data.sequencer_index = port->data.ioByte;
    }
}

static C_VOID core_machine_vadp_read_sequencer_data(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    const t_vadp *adapter = (const t_vadp *)owner;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL) return;
    port->data.ioByte = adapter->data.ega_sequencer_configured &&
        core_machine_vadp_sequencer_index_supported(adapter->data.sequencer_index) ?
        adapter->data.sequencer[adapter->data.sequencer_index] : 0xffu;
}

static C_VOID core_machine_vadp_write_sequencer_data(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;
    type_unsigned_8 index;
    type_unsigned_8 value;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL ||
        !adapter->data.ega_sequencer_configured) return;
    index = adapter->data.sequencer_index;
    if (!core_machine_vadp_sequencer_index_supported(index)) return;
    value = port->data.ioByte & core_machine_vadp_sequencer_mask(index);
    if (adapter->data.sequencer[index] != value) {
        adapter->data.sequencer[index] = value;
        core_machine_vadp_mark_dirty(adapter);
    }
}

static C_VOID core_machine_vadp_register_cga_ports(t_vadp *adapter, t_port *port)
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
}

C_VOID core_machine_vadp_initialize(t_vadp *adapter, t_port *port)
{
    if (adapter == STD_NULL || port == STD_NULL) return;
    STD_MEMSET(adapter, TYPE_ZERO_8, sizeof(*adapter));
    core_machine_vadp_register_cga_ports(adapter, port);
    core_machine_vadp_reset(adapter);
}

C_VOID core_machine_vadp_configure_ega_ports(t_vadp *adapter, t_port *port)
{
    if (adapter == STD_NULL || port == STD_NULL) return;
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

C_VOID core_machine_vadp_reset(t_vadp *adapter)
{
    core_machine_vadp_text_timing timing;
    core_machine_vadp_ega_sequencer_config ega_sequencer;
    core_machine_vadp_ega_controller_config ega_controller;
    type_unsigned_8 crtc[CORE_MACHINE_VADP_CRTC_REGISTER_COUNT];
    type_bool crtc_initialized;
    type_bool ega_sequencer_configured;
    type_bool ega_controller_configured;
    type_bool ega_planar_enabled;
    type_virtual_address ega_planar_vram;

    if (adapter == STD_NULL) return;
    timing = adapter->data.text_timing;
    if (!core_machine_vadp_valid_text_timing(&timing)) {
        timing.active_display_ticks = CORE_MACHINE_VADP_DEFAULT_ACTIVE_DISPLAY_TICKS;
        timing.horizontal_blank_ticks = CORE_MACHINE_VADP_DEFAULT_HORIZONTAL_BLANK_TICKS;
        timing.vertical_retrace_ticks = CORE_MACHINE_VADP_DEFAULT_VERTICAL_RETRACE_TICKS;
    }
    ega_sequencer = adapter->data.ega_sequencer;
    ega_sequencer_configured = adapter->data.ega_sequencer_configured;
    ega_controller = adapter->data.ega_controller;
    ega_controller_configured = adapter->data.ega_controller_configured;
    ega_planar_enabled = adapter->data.ega_planar_enabled;
    ega_planar_vram = adapter->data.ega_planar_vram;
    crtc_initialized = adapter->data.crtc_initialized;
    STD_MEMCPY(crtc, adapter->data.crtc, sizeof(crtc));
    if (ega_planar_vram != 0u) {
        STD_MEMSET((C_VOID *)ega_planar_vram, 0,
            CORE_MACHINE_VADP_EGA_PLANES * CORE_MACHINE_VADP_EGA_PLANE_BYTES);
    }
    STD_MEMSET(&adapter->data, TYPE_ZERO_8, sizeof(adapter->data));
    adapter->data.mode_control = 0x05u;
    adapter->data.text_timing = timing;
    adapter->data.raster_phase = timing.vertical_retrace_ticks;
    adapter->data.columns = 80u;
    adapter->data.rows = 25u;
    adapter->data.color_enabled = TYPE_TRUE;
    adapter->data.crtc_initialized = TYPE_TRUE;
    if (!ega_controller_configured && crtc_initialized) {
        STD_MEMCPY(adapter->data.crtc, crtc, sizeof(adapter->data.crtc));
    } else {
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_TOP] = 6u;
        adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM] = 7u;
    }
    adapter->data.ega_sequencer = ega_sequencer;
    adapter->data.ega_sequencer_configured = ega_sequencer_configured;
    core_machine_vadp_reset_sequencer(adapter);
    adapter->data.ega_controller = ega_controller;
    adapter->data.ega_controller_configured = ega_controller_configured;
    core_machine_vadp_reset_ega_controllers(adapter);
    adapter->data.ega_planar_enabled = ega_planar_enabled;
    adapter->data.ega_planar_vram = ega_planar_vram;
    if (core_machine_vadp_cga_logical_raster_active(adapter)) {
        adapter->data.raster_phase = 0u;
        adapter->data.cga_logical_raster_started = TYPE_FALSE;
    }
    adapter->data.dirty_generation = 1u;
}

C_VOID core_machine_vadp_advance(t_vadp *adapter, t_ram *memory,
    type_unsigned_64 elapsed_ticks)
{
    type_unsigned_32 period;

    (C_VOID)memory;
    if (adapter == STD_NULL) return;
    if (core_machine_vadp_cga_logical_raster_active(adapter)) {
        type_unsigned_32 phase;

        period = core_machine_vadp_cga_logical_raster_period(adapter);
        if (period == 0u) return;
        phase = adapter->data.raster_phase % period;
        if (!adapter->data.cga_logical_raster_started && elapsed_ticks >=
            period - phase) {
            adapter->data.cga_logical_raster_started = TYPE_TRUE;
        }
        adapter->data.raster_phase = (type_unsigned_32)((phase + elapsed_ticks % period) % period);
        return;
    }
    period = core_machine_vadp_raster_period(&adapter->data.text_timing);
    if (period == 0u) return;
    adapter->data.raster_phase = (type_unsigned_32)((adapter->data.raster_phase +
        elapsed_ticks % period) % period);
}

C_VOID core_machine_vadp_finalize(t_vadp *adapter)
{
    if (adapter == STD_NULL) return;
    STD_FREE((C_VOID *)adapter->data.ega_planar_vram);
    adapter->data.ega_planar_vram = 0u;
}

type_status core_machine_vadp_configure_text(t_vadp *adapter, type_unsigned_8 mode,
    type_unsigned_16 columns, type_unsigned_16 rows, C_INT color_enabled)
{
    type_unsigned_8 mode_control;

    if (adapter == STD_NULL || rows == 0u || rows > CORE_MACHINE_DISPLAY_MAX_ROWS ||
        (columns != 40u && columns != 80u) || mode > 3u) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    mode_control = (columns == 80u ? 0x01u : 0u) |
        (color_enabled ? 0x04u : 0u);
    if (adapter->data.mode_control != mode_control ||
        adapter->data.columns != columns || adapter->data.rows != rows) {
        adapter->data.mode_control = mode_control;
        adapter->data.columns = columns;
        adapter->data.rows = rows;
        adapter->data.color_enabled = color_enabled;
        core_machine_vadp_mark_dirty(adapter);
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_vadp_configure_text_timing(t_vadp *adapter,
    const core_machine_vadp_text_timing *timing)
{
    if (adapter == STD_NULL || !core_machine_vadp_valid_text_timing(timing)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    adapter->data.text_timing = *timing;
    adapter->data.raster_phase = timing->vertical_retrace_ticks;
    return TYPE_STATUS_OK;
}

type_status core_machine_vadp_configure_cga_memory(t_vadp *adapter, t_ram *memory)
{
    return adapter == STD_NULL || memory == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        core_machine_memory_register_device_provider(memory,
            CORE_MACHINE_VADP_VIDEO_BASE, CORE_MACHINE_VADP_VIDEO_BYTES,
            core_machine_vadp_cga_read, core_machine_vadp_cga_write,
            core_machine_vadp_cga_query, adapter);
}

type_status core_machine_vadp_configure_ega_sequencer(t_vadp *adapter,
    t_ram *memory, const core_machine_vadp_ega_sequencer_config *config)
{
    type_status status;
    type_virtual_address planar_vram = 0u;

    if (adapter == STD_NULL || memory == STD_NULL || config == STD_NULL ||
        config->aperture_base != CORE_MACHINE_VADP_EGA_APERTURE_BASE ||
        config->aperture_bytes != CORE_MACHINE_VADP_EGA_APERTURE_BYTES) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (adapter->data.ega_sequencer_configured) return TYPE_STATUS_INVALID_STATE;
    if (config->planar_ega) {
        planar_vram = (type_virtual_address)STD_CALLOC(1u,
            CORE_MACHINE_VADP_EGA_PLANES * CORE_MACHINE_VADP_EGA_PLANE_BYTES);
        if (planar_vram == 0u) return TYPE_STATUS_NO_MEMORY;
        status = core_machine_memory_register_device_provider_and_write_observer(memory,
            CORE_MACHINE_VADP_EGA_APERTURE_BASE,
            CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
            core_machine_vadp_ega_planar_read, core_machine_vadp_ega_planar_write,
            core_machine_vadp_ega_planar_query, adapter,
            core_machine_vadp_ega_write_observer);
        if (status != TYPE_STATUS_OK) {
            STD_FREE((C_VOID *)planar_vram);
            return status;
        }
        adapter->data.ega_planar_vram = planar_vram;
    } else {
        status = core_machine_memory_register_write_observer(memory,
            core_machine_vadp_ega_write_observer, adapter);
        if (status != TYPE_STATUS_OK) return status;
    }
    adapter->data.ega_sequencer = *config;
    adapter->data.ega_sequencer_configured = TYPE_TRUE;
    adapter->data.ega_planar_enabled = config->planar_ega;
    core_machine_vadp_reset_sequencer(adapter);
    return TYPE_STATUS_OK;
}

type_status core_machine_vadp_configure_ega_controllers(t_vadp *adapter,
    const core_machine_vadp_ega_controller_config *config)
{
    if (adapter == STD_NULL || config == STD_NULL ||
        !adapter->data.ega_sequencer_configured) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (adapter->data.ega_controller_configured) return TYPE_STATUS_INVALID_STATE;
    adapter->data.ega_controller = *config;
    core_machine_vadp_normalize_ega_controllers(&adapter->data.ega_controller);
    adapter->data.ega_controller_configured = TYPE_TRUE;
    core_machine_vadp_reset_ega_controllers(adapter);
    return TYPE_STATUS_OK;
}

C_INT core_machine_vadp_ega_aperture_contains(const t_vadp *adapter,
    type_unsigned_32 physical, STD_SIZE_T bytes)
{
    type_unsigned_64 aperture_end;
    type_unsigned_64 request_end;

    if (adapter == STD_NULL || !adapter->data.ega_sequencer_configured ||
        bytes == 0u) return TYPE_FALSE;
    {
        type_unsigned_32 aperture_base;
        type_unsigned_32 aperture_bytes;

        core_machine_vadp_active_ega_aperture(adapter, &aperture_base,
            &aperture_bytes);
        aperture_end = (type_unsigned_64)aperture_base + aperture_bytes;
        request_end = (type_unsigned_64)physical + bytes;
        return physical >= aperture_base && request_end <= aperture_end;
    }
}

C_INT core_machine_vadp_capture_text_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot)
{
    type_unsigned_16 row;
    type_unsigned_16 column;
    type_unsigned_16 start;
    type_unsigned_16 cursor;
    type_unsigned_16 relative_cursor;
    type_unsigned_16 start_byte;
    type_unsigned_16 columns;
    type_unsigned_16 rows;
    STD_SIZE_T visible_bytes;
    STD_SIZE_T first_bytes;
    type_unsigned_8 cells[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS * 2u];
    C_INT buffer_changed = TYPE_FALSE;
    C_INT cursor_changed;
    C_INT cursor_visible;

    if (adapter == STD_NULL || memory == STD_NULL || out_snapshot == STD_NULL) return TYPE_FALSE;
    columns = core_machine_vadp_text_columns(adapter);
    rows = core_machine_vadp_text_rows(adapter);
    if (columns == 0u || rows == 0u) return TYPE_FALSE;
    STD_MEMSET(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = CORE_MACHINE_DISPLAY_KIND_TEXT;
    start = core_machine_vadp_crtc_word(adapter, CORE_MACHINE_VADP_CRTC_START_HIGH);
    cursor = core_machine_vadp_crtc_word(adapter, CORE_MACHINE_VADP_CRTC_CURSOR_HIGH);
    visible_bytes = (STD_SIZE_T)columns * rows * 2u;
    start_byte = (type_unsigned_16)((start % (CORE_MACHINE_VADP_TEXT_BYTES / 2u)) * 2u);
    first_bytes = CORE_MACHINE_VADP_TEXT_BYTES - start_byte;
    if (first_bytes > visible_bytes) first_bytes = visible_bytes;
    if (core_machine_memory_read_physical(memory,
            CORE_MACHINE_VADP_TEXT_BASE + start_byte,
            (type_virtual_address)cells, first_bytes) != TYPE_STATUS_OK ||
        (first_bytes < visible_bytes && core_machine_memory_read_physical(memory,
            CORE_MACHINE_VADP_TEXT_BASE, (type_virtual_address)(cells + first_bytes),
            visible_bytes - first_bytes) != TYPE_STATUS_OK)) {
        return TYPE_FALSE;
    }
    out_snapshot->columns = columns;
    out_snapshot->rows = rows;
    out_snapshot->cursor_top = adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_CURSOR_TOP] & 0x1fu;
    out_snapshot->cursor_bottom = adapter->data.crtc[
        CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM] & 0x1fu;
    relative_cursor = (type_unsigned_16)((cursor - start) %
        (CORE_MACHINE_VADP_TEXT_BYTES / 2u));
    cursor_visible = (adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_TOP] &
        0x20u) == 0u && out_snapshot->cursor_top <= out_snapshot->cursor_bottom &&
        relative_cursor < columns * rows;
    out_snapshot->cursor_visible = cursor_visible;
    if (cursor_visible) {
        out_snapshot->cursor_x = (type_unsigned_8)(relative_cursor % columns);
        out_snapshot->cursor_y = (type_unsigned_8)(relative_cursor / columns);
    }
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind !=
        CORE_MACHINE_DISPLAY_KIND_TEXT || STD_MEMCMP(adapter->data.text_cells,
        cells, visible_bytes) != 0;
    if (buffer_changed) {
        STD_MEMCPY(adapter->data.text_cells, cells, visible_bytes);
        for (row = 0u; row < rows; ++row) {
            for (column = 0u; column < columns; ++column) {
                type_unsigned_16 index = (type_unsigned_16)(row * CORE_MACHINE_DISPLAY_MAX_COLUMNS + column);
                type_unsigned_16 cell = (type_unsigned_16)(row * columns + column);
                adapter->data.characters[index] = cells[(STD_SIZE_T)cell * 2u];
                adapter->data.attributes[index] = cells[(STD_SIZE_T)cell * 2u + 1u];
            }
        }
    }
    STD_MEMCPY(out_snapshot->characters, adapter->data.characters,
        sizeof(out_snapshot->characters));
    STD_MEMCPY(out_snapshot->attributes, adapter->data.attributes,
        sizeof(out_snapshot->attributes));
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
    adapter->data.captured_cursor_visible = out_snapshot->cursor_visible;
    adapter->data.captured = TYPE_TRUE;
    adapter->data.captured_kind = CORE_MACHINE_DISPLAY_KIND_TEXT;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = cursor_changed;
    return TYPE_TRUE;
}

static C_INT core_machine_vadp_capture_graphics_snapshot(t_vadp *adapter,
    t_ram *memory, core_machine_display_snapshot *out_snapshot)
{
    type_unsigned_8 bytes[CORE_MACHINE_VADP_VIDEO_BYTES];
    type_unsigned_16 y;
    type_unsigned_16 x;
    C_INT buffer_changed;

    if (adapter == STD_NULL || memory == STD_NULL || out_snapshot == STD_NULL ||
        !core_machine_vadp_is_graphics_mode(adapter)) {
        return TYPE_FALSE;
    }
    if (core_machine_memory_read_physical(memory, CORE_MACHINE_VADP_VIDEO_BASE,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK) {
        return TYPE_FALSE;
    }
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind !=
        CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4 ||
        adapter->data.captured_mode_control != adapter->data.mode_control ||
        adapter->data.captured_color_select != adapter->data.color_select || STD_MEMCMP(
        adapter->data.graphics_bytes, bytes, sizeof(bytes)) != 0;
    if (buffer_changed) {
        STD_MEMCPY(adapter->data.graphics_bytes, bytes, sizeof(bytes));
    }
    STD_MEMSET(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4;
    out_snapshot->pixel_width = CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH;
    out_snapshot->pixel_height = CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT;
    core_machine_vadp_graphics_palette(adapter, out_snapshot->palette_rgb);
    for (y = 0u; y < CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT; ++y) {
        type_unsigned_32 row_offset = (type_unsigned_32)(y & 1u) *
            CORE_MACHINE_VADP_GRAPHICS_ODD_ROW_OFFSET + (type_unsigned_32)(y >> 1) *
            CORE_MACHINE_VADP_GRAPHICS_BYTES_PER_ROW;
        for (x = 0u; x < CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH; ++x) {
            type_unsigned_8 byte = bytes[row_offset + (x >> 2)];
            out_snapshot->pixels[(type_unsigned_32)y * CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH + x] =
                (type_unsigned_8)((byte >> (6u - 2u * (x & 3u))) & 0x03u);
        }
    }
    adapter->data.captured = TYPE_TRUE;
    adapter->data.captured_kind = CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4;
    adapter->data.captured_mode_control = adapter->data.mode_control;
    adapter->data.captured_color_select = adapter->data.color_select;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = TYPE_FALSE;
    return TYPE_TRUE;
}

static C_INT core_machine_vadp_capture_high_res_graphics_snapshot(t_vadp *adapter,
    t_ram *memory, core_machine_display_snapshot *out_snapshot)
{
    type_unsigned_8 bytes[CORE_MACHINE_VADP_VIDEO_BYTES];
    type_unsigned_16 y;
    type_unsigned_16 x;
    C_INT buffer_changed;

    if (adapter == STD_NULL || memory == STD_NULL || out_snapshot == STD_NULL ||
        !core_machine_vadp_is_high_res_graphics_mode(adapter)) return TYPE_FALSE;
    if (core_machine_memory_read_physical(memory, CORE_MACHINE_VADP_VIDEO_BASE,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK) {
        return TYPE_FALSE;
    }
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind !=
        CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2 ||
        adapter->data.captured_mode_control != adapter->data.mode_control ||
        adapter->data.captured_color_select != adapter->data.color_select ||
        STD_MEMCMP(adapter->data.graphics_bytes, bytes, sizeof(bytes)) != 0;
    if (buffer_changed) STD_MEMCPY(adapter->data.graphics_bytes, bytes, sizeof(bytes));
    STD_MEMSET(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2;
    out_snapshot->pixel_width = CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH;
    out_snapshot->pixel_height = CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT;
    core_machine_vadp_high_res_palette(adapter, out_snapshot->palette_rgb);
    for (y = 0u; y < CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT; ++y) {
        type_unsigned_32 row_offset = (type_unsigned_32)(y & 1u) *
            CORE_MACHINE_VADP_GRAPHICS_ODD_ROW_OFFSET + (type_unsigned_32)(y >> 1) *
            CORE_MACHINE_VADP_GRAPHICS_BYTES_PER_ROW;

        for (x = 0u; x < CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH; ++x) {
            type_unsigned_8 byte = bytes[row_offset + (x >> 3u)];

            out_snapshot->pixels[(type_unsigned_32)y * CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH + x] =
                (type_unsigned_8)((byte >> (7u - (x & 7u))) & 0x01u);
        }
    }
    adapter->data.captured = TYPE_TRUE;
    adapter->data.captured_kind = CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2;
    adapter->data.captured_mode_control = adapter->data.mode_control;
    adapter->data.captured_color_select = adapter->data.color_select;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = TYPE_FALSE;
    return TYPE_TRUE;
}

static C_INT core_machine_vadp_capture_ega_planar_snapshot(t_vadp *adapter,
    core_machine_display_snapshot *out_snapshot)
{
    core_machine_display_kind kind;
    type_unsigned_16 width;
    type_unsigned_16 height;
    type_unsigned_16 row_bytes;
    type_unsigned_32 start_byte;
    type_unsigned_16 y;
    type_unsigned_16 x;
    C_INT buffer_changed;

    if (adapter == STD_NULL || out_snapshot == STD_NULL ||
        !core_machine_vadp_ega_planar_active(adapter)) {
        return TYPE_FALSE;
    }
    kind = core_machine_vadp_ega_display_kind(adapter);
    width = kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ?
        CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH : CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH;
    height = kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ?
        CORE_MACHINE_DISPLAY_EGA_HIGH_RES_HEIGHT : CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT;
    row_bytes = kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ?
        CORE_MACHINE_VADP_EGA_640X350_ROW_BYTES :
        CORE_MACHINE_VADP_EGA_320X200_ROW_BYTES;
    /* EGA CRTC start is a word address; 64 KiB plane addressing wraps. */
    start_byte = kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ?
        ((type_unsigned_32)core_machine_vadp_crtc_word(adapter,
            CORE_MACHINE_VADP_CRTC_START_HIGH) * 2u) &
            (CORE_MACHINE_VADP_EGA_PLANE_BYTES - 1u) : 0u;
    buffer_changed = !adapter->data.captured || adapter->data.captured_kind != kind ||
        adapter->data.captured_ega_dirty_generation != adapter->data.dirty_generation;
    STD_MEMSET(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = kind;
    out_snapshot->pixel_width = width;
    out_snapshot->pixel_height = height;
    for (x = 0u; x < CORE_MACHINE_DISPLAY_PALETTE_ENTRIES; ++x) {
        type_unsigned_8 enabled_index = (type_unsigned_8)(x & adapter->data.attribute[18]);
        out_snapshot->palette_rgb[x] = core_machine_vadp_rgbi_color(
            adapter->data.attribute[enabled_index] & 0x0fu);
    }
    for (y = 0u; y < height; ++y) {
        for (x = 0u; x < width; ++x) {
            type_unsigned_32 offset = (start_byte + (type_unsigned_32)y * row_bytes +
                (x >> 3)) & (CORE_MACHINE_VADP_EGA_PLANE_BYTES - 1u);
            type_unsigned_8 bit = (type_unsigned_8)(0x80u >> (x & 7u));
            type_unsigned_8 plane;
            type_unsigned_8 pixel = 0u;

            for (plane = 0u; plane < CORE_MACHINE_VADP_EGA_PLANES; ++plane) {
                const type_unsigned_8 *source = (const type_unsigned_8 *)adapter->data.ega_planar_vram +
                    (STD_SIZE_T)plane * CORE_MACHINE_VADP_EGA_PLANE_BYTES;
                if ((source[offset] & bit) != 0u) pixel |= (type_unsigned_8)(1u << plane);
            }
            out_snapshot->pixels[(type_unsigned_32)y * width + x] = pixel;
        }
    }
    adapter->data.captured = TYPE_TRUE;
    adapter->data.captured_kind = kind;
    adapter->data.captured_ega_dirty_generation = adapter->data.dirty_generation;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = TYPE_FALSE;
    return TYPE_TRUE;
}

C_INT core_machine_vadp_capture_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot)
{
    if (core_machine_vadp_ega_planar_active(adapter)) {
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
