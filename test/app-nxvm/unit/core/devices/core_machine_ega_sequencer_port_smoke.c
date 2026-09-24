#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/vadp.h"

static lib_i32 core_machine_ega_write(t_ram *memory, lib_u32 physical,
    lib_u8 value)
{
    return core_machine_memory_write_physical(memory, physical,
        (lib_uptr)&value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 core_machine_ega_read(t_ram *memory, lib_u32 physical,
    lib_u8 *value)
{
    return core_machine_memory_read_physical(memory, physical,
        (lib_uptr)value, sizeof(*value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    const core_machine_vadp_ega_sequencer_config config = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_FALSE
    };
    t_port port;
    t_ram memory;
    t_vadp vadp;
    lib_u8 value = 0u;
    lib_u64 dirty_generation;
    lib_i32 failed = 0;

    lib_memory_set(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    failed |= core_machine_memory_initialize_for(&memory, 16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    failed |= core_machine_vadp_configure_ega_sequencer(&vadp, &memory,
        &config) != LIB_STATUS_OK;
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
    printf("M5:T235:S1:EGA-SEQUENCER:PORT:OK\n");
    printf("M5:T480:S3:EGA-VGA-COMMON:OK\n");
    return 0;
}
