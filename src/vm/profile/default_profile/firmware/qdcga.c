/* Copyright 2012-2014 Neko. */

/* QDCGA implements the default-profile text-mode INT 10h services. */

#include "type.h"

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

static C_VOID qdcga_set_cursor(vm_profile_default_context *profile, uint8_t page,
    uint8_t row, uint8_t column)
{
    qdcga_write8(profile, QDCGA_BDA_CURSOR + (uint32_t)page * 2u, column);
    qdcga_write8(profile, QDCGA_BDA_CURSOR + (uint32_t)page * 2u + 1u, row);
    core_machine_profile_binding_set_video_cursor_address(&profile->binding,
        qdcga_cursor_address(profile, page, row, column));
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
