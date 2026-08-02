/* Copyright 2012-2014 Neko. */

/* VRAM is the random accessing memory module.  */

#include "type.h"

#include "core/machine/pit.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"

static t_ram *coreMachineMemory;

t_ram *core_machine_memory_current(void)
{
    return coreMachineMemory;
}

void core_machine_memory_bind_live(t_ram *ram)
{
    coreMachineMemory = ram;
}

void core_machine_memory_unbind_live(void)
{
    coreMachineMemory = NULL;
}

/* Allocates memory for virtual machine ram */
static t_nubit32 core_machine_memory_wrap_a20(const t_ram *ram,
    t_nubit32 offset)
{
    return offset & (ram->data.flagA20 ? Max32 : ~VRAM_BIT_A20);
}

static t_vaddrcc core_machine_memory_address(t_ram *ram,
    t_nubit32 physical)
{
    return ram->connect.pBase + (t_vaddrcc)core_machine_memory_wrap_a20(
        ram, physical);
}

/* Allocates memory for one composition-owned RAM object. */
void core_machine_memory_allocate_for(t_ram *ram, size_t newsize) {
    if (ram == NULL) return;
    if (newsize) {
        ram->connect.size = newsize;
        if (ram->connect.pBase) {
            FREE((void *) ram->connect.pBase);
        }
        ram->connect.pBase = (t_vaddrcc) MALLOC(ram->connect.size);
        MEMSET((void *) ram->connect.pBase, Zero8, ram->connect.size);
    }
}
static void core_machine_memory_read_a20(t_port *port, t_nubit16 port_id,
    void *owner)
{
    t_ram *ram = (t_ram *)owner;

    (void)port_id;
    if (ram == NULL) return;
    port->data.ioByte = ram->data.flagA20 ? VRAM_FLAG_A20 : Zero8;
}
static void core_machine_memory_write_a20(t_port *port, t_nubit16 port_id,
    void *owner)
{
    t_ram *ram = (t_ram *)owner;

    (void)port_id;
    if (ram == NULL) return;
    ram->data.flagA20 = GetBit(port->data.ioByte, VRAM_FLAG_A20);
}

void core_machine_memory_read_physical(t_ram *ram, t_nubit32 physical,
    t_vaddrcc destination, t_nubitcc byte)
{
    if (ram == NULL) return;
    if (physical >= ram->connect.size && physical >= 0xfffe0000) {
        physical &= 0x001fffff;
    }
    MEMCPY((void *) destination,
        (void *) core_machine_memory_address(ram, physical), byte);
}
void core_machine_memory_write_physical(t_ram *ram, t_nubit32 physical,
    t_vaddrcc source, t_nubitcc byte)
{
    if (ram == NULL) return;
    MEMCPY((void *) core_machine_memory_address(ram, physical),
        (void *) source, byte);
}

void core_machine_memory_initialize(t_ram *ram)
{
    if (ram == NULL) return;
    MEMSET((void *)ram, Zero8, sizeof(*ram));
    core_machine_memory_allocate_for(ram, 1u << 24);
}

void core_machine_memory_reset(t_ram *ram)
{
    if (ram == NULL || ram->connect.pBase == 0u) return;
    MEMSET((void *)&ram->data, Zero8, sizeof(ram->data));
    MEMSET((void *)ram->connect.pBase, Zero8, ram->connect.size);
}

void core_machine_memory_finalize(t_ram *ram)
{
    if (ram == NULL) return;
    if (ram->connect.pBase != 0u) {
        FREE((void *)ram->connect.pBase);
    }
    ram->connect.pBase = 0u;
    ram->connect.size = 0u;
}

void core_machine_memory_register_ports(t_ram *ram, t_port *port)
{
    core_machine_port_add_read(port, 0x0092,
        core_machine_memory_read_a20, ram);
    core_machine_port_add_write(port, 0x0092,
        core_machine_memory_write_a20, ram);
}

void core_machine_memory_allocate(size_t bytes)
{
    core_machine_memory_allocate_for(core_machine_memory_current(), bytes);
}

void core_machine_memory_read_real(uint16_t segment, uint16_t offset,
    void *out_data, size_t size)
{
    core_machine_memory_read_real_from(core_machine_memory_current(), segment,
        offset, out_data, size);
}

void core_machine_memory_write_real(uint16_t segment, uint16_t offset,
    const void *in_data, size_t size)
{
    core_machine_memory_write_real_to(core_machine_memory_current(), segment,
        offset, in_data, size);
}

void core_machine_memory_read_real_from(t_ram *ram, uint16_t segment,
    uint16_t offset, void *out_data, size_t size)
{
    t_nubit32 physical;

    if (ram == NULL || ram->connect.size == 0u) return;
    physical = core_machine_memory_wrap_a20(ram,
        (GetMax16(segment) << 4) + GetMax16(offset));
    physical %= ram->connect.size;
    MEMCPY(out_data, (void *)(ram->connect.pBase + physical), size);
}

void core_machine_memory_write_real_to(t_ram *ram, uint16_t segment,
    uint16_t offset, const void *in_data, size_t size)
{
    t_nubit32 physical;

    if (ram == NULL || ram->connect.size == 0u) return;
    physical = core_machine_memory_wrap_a20(ram,
        (GetMax16(segment) << 4) + GetMax16(offset));
    physical %= ram->connect.size;
    MEMCPY((void *)(ram->connect.pBase + physical), (void *)in_data, size);
}

void *core_machine_memory_real_address(t_ram *ram, uint16_t segment,
    uint16_t offset)
{
    t_nubit32 physical;

    if (ram == NULL || ram->connect.pBase == 0u || ram->connect.size == 0u) {
        return NULL;
    }
    physical = core_machine_memory_wrap_a20(ram,
        ((t_nubit32)segment << 4) + offset);
    return (void *)(ram->connect.pBase + (physical % ram->connect.size));
}
