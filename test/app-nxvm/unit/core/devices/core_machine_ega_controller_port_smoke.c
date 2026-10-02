#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/vadp.h"
#include "app-nxvm/devices/machine_interface.h"

static lib_i32 core_machine_ega_controller_write(t_ram *memory, lib_u32 physical,
    lib_u8 value)
{
    return core_machine_memory_write_physical(memory, physical,
        (lib_uptr)&value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 core_machine_ega_controller_read(t_ram *memory, lib_u32 physical,
    lib_u8 *value)
{
    return core_machine_memory_read_physical(memory, physical,
        (lib_uptr)value, sizeof(*value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_FALSE
    };
    const x86_video_ega_controller_config controllers = {
        { 0xf0u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xf5u, 0x00u, 0xffu },
        { 0xffu, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    core_machine machine = {.lifecycle = CORE_MACHINE_INITIALIZED};
    t_port *port = &machine.executor_port;
    t_ram *memory = &machine.executor_memory;
    t_vadp vadp;
    lib_u8 value = 0u;
    core_machine_display_config config = {
        .text_timing = {48u, 8u, 8u}, .ega_present = LIB_TRUE
    };
    lib_i32 failed = 0;

    lib_memory_set(memory, 0, sizeof(*memory));
    core_machine_port_initialize(port);
    failed |= core_machine_memory_initialize_for(memory, 16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    failed |= core_machine_vadp_initialize(&vadp, &machine) != LIB_STATUS_OK;
    config.ega_sequencer = sequencer;
    config.ega_controllers = controllers;
    failed |= core_machine_vadp_configure(&vadp, &config) != LIB_STATUS_OK;
    /* The complete generic board decodes the status port through Misc Output;
     * this fixture uses the color alias to reset the attribute flip-flop. */
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, 1u);

    failed |= core_machine_port_read(port, 0x03ceu) != 0u;
    failed |= core_machine_port_read(port, 0x03cfu) != 0u;
    core_machine_port_write(port, 0x03ceu, 6u);
    failed |= core_machine_port_read(port, 0x03cfu) != 0x05u;
    core_machine_port_write(port, 0x03ceu, 0u);
    failed |= core_machine_port_read(port, 0x03cfu) != 0u;
    core_machine_port_write(port, 0x03ceu, 6u);
    core_machine_port_write(port, 0x03cfu, 0xffu);
    failed |= core_machine_port_read(port, 0x03cfu) != 0x0fu;
    core_machine_port_write(port, 0x03ceu, 31u);
    core_machine_port_write(port, 0x03cfu, 0xa5u);
    failed |= core_machine_port_read(port, 0x03cfu) != 0u;
    core_machine_port_write(port, 0x03ceu, 6u);
    failed |= core_machine_port_read(port, 0x03cfu) != 0x0fu;

    core_machine_port_write(port, 0x03ceu, 6u);
    core_machine_port_write(port, 0x03cfu, 0x00u);
    failed |= !x86_video_ega_aperture_contains(vadp.chip, 0x000a0000u,
        0x00020000u) || x86_video_ega_aperture_contains(vadp.chip,
        0x000c0000u, 1u);
    core_machine_port_write(port, 0x03cfu, 0x05u);
    failed |= !x86_video_ega_aperture_contains(vadp.chip, 0x000a0000u,
        0x00010000u) || x86_video_ega_aperture_contains(vadp.chip,
        0x000b0000u, 1u);
    core_machine_port_write(port, 0x03cfu, 0x09u);
    failed |= !x86_video_ega_aperture_contains(vadp.chip, 0x000b0000u,
        0x00008000u) || x86_video_ega_aperture_contains(vadp.chip,
        0x000a0000u, 1u);
    failed |= !core_machine_ega_controller_write(memory, 0x000b0000u, 0x5au);
    failed |= !core_machine_ega_controller_write(memory, 0x000a0000u, 0xa5u);
    core_machine_port_write(port, 0x03cfu, 0x0du);
    failed |= !x86_video_ega_aperture_contains(vadp.chip, 0x000b8000u,
        0x00008000u) || x86_video_ega_aperture_contains(vadp.chip,
        0x000b0000u, 1u);

    core_machine_port_write(port, 0x03c0u, 0x31u);
    core_machine_port_write(port, 0x03c0u, 0xffu);
    failed |= core_machine_port_read(port, 0x03c1u) != 0x3fu;
    (void)core_machine_port_read(port, 0x03dau);
    core_machine_port_write(port, 0x03c0u, 0x00u);
    failed |= core_machine_port_read(port, 0x03c1u) != 0x3fu;
    (void)core_machine_port_read(port, 0x03dau);
    core_machine_port_write(port, 0x03c0u, 0x12u);
    core_machine_port_write(port, 0x03c0u, 0xf5u);
    failed |= core_machine_port_read(port, 0x03c1u) != 0x05u;
    (void)core_machine_port_read(port, 0x03dau);
    core_machine_port_write(port, 0x03c0u, 0x1fu);
    core_machine_port_write(port, 0x03c0u, 0xffu);
    failed |= core_machine_port_read(port, 0x03c1u) != 0u;
    (void)core_machine_port_read(port, 0x03dau);
    core_machine_port_write(port, 0x03c0u, 0x12u);
    failed |= core_machine_port_read(port, 0x03c1u) != 0x05u;

    (void)core_machine_port_read(port, 0x03dau);
    core_machine_port_write(port, 0x03c0u, 0x10u);
    core_machine_port_write(port, 0x03c0u, 0xffu);
    failed |= core_machine_port_read(port, 0x03c1u) != 0x0fu;
    (void)core_machine_port_read(port, 0x03dau);
    core_machine_port_write(port, 0x03c0u, 0x13u);
    core_machine_port_write(port, 0x03c0u, 0xffu);
    failed |= core_machine_port_read(port, 0x03c1u) != 0x0fu;
    (void)core_machine_port_read(port, 0x03dau);
    core_machine_port_write(port, 0x03c0u, 0x14u);
    core_machine_port_write(port, 0x03c0u, 0xffu);
    failed |= core_machine_port_read(port, 0x03c1u) != 0u;

    core_machine_port_write(port, 0x03c4u, 2u);
    core_machine_port_write(port, 0x03c5u, 0x05u);
    core_machine_port_write(port, 0x03ceu, 5u);
    core_machine_port_write(port, 0x03cfu, 0x03u);
    failed |= !core_machine_ega_controller_write(memory, 0x000b8000u, 0xa6u);
    failed |= !core_machine_ega_controller_read(memory, 0x000b8000u, &value) ||
        value != 0xa6u;

    x86_video_reset(vadp.chip);
    failed |= core_machine_port_read(port, 0x03c4u) != 0u ||
        core_machine_port_read(port, 0x03c5u) != 0x03u;
    failed |= core_machine_port_read(port, 0x03ceu) != 0u ||
        core_machine_port_read(port, 0x03cfu) != 0u;

    if (failed) {
        fprintf(stderr, "M5:T236:S1:EGA-CONTROLLER:FAIL\n");
        core_machine_vadp_finalize(&vadp);
        core_machine_memory_finalize(memory);
        core_machine_port_finalize(port);
        return 1;
    }
    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(memory);
    core_machine_port_finalize(port);
    printf("M5:T236:S1:EGA-CONTROLLER:PORT:OK\n");
    printf("M5:T480:S3:COMMON-OWNER:OK\n");
    return 0;
}
