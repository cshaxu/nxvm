#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "video_fixture.h"
#include "x86-nxvm/board-base/vadp.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "board_board_fixture.h"

static lib_i32 core_machine_ega_write(x86_core_machine *machine, lib_u32 physical,
    lib_u8 value)
{
    return x86_core_machine_memory_write(machine, physical,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 core_machine_ega_read(x86_core_machine *machine, lib_u32 physical,
    lib_u8 *value)
{
    return x86_core_machine_memory_read(machine, physical,
        value, sizeof(*value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    const x86_nxvm_board_display_config config = {
        .text_timing = {48u, 8u, 8u},
        /* Owned CGA backing makes the public frame generation observable;
         * A0000 remains ordinary RAM with the EGA write observer. */
        .cga_vram_present = LIB_TRUE,
        .ega_present = LIB_TRUE,
        .ega_sequencer = {
            X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, X86_NXVM_BOARD_VADP_EGA_APERTURE_BYTES,
            0x03u, 0x00u, 0x0fu, 0x02u, LIB_FALSE
        },
        /* Whole-board configuration includes the controller; select the same
         * 64 KiB A0000 window as the original sequencer-only fixture. */
        .ega_controllers = { .graphics = { [6] = 0x05u } }
    };
    x86_core_machine *machine;
    t_vadp vadp;
    lib_u8 value = 0u;
    lib_u64 dirty_generation;
    x86_video_snapshot_observation observation;
    lib_i32 failed = 0;
    machine = test_video_create(&vadp);
    failed |= x86_nxvm_board_vadp_configure(&vadp, &config) != LIB_STATUS_OK;
    failed |= test_x86_nxvm_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != LIB_STATUS_OK;
    {
        static const lib_u16 read_ports[] = { 0x03c1u, 0x03c4u, 0x03c5u, 0x03ceu, 0x03cfu };

        for (lib_size index = 0u; index < sizeof(read_ports) / sizeof(read_ports[0]); ++index)
            failed |= test_video_port_is_owned(machine, read_ports[index], LIB_FALSE);
    }
    failed |= !test_video_port_read_is_unsupported(machine, 0x03c4u);
    failed |= !test_video_port_read_is_unsupported(machine, 0x03c5u);

    test_video_port_write(machine, 0x03c4u, 1u);
    test_video_port_write(machine, 0x03c5u, 0xffu);
    failed |= !test_video_port_read_is_unsupported(machine, 0x03c5u);
    test_video_port_write(machine, 0x03c4u, 2u);
    test_video_port_write(machine, 0x03c5u, 0xa5u);
    failed |= !test_video_port_read_is_unsupported(machine, 0x03c5u);
    test_video_port_write(machine, 0x03c4u, 3u);
    test_video_port_write(machine, 0x03c5u, 0xffu);
    failed |= !test_video_port_read_is_unsupported(machine, 0x03c5u);
    test_video_port_write(machine, 0x03c4u, 2u);
    failed |= !test_video_port_read_is_unsupported(machine, 0x03c5u);

    {
        static const lib_u8 masks[] = { 0x03u, 0x0fu, 0x0fu, 0x0fu, 0x07u };

        /* Observe normalized writes through the existing public generation:
         * replaying each canonical value must not change stored state. */
        for (lib_u8 index = 0u; index < sizeof(masks); ++index) {
            test_video_port_write(machine, 0x03c4u, index);
            test_video_port_write(machine, 0x03c5u, 0xffu);
            x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
            dirty_generation = observation.generation;
            test_video_port_write(machine, 0x03c5u, masks[index]);
            x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
            failed |= observation.generation != dirty_generation ||
                !test_video_port_read_is_unsupported(machine, 0x03c5u);
        }
    }

    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    dirty_generation = observation.generation;
    failed |= !core_machine_ega_write(machine, X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE,
        0x5au);
    failed |= !core_machine_ega_read(machine, X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE,
        &value) || value != 0x5au;
    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    failed |= observation.generation != dirty_generation + 1u;
    failed |= !x86_video_ega_aperture_contains(vadp.chip,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, 1u);
    failed |= x86_video_ega_aperture_contains(vadp.chip,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE + X86_NXVM_BOARD_VADP_EGA_APERTURE_BYTES,
        1u);
    dirty_generation = observation.generation;
    failed |= !core_machine_ega_write(machine,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE + X86_NXVM_BOARD_VADP_EGA_APERTURE_BYTES,
        0xa5u);
    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    failed |= observation.generation != dirty_generation;
    failed |= !core_machine_ega_read(machine, X86_NXVM_BOARD_VADP_VIDEO_BASE, &value) ||
        value != 0u;

    {
        /* MOV DX,3C5h; IN AL,DX; MOV [0500h],AL; HLT. The actual CPU
         * uses Core's unregistered bus model, not the paused inspection API. */
        static const lib_u8 program[] = { 0xbau, 0xc5u, 0x03u, 0xecu,
            0xa2u, 0x00u, 0x05u, 0xf4u };
        x86_core_machine_run_result result;
        x86_core_cpu_state cpu;

        value = 0xa5u;
        failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, program,
                sizeof(program)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x0500u, &value, 1u) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){ 4u, 0u },
                &result) != LIB_STATUS_OK || result.executed != 4u ||
            result.reason == X86_CORE_MACHINE_STOP_FAULT ||
            x86_core_machine_get_cpu_state(machine, &cpu) != LIB_STATUS_OK || !cpu.halted ||
            x86_core_machine_memory_read(machine, 0x0500u, &value, 1u) != LIB_STATUS_OK ||
            value != 0u;
    }
    {
        /* A mixed CRTC port is still registered for readable indices. Reading
         * a write-only index must not cancel the actual processor I/O cycle. */
        static const lib_u8 program[] = {0xbau, 0xd5u, 0x03u, 0xecu,
            0xa2u, 0x00u, 0x05u, 0xf4u};
        x86_core_machine_run_result result;

        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
        test_video_port_write(machine, 0x03c2u, 3u);
        test_video_port_write(machine, 0x03d4u, 0x13u);
        test_video_port_write(machine, 0x03d5u, 0x77u);
        failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, program,
                sizeof(program)) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){4u, 0u}, &result) != LIB_STATUS_OK ||
            result.executed != 4u || result.reason == X86_CORE_MACHINE_STOP_FAULT ||
            x86_core_machine_memory_read(machine, 0x0500u, &value, 1u) != LIB_STATUS_OK || value != 0x77u;
    }
    x86_nxvm_board_vadp_finalize(&vadp);
    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("EGA-SEQUENCER:PORT:OK\n");
    lib_c_printf("EGA-VGA-COMMON:OK\n");
    return 0;
}
