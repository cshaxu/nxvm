/* Copyright 2012-2014 Neko. */

/* QDCGA implements the default-profile text-mode INT 10h services. */

#include "type.h"

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

static type_unsigned_16 qdcga_read16(core_machine_firmware_context *firmware,
    type_unsigned_32 address)
{
    type_unsigned_16 value = 0u;
    (C_VOID)core_machine_firmware_memory_read(firmware, address, &value,
        sizeof(value));
    return value;
}

static C_VOID qdcga_write8(core_machine_firmware_context *firmware, type_unsigned_32 address,
    type_unsigned_8 value)
{
    (C_VOID)core_machine_firmware_memory_write(firmware, address, &value,
        sizeof(value));
}

static C_VOID qdcga_write16(core_machine_firmware_context *firmware, type_unsigned_32 address,
    type_unsigned_16 value)
{
    (C_VOID)core_machine_firmware_memory_write(firmware, address, &value,
        sizeof(value));
}

static type_unsigned_16 qdcga_columns(core_machine_firmware_context *firmware)
{
    return qdcga_read16(firmware, QDCGA_BDA_COLUMNS);
}

static type_unsigned_16 qdcga_page_size(core_machine_firmware_context *firmware)
{
    return qdcga_read16(firmware, QDCGA_BDA_PAGE_SIZE);
}

static type_unsigned_16 qdcga_cursor_address(core_machine_firmware_context *firmware,
    type_unsigned_8 page, type_unsigned_8 row, type_unsigned_8 column)
{
    return (type_unsigned_16)((type_unsigned_32)page * qdcga_page_size(firmware) / 2u +
        (type_unsigned_32)row * qdcga_columns(firmware) + column);
}

static C_VOID qdcga_set_cursor(core_machine_firmware_context *firmware, type_unsigned_8 page,
    type_unsigned_8 row, type_unsigned_8 column)
{
    qdcga_write8(firmware, QDCGA_BDA_CURSOR + (type_unsigned_32)page * 2u, column);
    qdcga_write8(firmware, QDCGA_BDA_CURSOR + (type_unsigned_32)page * 2u + 1u, row);
    type_unsigned_16 address = qdcga_cursor_address(firmware, page, row, column);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d4u,
        0x0eu);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d5u,
        address >> 8);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d4u,
        0x0fu);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d5u,
        address & 0xffu);
}

C_VOID vm_profile_default_cga_reset(vm_profile_default_context *profile,
    core_machine_firmware_context *firmware)
{
    if (profile == STD_NULL) return;
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d8u,
        0x05u);
    qdcga_write16(firmware, QDCGA_BDA_COLUMNS, 80u);
    qdcga_write16(firmware, QDCGA_BDA_PAGE_SIZE, 0x1000u);
    qdcga_write8(firmware, QDCGA_BDA_PAGE, 0u);
    qdcga_write8(firmware, QDCGA_BDA_MODE, 3u);
    qdcga_write8(firmware, QDCGA_BDA_CURSOR_TOP, 6u);
    qdcga_write8(firmware, QDCGA_BDA_CURSOR_BOTTOM, 7u);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d4u,
        0x0au);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d5u,
        6u);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d4u,
        0x0bu);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d5u,
        7u);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d4u,
        0x0cu);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d5u,
        0u);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d4u,
        0x0du);
    (C_VOID)core_machine_firmware_port_write(firmware, 0x03d5u,
        0u);
    qdcga_set_cursor(firmware, 0u, 5u, 0u);
}
