#include "lib/types/types_interface.h"
#include <stdio.h>

#include "video_fixture.h"
#include "x86/ibmpc-common/vadp.h"
#include "x86/ibmpc-common/machine_board_interface.h"

static lib_i32 core_machine_ega_write(core_machine *machine, lib_u32 physical,
    lib_u8 value)
{
    return core_machine_memory_write(machine, physical,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 core_machine_ega_read(core_machine *machine, lib_u32 physical,
    lib_u8 *value)
{
    return core_machine_memory_read(machine, physical,
        value, sizeof(*value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    const core_machine_display_config config = {
        .text_timing = {48u, 8u, 8u},
        /* Owned CGA backing makes the public frame generation observable;
         * A0000 remains ordinary RAM with the EGA write observer. */
        .cga_vram_present = LIB_TRUE,
        .ega_present = LIB_TRUE,
        .ega_sequencer = {
            CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
            0x03u, 0x00u, 0x0fu, 0x02u, LIB_FALSE
        },
        /* Whole-board configuration includes the controller; select the same
         * 64 KiB A0000 window as the original sequencer-only fixture. */
        .ega_controllers = { .graphics = { [6] = 0x05u } }
    };
    core_machine *machine;
    t_vadp vadp;
    lib_u8 value = 0u;
    lib_u64 dirty_generation;
    x86_video_snapshot_observation observation;
    lib_i32 failed = 0;
    machine = test_video_create(&vadp);
    failed |= core_machine_vadp_configure(&vadp, &config) != LIB_STATUS_OK;
    failed |= test_video_port_read(machine, 0x03c4u) != 0u;
    failed |= test_video_port_read(machine, 0x03c5u) != 0x03u;

    test_video_port_write(machine, 0x03c4u, 1u);
    test_video_port_write(machine, 0x03c5u, 0xffu);
    failed |= test_video_port_read(machine, 0x03c5u) != 0x3du;
    test_video_port_write(machine, 0x03c4u, 2u);
    test_video_port_write(machine, 0x03c5u, 0xa5u);
    failed |= test_video_port_read(machine, 0x03c5u) != 0x05u;
    test_video_port_write(machine, 0x03c4u, 3u);
    test_video_port_write(machine, 0x03c5u, 0xffu);
    failed |= test_video_port_read(machine, 0x03c5u) != 0x3fu;
    test_video_port_write(machine, 0x03c4u, 2u);
    failed |= test_video_port_read(machine, 0x03c5u) != 0x05u;

    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    dirty_generation = observation.generation;
    failed |= !core_machine_ega_write(machine, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        0x5au);
    failed |= !core_machine_ega_read(machine, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        &value) || value != 0x5au;
    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    failed |= observation.generation != dirty_generation + 1u;
    failed |= !x86_video_ega_aperture_contains(vadp.chip,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, 1u);
    failed |= x86_video_ega_aperture_contains(vadp.chip,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE + CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        1u);
    dirty_generation = observation.generation;
    failed |= !core_machine_ega_write(machine,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE + CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0xa5u);
    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    failed |= observation.generation != dirty_generation;
    failed |= !core_machine_ega_read(machine, CORE_MACHINE_VADP_VIDEO_BASE, &value) ||
        value != 0u;

    core_machine_vadp_finalize(&vadp);
    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T235:S1:EGA-SEQUENCER:PORT:OK\n");
    printf("M5:T480:S3:EGA-VGA-COMMON:OK\n");
    return 0;
}
