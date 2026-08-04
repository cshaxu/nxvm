/* Copyright 2012-2014 Neko. */

/* QDCGA implements the default-profile text-mode INT 10h services. */

#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/memory.h"

#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/firmware/qdcga.h"

#define QDCGA_BDA_MODE 0x0449u
#define QDCGA_BDA_COLUMNS 0x044au
#define QDCGA_BDA_PAGE_SIZE 0x044cu
#define QDCGA_BDA_PAGE_OFFSET 0x044eu
#define QDCGA_BDA_CURSOR 0x0450u
#define QDCGA_BDA_CURSOR_BOTTOM 0x0460u
#define QDCGA_BDA_CURSOR_TOP 0x0461u
#define QDCGA_BDA_PAGE 0x0462u
#define QDCGA_TEXT_BASE 0x000b8000u
#define QDCGA_MAX_PAGES 8u
#define QDCGA_TEXT_ROWS 25u

#define profile_cpu (*vm_profile_default_context_execution(profile)->cpu)

static uint8_t qdcga_read8(vm_profile_default_context *profile, uint32_t address)
{
    uint8_t value = 0u;
    (C_VOID)core_machine_memory_read_physical(vm_profile_default_context_memory(profile), address,
        (type_virtual_address)&value, sizeof(value));
    return value;
}

static uint16_t qdcga_read16(vm_profile_default_context *profile, uint32_t address)
{
    uint16_t value = 0u;
    (C_VOID)core_machine_memory_read_physical(vm_profile_default_context_memory(profile), address,
        (type_virtual_address)&value, sizeof(value));
    return value;
}

static C_VOID qdcga_write8(vm_profile_default_context *profile, uint32_t address,
    uint8_t value)
{
    (C_VOID)core_machine_memory_write_physical(vm_profile_default_context_memory(profile), address,
        (type_virtual_address)&value, sizeof(value));
}

static C_VOID qdcga_write16(vm_profile_default_context *profile, uint32_t address,
    uint16_t value)
{
    (C_VOID)core_machine_memory_write_physical(vm_profile_default_context_memory(profile), address,
        (type_virtual_address)&value, sizeof(value));
}

static uint16_t qdcga_columns(vm_profile_default_context *profile)
{
    return qdcga_read16(profile, QDCGA_BDA_COLUMNS);
}

static uint16_t qdcga_page_size(vm_profile_default_context *profile)
{
    return qdcga_read16(profile, QDCGA_BDA_PAGE_SIZE);
}

static uint16_t qdcga_rows(C_VOID)
{
    return QDCGA_TEXT_ROWS;
}

static uint16_t qdcga_cursor_address(vm_profile_default_context *profile,
    uint8_t page, uint8_t row, uint8_t column)
{
    return (uint16_t)((uint32_t)page * qdcga_page_size(profile) / 2u +
        (uint32_t)row * qdcga_columns(profile) + column);
}

static uint32_t qdcga_cell_address(vm_profile_default_context *profile,
    uint8_t page, uint16_t row, uint16_t column)
{
    return QDCGA_TEXT_BASE + (uint32_t)page * qdcga_page_size(profile) +
        ((uint32_t)row * qdcga_columns(profile) + column) * 2u;
}

static uint8_t qdcga_cursor_column(vm_profile_default_context *profile, uint8_t page)
{
    return qdcga_read8(profile, QDCGA_BDA_CURSOR + (uint32_t)page * 2u);
}

static uint8_t qdcga_cursor_row(vm_profile_default_context *profile, uint8_t page)
{
    return qdcga_read8(profile, QDCGA_BDA_CURSOR + (uint32_t)page * 2u + 1u);
}

static C_VOID qdcga_set_cursor(vm_profile_default_context *profile, uint8_t page,
    uint8_t row, uint8_t column)
{
    qdcga_write8(profile, QDCGA_BDA_CURSOR + (uint32_t)page * 2u, column);
    qdcga_write8(profile, QDCGA_BDA_CURSOR + (uint32_t)page * 2u + 1u, row);
    core_machine_profile_binding_set_video_cursor_address(&profile->binding,
        qdcga_cursor_address(profile, page, row, column));
}

static C_VOID qdcga_clear_page(vm_profile_default_context *profile, uint8_t page)
{
    uint16_t row;
    uint16_t column;

    for (row = 0u; row < qdcga_rows(); ++row) {
        for (column = 0u; column < qdcga_columns(profile); ++column) {
            uint32_t address = qdcga_cell_address(profile, page, row, column);
            qdcga_write8(profile, address, 0u);
            qdcga_write8(profile, address + 1u, 0u);
        }
    }
}

static C_VOID qdcga_scroll_up(vm_profile_default_context *profile, uint8_t page)
{
    uint16_t row;
    uint16_t column;

    for (row = 1u; row < qdcga_rows(); ++row) {
        for (column = 0u; column < qdcga_columns(profile); ++column) {
            uint32_t destination = qdcga_cell_address(profile, page, row - 1u, column);
            uint32_t source = qdcga_cell_address(profile, page, row, column);
            qdcga_write8(profile, destination, qdcga_read8(profile, source));
            qdcga_write8(profile, destination + 1u, qdcga_read8(profile, source + 1u));
        }
    }
    for (column = 0u; column < qdcga_columns(profile); ++column) {
        uint32_t address = qdcga_cell_address(profile, page,
            qdcga_rows() - 1u, column);
        qdcga_write8(profile, address, 0u);
        qdcga_write8(profile, address + 1u, 0u);
    }
}

static C_VOID qdcga_put_character(vm_profile_default_context *profile, uint8_t value,
    uint8_t attribute, uint8_t page, C_INT move)
{
    uint8_t row = qdcga_cursor_row(profile, page);
    uint8_t column = qdcga_cursor_column(profile, page);
    uint8_t original_row = row;
    uint8_t original_column = column;

    if (value == 0x08u) {
        if (column != 0u) --column;
        else if (row != 0u) {
            --row;
            column = (uint8_t)(qdcga_columns(profile) - 1u);
        }
    } else if (value == 0x0au) {
        if (row + 1u < qdcga_rows()) ++row;
        else qdcga_scroll_up(profile, page);
    } else if (value == 0x0du) {
        column = 0u;
    } else if (value != 0x07u) {
        uint32_t address = qdcga_cell_address(profile, page, row, column);
        qdcga_write8(profile, address, value);
        qdcga_write8(profile, address + 1u, attribute);
        if (column + 1u < qdcga_columns(profile)) ++column;
        else {
            column = 0u;
            if (row + 1u < qdcga_rows()) ++row;
            else qdcga_scroll_up(profile, page);
        }
    }
    qdcga_set_cursor(profile, page, move ? row : original_row,
        move ? column : original_column);
}

static C_VOID qdcga_set_mode(vm_profile_default_context *profile)
{
    uint8_t mode = profile_cpu.data.al;
    uint16_t columns = mode == 0u || mode == 1u ? 40u : 80u;

    if (core_machine_profile_binding_configure_text_video(&profile->binding,
            mode, columns, qdcga_rows(), mode != 0u && mode != 2u) !=
        TYPE_STATUS_OK) {
        return;
    }

    qdcga_write8(profile, QDCGA_BDA_MODE, mode);
    qdcga_write8(profile, QDCGA_BDA_PAGE, 0u);
    qdcga_write16(profile, QDCGA_BDA_PAGE_OFFSET, 0u);
    qdcga_write16(profile, QDCGA_BDA_COLUMNS, columns);
    qdcga_write16(profile, QDCGA_BDA_PAGE_SIZE, columns * qdcga_rows() * 2u);
    core_machine_profile_binding_set_video_display_start(&profile->binding, 0u);
    qdcga_set_cursor(profile, 0u, 0u, 0u);
    qdcga_clear_page(profile, 0u);
    core_machine_display_notify_mode_changed_to(profile->display_provider);
}

C_VOID vm_profile_default_cga_handle_int10(vm_profile_default_context *profile)
{
    uint8_t page = profile_cpu.data.bh;
    uint16_t index;

    switch (profile_cpu.data.ah) {
    case 0x00u: qdcga_set_mode(profile); break;
    case 0x01u:
        qdcga_write8(profile, QDCGA_BDA_CURSOR_TOP, profile_cpu.data.ch);
        qdcga_write8(profile, QDCGA_BDA_CURSOR_BOTTOM, profile_cpu.data.cl);
        core_machine_profile_binding_set_video_cursor_shape(&profile->binding,
            profile_cpu.data.ch, profile_cpu.data.cl);
        break;
    case 0x02u: qdcga_set_cursor(profile, page, profile_cpu.data.dh, profile_cpu.data.dl); break;
    case 0x03u:
        profile_cpu.data.dh = qdcga_cursor_row(profile, page);
        profile_cpu.data.dl = qdcga_cursor_column(profile, page);
        profile_cpu.data.ch = qdcga_read8(profile, QDCGA_BDA_CURSOR_TOP);
        profile_cpu.data.cl = qdcga_read8(profile, QDCGA_BDA_CURSOR_BOTTOM);
        break;
    case 0x05u:
        qdcga_write8(profile, QDCGA_BDA_PAGE, profile_cpu.data.al);
        qdcga_write16(profile, QDCGA_BDA_PAGE_OFFSET,
            (uint16_t)(profile_cpu.data.al * qdcga_page_size(profile)));
        core_machine_profile_binding_set_video_display_start(&profile->binding,
            (uint16_t)((uint32_t)profile_cpu.data.al * qdcga_page_size(profile) / 2u));
        break;
    case 0x06u:
    case 0x07u:
        qdcga_clear_page(profile, qdcga_read8(profile, QDCGA_BDA_PAGE));
        break;
    case 0x08u: {
        uint32_t address = qdcga_cell_address(profile, page,
            qdcga_cursor_row(profile, page), qdcga_cursor_column(profile, page));
        profile_cpu.data.al = qdcga_read8(profile, address);
        profile_cpu.data.ah = qdcga_read8(profile, address + 1u);
        break;
    }
    case 0x09u:
    case 0x0au:
    case 0x0eu:
        for (index = 0u; index < (profile_cpu.data.ah == 0x09u ? profile_cpu.data.cx : 1u); ++index) {
            qdcga_put_character(profile, profile_cpu.data.al,
                profile_cpu.data.ah == 0x0eu ? 0x0fu : profile_cpu.data.bl,
                page, profile_cpu.data.ah != 0x09u);
        }
        break;
    case 0x0fu:
        profile_cpu.data.ah = (uint8_t)qdcga_columns(profile);
        profile_cpu.data.al = qdcga_read8(profile, QDCGA_BDA_MODE);
        profile_cpu.data.bh = qdcga_read8(profile, QDCGA_BDA_PAGE);
        break;
    case 0x11u:
        if (profile_cpu.data.al == 0x30u) {
            uint32_t vector = profile_cpu.data.bh == 0u ? 0x001fu * 4u : 0x0043u * 4u;
            profile_cpu.data.bp = qdcga_read16(profile, vector);
            profile_cpu.data.es.selector = qdcga_read16(profile, vector + 2u);
            profile_cpu.data.cx = 0x0010u;
            profile_cpu.data.dl = (uint8_t)(qdcga_rows() - 1u);
        }
        break;
    case 0x12u:
        if (profile_cpu.data.bl == 0x10u) {
            profile_cpu.data.bh = 0u; profile_cpu.data.bl = 3u; profile_cpu.data.cl = 7u;
        }
        break;
    case 0x13u:
        for (index = 0u; index < profile_cpu.data.cx; ++index) {
            uint8_t value = 0u;
            (C_VOID)core_machine_memory_read_real_from(vm_profile_default_context_memory(profile),
                profile_cpu.data.es.selector, (uint16_t)(profile_cpu.data.bp + index),
                &value, sizeof(value));
            qdcga_put_character(profile, value, profile_cpu.data.bl, page, 1);
        }
        break;
    case 0x1au:
        if (profile_cpu.data.al == 0u) {
            profile_cpu.data.al = 0x1au; profile_cpu.data.bh = 0u; profile_cpu.data.bl = 8u;
        }
        break;
    default: break;
    }
}

C_VOID vm_profile_default_cga_reset(vm_profile_default_context *profile)
{
    if (profile == STD_NULL || core_machine_profile_binding_configure_text_video(
            &profile->binding, 3u, 80u, qdcga_rows(), TYPE_TRUE) != TYPE_STATUS_OK) {
        return;
    }
    qdcga_write16(profile, QDCGA_BDA_COLUMNS, 80u);
    qdcga_write16(profile, QDCGA_BDA_PAGE_SIZE, 0x1000u);
    qdcga_write8(profile, QDCGA_BDA_PAGE, 0u);
    qdcga_write8(profile, QDCGA_BDA_MODE, 3u);
    qdcga_write8(profile, QDCGA_BDA_CURSOR_TOP, 6u);
    qdcga_write8(profile, QDCGA_BDA_CURSOR_BOTTOM, 7u);
    core_machine_profile_binding_set_video_cursor_shape(&profile->binding, 6u, 7u);
    core_machine_profile_binding_set_video_display_start(&profile->binding, 0u);
    qdcga_set_cursor(profile, 0u, 5u, 0u);
}
#undef profile_cpu
