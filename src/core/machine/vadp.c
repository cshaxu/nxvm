/* Copyright 2012-2014 Neko. */

/* Core-owned CGA text-controller state. Host presentation is outside core. */

#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

#define CORE_MACHINE_VADP_PORT_CRTC_INDEX 0x03d4u
#define CORE_MACHINE_VADP_PORT_CRTC_DATA 0x03d5u
#define CORE_MACHINE_VADP_PORT_MODE 0x03d8u
#define CORE_MACHINE_VADP_PORT_COLOR 0x03d9u
#define CORE_MACHINE_VADP_PORT_STATUS 0x03dau
#define CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE 0x01u
#define CORE_MACHINE_VADP_STATUS_VERTICAL_RETRACE 0x08u
#define CORE_MACHINE_VADP_DEFAULT_ACTIVE_DISPLAY_TICKS 48u
#define CORE_MACHINE_VADP_DEFAULT_HORIZONTAL_BLANK_TICKS 8u
#define CORE_MACHINE_VADP_DEFAULT_VERTICAL_RETRACE_TICKS 8u
#define CORE_MACHINE_VADP_CRTC_CURSOR_TOP 0x0au
#define CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM 0x0bu
#define CORE_MACHINE_VADP_CRTC_START_HIGH 0x0cu
#define CORE_MACHINE_VADP_CRTC_START_LOW 0x0du
#define CORE_MACHINE_VADP_CRTC_CURSOR_HIGH 0x0eu
#define CORE_MACHINE_VADP_CRTC_CURSOR_LOW 0x0fu

static C_INT core_machine_vadp_supported_crtc_index(uint8_t index)
{
    return index >= CORE_MACHINE_VADP_CRTC_CURSOR_TOP &&
        index <= CORE_MACHINE_VADP_CRTC_CURSOR_LOW;
}

static uint16_t core_machine_vadp_crtc_word(const t_vadp *adapter,
    uint8_t high_index)
{
    return (uint16_t)(((uint16_t)adapter->data.crtc[high_index] << 8) |
        adapter->data.crtc[high_index + 1u]);
}

static C_VOID core_machine_vadp_mark_dirty(t_vadp *adapter)
{
    if (adapter != STD_NULL) ++adapter->data.dirty_generation;
}

static uint32_t core_machine_vadp_raster_period(
    const core_machine_vadp_text_timing *timing)
{
    return timing->active_display_ticks + timing->horizontal_blank_ticks +
        timing->vertical_retrace_ticks;
}

static C_INT core_machine_vadp_valid_text_timing(
    const core_machine_vadp_text_timing *timing)
{
    uint32_t period;

    if (timing == STD_NULL || timing->active_display_ticks == 0u ||
        timing->vertical_retrace_ticks == 0u) {
        return TYPE_FALSE;
    }
    period = core_machine_vadp_raster_period(timing);
    return period >= timing->active_display_ticks &&
        period >= timing->horizontal_blank_ticks &&
        period >= timing->vertical_retrace_ticks;
}

static uint8_t core_machine_vadp_status(const t_vadp *adapter)
{
    uint32_t vertical_end;
    uint32_t display_end;

    if (adapter == STD_NULL) return 0u;
    vertical_end = adapter->data.text_timing.vertical_retrace_ticks;
    display_end = vertical_end + adapter->data.text_timing.active_display_ticks;
    if (adapter->data.raster_phase < vertical_end) {
        return CORE_MACHINE_VADP_STATUS_VERTICAL_RETRACE;
    }
    if (adapter->data.raster_phase < display_end) {
        return CORE_MACHINE_VADP_STATUS_DISPLAY_ENABLE;
    }
    return 0u;
}

static C_VOID core_machine_vadp_read_crtc_index(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    (C_VOID)port_id;
    if (port != STD_NULL && owner != STD_NULL) {
        port->data.ioByte = ((t_vadp *)owner)->data.crtc_index;
    }
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
    port->data.ioByte = core_machine_vadp_supported_crtc_index(
        adapter->data.crtc_index) ?
        adapter->data.crtc[adapter->data.crtc_index] : 0u;
}

static C_VOID core_machine_vadp_write_crtc_data(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_vadp *adapter = (t_vadp *)owner;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL ||
        !core_machine_vadp_supported_crtc_index(adapter->data.crtc_index)) {
        return;
    }
    if (adapter->data.crtc[adapter->data.crtc_index] != port->data.ioByte) {
        adapter->data.crtc[adapter->data.crtc_index] = port->data.ioByte;
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
    uint8_t value;

    (C_VOID)port_id;
    if (port == STD_NULL || adapter == STD_NULL) return;
    value = port->data.ioByte;
    /* Graphics selection is deliberately inert in the text-only slice. */
    if ((value & 0x02u) != 0u) return;
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
    (C_VOID)port_id;
    if (port != STD_NULL && owner != STD_NULL) {
        port->data.ioByte = core_machine_vadp_status((const t_vadp *)owner);
    }
}

static C_VOID core_machine_vadp_register_ports(t_vadp *adapter, t_port *port)
{
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX,
        core_machine_vadp_read_crtc_index, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX,
        core_machine_vadp_write_crtc_index, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_CRTC_DATA,
        core_machine_vadp_read_crtc_data, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_CRTC_DATA,
        core_machine_vadp_write_crtc_data, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_MODE,
        core_machine_vadp_read_mode, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_MODE,
        core_machine_vadp_write_mode, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_COLOR,
        core_machine_vadp_read_color, adapter);
    core_machine_port_add_write(port, CORE_MACHINE_VADP_PORT_COLOR,
        core_machine_vadp_write_color, adapter);
    core_machine_port_add_read(port, CORE_MACHINE_VADP_PORT_STATUS,
        core_machine_vadp_read_status, adapter);
}

C_VOID core_machine_vadp_initialize(t_vadp *adapter, t_port *port)
{
    if (adapter == STD_NULL || port == STD_NULL) return;
    STD_MEMSET(adapter, TYPE_ZERO_8, sizeof(*adapter));
    core_machine_vadp_register_ports(adapter, port);
    core_machine_vadp_reset(adapter);
}

C_VOID core_machine_vadp_reset(t_vadp *adapter)
{
    core_machine_vadp_text_timing timing;

    if (adapter == STD_NULL) return;
    timing = adapter->data.text_timing;
    if (!core_machine_vadp_valid_text_timing(&timing)) {
        timing.active_display_ticks = CORE_MACHINE_VADP_DEFAULT_ACTIVE_DISPLAY_TICKS;
        timing.horizontal_blank_ticks = CORE_MACHINE_VADP_DEFAULT_HORIZONTAL_BLANK_TICKS;
        timing.vertical_retrace_ticks = CORE_MACHINE_VADP_DEFAULT_VERTICAL_RETRACE_TICKS;
    }
    STD_MEMSET(&adapter->data, TYPE_ZERO_8, sizeof(adapter->data));
    adapter->data.mode_control = 0x05u;
    adapter->data.text_timing = timing;
    adapter->data.raster_phase = timing.vertical_retrace_ticks;
    adapter->data.columns = 80u;
    adapter->data.rows = 25u;
    adapter->data.color_enabled = TYPE_TRUE;
    adapter->data.cursor_top = 6u;
    adapter->data.cursor_bottom = 7u;
    adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_TOP] = 6u;
    adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM] = 7u;
    adapter->data.dirty_generation = 1u;
}

C_VOID core_machine_vadp_advance(t_vadp *adapter, t_ram *memory,
    uint64_t elapsed_ticks)
{
    uint32_t period;

    (C_VOID)memory;
    if (adapter == STD_NULL) return;
    period = core_machine_vadp_raster_period(&adapter->data.text_timing);
    if (period == 0u) return;
    adapter->data.raster_phase = (uint32_t)((adapter->data.raster_phase +
        elapsed_ticks % period) % period);
}

C_VOID core_machine_vadp_finalize(t_vadp *adapter)
{
    (C_VOID)adapter;
}

type_status core_machine_vadp_configure_text(t_vadp *adapter, uint8_t mode,
    uint16_t columns, uint16_t rows, C_INT color_enabled)
{
    uint8_t mode_control;

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

C_VOID core_machine_vadp_set_cursor_shape(t_vadp *adapter, uint8_t top,
    uint8_t bottom)
{
    if (adapter == STD_NULL) return;
    adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_TOP] = top;
    adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM] = bottom;
    adapter->data.cursor_top = top;
    adapter->data.cursor_bottom = bottom;
    core_machine_vadp_mark_dirty(adapter);
}

C_VOID core_machine_vadp_set_cursor_address(t_vadp *adapter, uint16_t address)
{
    if (adapter == STD_NULL) return;
    adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_HIGH] = (uint8_t)(address >> 8);
    adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_LOW] = (uint8_t)address;
    adapter->data.cursor_address = address;
    core_machine_vadp_mark_dirty(adapter);
}

C_VOID core_machine_vadp_set_display_start(t_vadp *adapter, uint16_t address)
{
    if (adapter == STD_NULL) return;
    adapter->data.crtc[CORE_MACHINE_VADP_CRTC_START_HIGH] = (uint8_t)(address >> 8);
    adapter->data.crtc[CORE_MACHINE_VADP_CRTC_START_LOW] = (uint8_t)address;
    core_machine_vadp_mark_dirty(adapter);
}

C_INT core_machine_vadp_capture_text_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot)
{
    uint16_t row;
    uint16_t column;
    uint16_t start;
    uint16_t cursor;
    uint16_t start_byte;
    STD_SIZE_T visible_bytes;
    STD_SIZE_T first_bytes;
    uint8_t cells[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS * 2u];
    C_INT buffer_changed = TYPE_FALSE;
    C_INT cursor_changed;

    if (adapter == STD_NULL || memory == STD_NULL || out_snapshot == STD_NULL ||
        adapter->data.columns == 0u || adapter->data.rows == 0u) return TYPE_FALSE;
    STD_MEMSET(out_snapshot, 0, sizeof(*out_snapshot));
    start = core_machine_vadp_crtc_word(adapter, CORE_MACHINE_VADP_CRTC_START_HIGH);
    cursor = core_machine_vadp_crtc_word(adapter, CORE_MACHINE_VADP_CRTC_CURSOR_HIGH);
    visible_bytes = (STD_SIZE_T)adapter->data.columns * adapter->data.rows * 2u;
    start_byte = (uint16_t)((start % (CORE_MACHINE_VADP_TEXT_BYTES / 2u)) * 2u);
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
    out_snapshot->columns = adapter->data.columns;
    out_snapshot->rows = adapter->data.rows;
    out_snapshot->cursor_top = adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_TOP];
    out_snapshot->cursor_bottom = adapter->data.crtc[CORE_MACHINE_VADP_CRTC_CURSOR_BOTTOM];
    out_snapshot->cursor_visible = out_snapshot->cursor_top < out_snapshot->cursor_bottom;
    out_snapshot->cursor_x = (uint8_t)((cursor - start) / adapter->data.columns);
    out_snapshot->cursor_y = (uint8_t)((cursor - start) % adapter->data.columns);
    buffer_changed = !adapter->data.captured || STD_MEMCMP(adapter->data.text_cells,
        cells, visible_bytes) != 0;
    if (buffer_changed) {
        STD_MEMCPY(adapter->data.text_cells, cells, visible_bytes);
        for (row = 0u; row < adapter->data.rows; ++row) {
            for (column = 0u; column < adapter->data.columns; ++column) {
                uint16_t index = (uint16_t)(row * CORE_MACHINE_DISPLAY_MAX_COLUMNS + column);
                uint16_t cell = (uint16_t)(row * adapter->data.columns + column);
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
        adapter->data.captured_cursor_address != cursor;
    if (buffer_changed || cursor_changed) core_machine_vadp_mark_dirty(adapter);
    adapter->data.cursor_top = out_snapshot->cursor_top;
    adapter->data.cursor_bottom = out_snapshot->cursor_bottom;
    adapter->data.cursor_address = cursor;
    adapter->data.captured_cursor_top = out_snapshot->cursor_top;
    adapter->data.captured_cursor_bottom = out_snapshot->cursor_bottom;
    adapter->data.captured_cursor_address = cursor;
    adapter->data.captured = TYPE_TRUE;
    out_snapshot->buffer_changed = buffer_changed;
    out_snapshot->cursor_changed = cursor_changed;
    return TYPE_TRUE;
}
