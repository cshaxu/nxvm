#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "video_fixture.h"
#include "core/board-base/vadp.h"
#include "core/board-base/machine_board_interface.h"

static lib_i32 core_machine_ega_controller_write(core_machine *machine, lib_u32 physical,
    lib_u8 value)
{
    return core_machine_memory_write(machine, physical,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 core_machine_ega_controller_read(core_machine *machine, lib_u32 physical,
    lib_u8 *value)
{
    return core_machine_memory_read(machine, physical,
        value, sizeof(*value)) == LIB_STATUS_OK;
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
    core_machine *machine;
    t_vadp vadp;
    lib_u8 value = 0u;
    core_machine_display_config config = {
        .text_timing = {48u, 8u, 8u}, .ega_present = LIB_TRUE
    };
    lib_i32 failed = 0;
    machine = test_video_create(&vadp);
    config.ega_sequencer = sequencer;
    config.ega_controllers = controllers;
    failed |= core_machine_vadp_configure(&vadp, &config) != LIB_STATUS_OK;
    /* The complete generic board decodes the status port through Misc Output;
     * this fixture uses the color alias to reset the attribute flip-flop. */
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, 1u);

    failed |= test_video_port_read(machine, 0x03ceu) != 0u;
    failed |= test_video_port_read(machine, 0x03cfu) != 0u;
    test_video_port_write(machine, 0x03ceu, 6u);
    failed |= test_video_port_read(machine, 0x03cfu) != 0x05u;
    test_video_port_write(machine, 0x03ceu, 0u);
    failed |= test_video_port_read(machine, 0x03cfu) != 0u;
    test_video_port_write(machine, 0x03ceu, 6u);
    test_video_port_write(machine, 0x03cfu, 0xffu);
    failed |= test_video_port_read(machine, 0x03cfu) != 0x0fu;
    test_video_port_write(machine, 0x03ceu, 31u);
    test_video_port_write(machine, 0x03cfu, 0xa5u);
    failed |= test_video_port_read(machine, 0x03cfu) != 0u;
    test_video_port_write(machine, 0x03ceu, 6u);
    failed |= test_video_port_read(machine, 0x03cfu) != 0x0fu;

    test_video_port_write(machine, 0x03ceu, 6u);
    test_video_port_write(machine, 0x03cfu, 0x00u);
    failed |= !x86_video_ega_aperture_contains(vadp.chip, 0x000a0000u,
        0x00020000u) || x86_video_ega_aperture_contains(vadp.chip,
        0x000c0000u, 1u);
    test_video_port_write(machine, 0x03cfu, 0x05u);
    failed |= !x86_video_ega_aperture_contains(vadp.chip, 0x000a0000u,
        0x00010000u) || x86_video_ega_aperture_contains(vadp.chip,
        0x000b0000u, 1u);
    test_video_port_write(machine, 0x03cfu, 0x09u);
    failed |= !x86_video_ega_aperture_contains(vadp.chip, 0x000b0000u,
        0x00008000u) || x86_video_ega_aperture_contains(vadp.chip,
        0x000a0000u, 1u);
    failed |= !core_machine_ega_controller_write(machine, 0x000b0000u, 0x5au);
    failed |= !core_machine_ega_controller_write(machine, 0x000a0000u, 0xa5u);
    test_video_port_write(machine, 0x03cfu, 0x0du);
    failed |= !x86_video_ega_aperture_contains(vadp.chip, 0x000b8000u,
        0x00008000u) || x86_video_ega_aperture_contains(vadp.chip,
        0x000b0000u, 1u);

    test_video_port_write(machine, 0x03c0u, 0x31u);
    test_video_port_write(machine, 0x03c0u, 0xffu);
    failed |= test_video_port_read(machine, 0x03c1u) != 0x3fu;
    (void)test_video_port_read(machine, 0x03dau);
    test_video_port_write(machine, 0x03c0u, 0x00u);
    failed |= test_video_port_read(machine, 0x03c1u) != 0x3fu;
    (void)test_video_port_read(machine, 0x03dau);
    test_video_port_write(machine, 0x03c0u, 0x12u);
    test_video_port_write(machine, 0x03c0u, 0xf5u);
    failed |= test_video_port_read(machine, 0x03c1u) != 0x05u;
    (void)test_video_port_read(machine, 0x03dau);
    test_video_port_write(machine, 0x03c0u, 0x1fu);
    test_video_port_write(machine, 0x03c0u, 0xffu);
    failed |= test_video_port_read(machine, 0x03c1u) != 0u;
    (void)test_video_port_read(machine, 0x03dau);
    test_video_port_write(machine, 0x03c0u, 0x12u);
    failed |= test_video_port_read(machine, 0x03c1u) != 0x05u;

    (void)test_video_port_read(machine, 0x03dau);
    test_video_port_write(machine, 0x03c0u, 0x10u);
    test_video_port_write(machine, 0x03c0u, 0xffu);
    failed |= test_video_port_read(machine, 0x03c1u) != 0x0fu;
    (void)test_video_port_read(machine, 0x03dau);
    test_video_port_write(machine, 0x03c0u, 0x13u);
    test_video_port_write(machine, 0x03c0u, 0xffu);
    failed |= test_video_port_read(machine, 0x03c1u) != 0x0fu;
    (void)test_video_port_read(machine, 0x03dau);
    test_video_port_write(machine, 0x03c0u, 0x14u);
    test_video_port_write(machine, 0x03c0u, 0xffu);
    failed |= test_video_port_read(machine, 0x03c1u) != 0u;

    test_video_port_write(machine, 0x03c4u, 2u);
    test_video_port_write(machine, 0x03c5u, 0x05u);
    test_video_port_write(machine, 0x03ceu, 5u);
    test_video_port_write(machine, 0x03cfu, 0x03u);
    failed |= !core_machine_ega_controller_write(machine, 0x000b8000u, 0xa6u);
    failed |= !core_machine_ega_controller_read(machine, 0x000b8000u, &value) ||
        value != 0xa6u;

    x86_video_reset(vadp.chip);
    failed |= test_video_port_read(machine, 0x03c4u) != 0u ||
        test_video_port_read(machine, 0x03c5u) != 0x03u;
    failed |= test_video_port_read(machine, 0x03ceu) != 0u ||
        test_video_port_read(machine, 0x03cfu) != 0u;

    if (failed) {
        lib_c_fprintf(lib_c_stderr, "EGA-CONTROLLER:FAIL\n");
        core_machine_vadp_finalize(&vadp);
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_vadp_finalize(&vadp);
    core_machine_destroy(machine);
    lib_c_printf("EGA-CONTROLLER:PORT:OK\n");
    lib_c_printf("COMMON-OWNER:OK\n");
    return 0;
}
