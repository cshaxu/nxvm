/* Copyright 2012-2014 Neko. */

/* VRAM is the random accessing memory module.  */

#include "type.h"

#include "core/machine/pit.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"

/* Allocates memory for virtual machine ram */
static ntvdm64_type_unsigned_32 core_machine_memory_wrap_a20(const t_ram *ram,
    ntvdm64_type_unsigned_32 offset)
{
    return offset & (ram->data.flagA20 ? NTVDM64_TYPE_MAX_UNSIGNED_32 : ~VRAM_BIT_A20);
}

static ntvdm64_type_virtual_address core_machine_memory_address(t_ram *ram,
    ntvdm64_type_unsigned_32 physical)
{
    return ram->connect.pBase + (ntvdm64_type_virtual_address)core_machine_memory_wrap_a20(
        ram, physical);
}

/* Allocates memory for one composition-owned RAM object. */
C_VOID core_machine_memory_allocate_for(t_ram *ram, STD_SIZE_T newsize) {
    if (ram == STD_NULL) return;
    if (newsize) {
        ram->connect.size = newsize;
        if (ram->connect.pBase) {
            STD_FREE((C_VOID *) ram->connect.pBase);
        }
        ram->connect.pBase = (ntvdm64_type_virtual_address) STD_MALLOC(ram->connect.size);
        STD_MEMSET((C_VOID *) ram->connect.pBase, NTVDM64_TYPE_ZERO_8, ram->connect.size);
    }
}
static C_VOID core_machine_memory_read_a20(t_port *port, ntvdm64_type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_ram *ram = (t_ram *)owner;

    (C_VOID)port_id;
    if (ram == STD_NULL) return;
    port->data.ioByte = ram->data.flagA20 ? VRAM_FLAG_A20 : NTVDM64_TYPE_ZERO_8;
}
static C_VOID core_machine_memory_write_a20(t_port *port, ntvdm64_type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_ram *ram = (t_ram *)owner;

    (C_VOID)port_id;
    if (ram == STD_NULL) return;
    ram->data.flagA20 = NTVDM64_TYPE_GET_BIT(port->data.ioByte, VRAM_FLAG_A20);
}

C_VOID core_machine_memory_read_physical(t_ram *ram, ntvdm64_type_unsigned_32 physical,
    ntvdm64_type_virtual_address destination, ntvdm64_type_native_unsigned byte)
{
    if (ram == STD_NULL) return;
    if (physical >= ram->connect.size && physical >= 0xfffe0000) {
        physical &= 0x001fffff;
    }
    STD_MEMCPY((C_VOID *) destination,
        (C_VOID *) core_machine_memory_address(ram, physical), byte);
}
C_VOID core_machine_memory_write_physical(t_ram *ram, ntvdm64_type_unsigned_32 physical,
    ntvdm64_type_virtual_address source, ntvdm64_type_native_unsigned byte)
{
    if (ram == STD_NULL) return;
    STD_MEMCPY((C_VOID *) core_machine_memory_address(ram, physical),
        (C_VOID *) source, byte);
}

C_VOID core_machine_memory_initialize(t_ram *ram)
{
    if (ram == STD_NULL) return;
    STD_MEMSET((C_VOID *)ram, NTVDM64_TYPE_ZERO_8, sizeof(*ram));
    core_machine_memory_allocate_for(ram, 1u << 24);
}

C_VOID core_machine_memory_reset(t_ram *ram)
{
    if (ram == STD_NULL || ram->connect.pBase == 0u) return;
    STD_MEMSET((C_VOID *)&ram->data, NTVDM64_TYPE_ZERO_8, sizeof(ram->data));
    STD_MEMSET((C_VOID *)ram->connect.pBase, NTVDM64_TYPE_ZERO_8, ram->connect.size);
}

C_VOID core_machine_memory_finalize(t_ram *ram)
{
    if (ram == STD_NULL) return;
    if (ram->connect.pBase != 0u) {
        STD_FREE((C_VOID *)ram->connect.pBase);
    }
    ram->connect.pBase = 0u;
    ram->connect.size = 0u;
}

C_VOID core_machine_memory_register_ports(t_ram *ram, t_port *port)
{
    core_machine_port_add_read(port, 0x0092,
        core_machine_memory_read_a20, ram);
    core_machine_port_add_write(port, 0x0092,
        core_machine_memory_write_a20, ram);
}

C_VOID core_machine_memory_read_real_from(t_ram *ram, uint16_t segment,
    uint16_t offset, C_VOID *out_data, STD_SIZE_T size)
{
    ntvdm64_type_unsigned_32 physical;

    if (ram == STD_NULL || ram->connect.size == 0u) return;
    physical = core_machine_memory_wrap_a20(ram,
        (NTVDM64_TYPE_MASK_UNSIGNED_16(segment) << 4) + NTVDM64_TYPE_MASK_UNSIGNED_16(offset));
    physical %= ram->connect.size;
    STD_MEMCPY(out_data, (C_VOID *)(ram->connect.pBase + physical), size);
}

C_VOID core_machine_memory_write_real_to(t_ram *ram, uint16_t segment,
    uint16_t offset, const C_VOID *in_data, STD_SIZE_T size)
{
    ntvdm64_type_unsigned_32 physical;

    if (ram == STD_NULL || ram->connect.size == 0u) return;
    physical = core_machine_memory_wrap_a20(ram,
        (NTVDM64_TYPE_MASK_UNSIGNED_16(segment) << 4) + NTVDM64_TYPE_MASK_UNSIGNED_16(offset));
    physical %= ram->connect.size;
    STD_MEMCPY((C_VOID *)(ram->connect.pBase + physical), (C_VOID *)in_data, size);
}

C_VOID *core_machine_memory_real_address(t_ram *ram, uint16_t segment,
    uint16_t offset)
{
    ntvdm64_type_unsigned_32 physical;

    if (ram == STD_NULL || ram->connect.pBase == 0u || ram->connect.size == 0u) {
        return STD_NULL;
    }
    physical = core_machine_memory_wrap_a20(ram,
        ((ntvdm64_type_unsigned_32)segment << 4) + offset);
    return (C_VOID *)(ram->connect.pBase + (physical % ram->connect.size));
}
