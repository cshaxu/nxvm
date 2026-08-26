#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT core_machine_ega_write(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 value)
{
    return core_machine_memory_write_physical(memory, physical,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
}

static C_INT core_machine_ega_read(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 *value)
{
    return core_machine_memory_read_physical(memory, physical,
        (type_virtual_address)value, sizeof(*value)) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    const core_machine_vadp_ega_sequencer_config config = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_FALSE
    };
    t_port port;
    t_ram memory;
    t_vadp vadp;
    type_unsigned_8 value = 0u;
    type_unsigned_64 dirty_generation;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    failed |= core_machine_vadp_configure_ega_sequencer(&vadp, &memory,
        &config) != TYPE_STATUS_OK;
    failed |= core_machine_port_read(&port, 0x03c4u) != 0u;
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x03u;

    core_machine_port_write(&port, 0x03c4u, 1u);
    core_machine_port_write(&port, 0x03c5u, 0xffu);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x3du;
    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0xa5u);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x05u;
    core_machine_port_write(&port, 0x03c4u, 3u);
    core_machine_port_write(&port, 0x03c5u, 0xffu);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x3fu;
    core_machine_port_write(&port, 0x03c4u, 2u);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x05u;

    dirty_generation = vadp.data.dirty_generation;
    failed |= !core_machine_ega_write(&memory, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        0x5au);
    failed |= !core_machine_ega_read(&memory, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        &value) || value != 0x5au;
    failed |= vadp.data.dirty_generation != dirty_generation + 1u;
    failed |= !core_machine_vadp_ega_aperture_contains(&vadp,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, 1u);
    failed |= core_machine_vadp_ega_aperture_contains(&vadp,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE + CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        1u);
    dirty_generation = vadp.data.dirty_generation;
    failed |= !core_machine_ega_write(&memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE + CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0xa5u);
    failed |= vadp.data.dirty_generation != dirty_generation;
    failed |= !core_machine_ega_read(&memory, CORE_MACHINE_VADP_VIDEO_BASE, &value) ||
        value != 0u;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T235:S1:EGA-SEQUENCER:PORT:OK\n");
    STD_PRINTF("M5:T480:S3:EGA-VGA-COMMON:OK\n");
    return 0;
}
