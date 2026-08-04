/* Copyright 2012-2014 Neko. */

/* VRAM is the random accessing memory module.  */

#include "type.h"

#include "core/machine/pit.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"

/* Allocates memory for virtual machine ram */
static type_unsigned_32 core_machine_memory_wrap_a20(const t_ram *ram,
    type_unsigned_32 offset)
{
    return offset & (ram->data.flagA20 ? TYPE_MAX_UNSIGNED_32 : ~VRAM_BIT_A20);
}

static type_status core_machine_memory_offset(const t_ram *ram,
    type_unsigned_32 physical, STD_SIZE_T size, STD_SIZE_T *out_offset)
{
    STD_SIZE_T offset;
    type_native_unsigned index;

    if (ram == STD_NULL || out_offset == STD_NULL || ram->connect.backing == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    offset = (STD_SIZE_T)core_machine_memory_wrap_a20(ram, physical);
    for (index = 0u; index < ram->connect.mapping_count; ++index) {
        const core_machine_memory_mapping *mapping = &ram->connect.mappings[index];
        if (physical >= mapping->physical_start &&
            (uint64_t)physical - mapping->physical_start + size <= mapping->bytes) {
            offset = (STD_SIZE_T)mapping->backing_start +
                (STD_SIZE_T)((uint64_t)physical - mapping->physical_start);
            break;
        }
    }
    if (offset > ram->connect.installed_bytes ||
        size > ram->connect.installed_bytes - offset) {
        return TYPE_STATUS_FAULT;
    }
    *out_offset = offset;
    return TYPE_STATUS_OK;
}

/* Allocates one core-owned RAM backing. Callers retain the t_ram, never backing. */
type_status core_machine_memory_allocate_for(t_ram *ram, STD_SIZE_T bytes) {
    C_VOID *backing;

    if (ram == STD_NULL || bytes == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    backing = STD_CALLOC(1u, bytes);
    if (backing == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    STD_FREE((C_VOID *)ram->connect.backing);
    ram->connect.backing = (type_virtual_address)backing;
    ram->connect.installed_bytes = bytes;
    ram->connect.backing_capacity = bytes;
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_mapping(t_ram *ram,
    type_unsigned_32 physical_start,
    type_unsigned_32 backing_start, STD_SIZE_T bytes)
{
    core_machine_memory_mapping *mapping;

    if (ram == STD_NULL || ram->connect.mappings_frozen || bytes == 0u ||
        backing_start > ram->connect.installed_bytes ||
        bytes > ram->connect.installed_bytes - backing_start) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.mapping_count >= CORE_MACHINE_MEMORY_MAPPING_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }
    mapping = &ram->connect.mappings[ram->connect.mapping_count++];
    mapping->physical_start = physical_start;
    mapping->backing_start = backing_start;
    mapping->bytes = bytes;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_memory_freeze_mappings(t_ram *ram)
{
    if (ram != STD_NULL) ram->connect.mappings_frozen = TYPE_TRUE;
}
static C_VOID core_machine_memory_read_a20(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_ram *ram = (t_ram *)owner;

    (C_VOID)port_id;
    if (ram == STD_NULL) return;
    port->data.ioByte = ram->data.flagA20 ? VRAM_FLAG_A20 : TYPE_ZERO_8;
}
static C_VOID core_machine_memory_write_a20(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_ram *ram = (t_ram *)owner;

    (C_VOID)port_id;
    if (ram == STD_NULL) return;
    ram->data.flagA20 = TYPE_GET_BIT(port->data.ioByte, VRAM_FLAG_A20);
}

type_status core_machine_memory_read_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned byte)
{
    STD_SIZE_T offset;
    type_status status = core_machine_memory_offset(ram, physical, byte, &offset);

    if (status != TYPE_STATUS_OK || destination == 0u) return status;
    STD_MEMCPY((C_VOID *)destination,
        (C_VOID *)(ram->connect.backing + offset), byte);
    return TYPE_STATUS_OK;
}
type_status core_machine_memory_write_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned byte)
{
    STD_SIZE_T offset;
    type_status status = core_machine_memory_offset(ram, physical, byte, &offset);

    if (status != TYPE_STATUS_OK || source == 0u) return status;
    STD_MEMCPY((C_VOID *)(ram->connect.backing + offset),
        (C_VOID *)source, byte);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_memory_initialize(t_ram *ram)
{
    if (ram == STD_NULL) return;
    STD_MEMSET((C_VOID *)ram, TYPE_ZERO_8, sizeof(*ram));
    (C_VOID)core_machine_memory_allocate_for(ram, 1u << 24);
}

C_VOID core_machine_memory_reset(t_ram *ram)
{
    if (ram == STD_NULL || ram->connect.backing == 0u) return;
    STD_MEMSET((C_VOID *)&ram->data, TYPE_ZERO_8, sizeof(ram->data));
    STD_MEMSET((C_VOID *)ram->connect.backing, TYPE_ZERO_8,
        ram->connect.backing_capacity);
}

C_VOID core_machine_memory_finalize(t_ram *ram)
{
    if (ram == STD_NULL) return;
    if (ram->connect.backing != 0u) {
        STD_FREE((C_VOID *)ram->connect.backing);
    }
    ram->connect.backing = 0u;
    ram->connect.installed_bytes = 0u;
    ram->connect.backing_capacity = 0u;
}

C_VOID core_machine_memory_register_ports(t_ram *ram, t_port *port)
{
    core_machine_port_add_read(port, 0x0092,
        core_machine_memory_read_a20, ram);
    core_machine_port_add_write(port, 0x0092,
        core_machine_memory_write_a20, ram);
}

type_status core_machine_memory_read_real_from(t_ram *ram, uint16_t segment,
    uint16_t offset, C_VOID *out_data, STD_SIZE_T size)
{
    type_unsigned_32 physical;

    if (ram == STD_NULL || out_data == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (TYPE_MASK_UNSIGNED_16(segment) << 4) + TYPE_MASK_UNSIGNED_16(offset));
    return core_machine_memory_read_physical(ram, physical,
        (type_virtual_address)out_data, size);
}

type_status core_machine_memory_write_real_to(t_ram *ram, uint16_t segment,
    uint16_t offset, const C_VOID *in_data, STD_SIZE_T size)
{
    type_unsigned_32 physical;

    if (ram == STD_NULL || in_data == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (TYPE_MASK_UNSIGNED_16(segment) << 4) + TYPE_MASK_UNSIGNED_16(offset));
    return core_machine_memory_write_physical(ram, physical,
        (type_virtual_address)in_data, size);
}
