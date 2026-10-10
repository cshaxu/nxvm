#include "app-mydeskpro386/profiles/observation_interface.h"
#include "../../../app-nxvm/unit/support/profile.h"
#include "core/machine/machine_interface.h"
#include "../../support/model40.h"
#include "../../../app-nxvm/unit/support/ibmpc/board-common/controller_fixture.h"
#include "../../../app-nxvm/unit/support/ibmpc/board-common/composition_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/machine/machine_private.h"
#include "core/machine/lifecycle.h"
#include "core/machine/media/fdd_interface.h"
#include "../../support/rom/model40_session_assets.h"

#define MODEL40_FDC_BYTES (80u * 2u * 15u * 512u)
static lib_i32 floppy_channel_matrix(void)
{
    static const lib_u8 allowed[] = {0x0fu, 0x06u, 0x04u, 0x08u};
    static const lib_u32 rates[] = {500000u, 300000u, 250000u};
    static const lib_u16 positions[] = {0u, 1u, 2u, 39u, 40u, 78u, 79u, 80u};

    for (vm_profile_floppy_kind drive = VM_PROFILE_FLOPPY_35_1440K;
            drive <= VM_PROFILE_FLOPPY_35_720K; ++drive) {
        const core_machine_fdc_channel_provider channel = vm_profile_floppy_channel_get(drive);
        for (vm_profile_floppy_kind media = VM_PROFILE_FLOPPY_35_1440K;
                media <= VM_PROFILE_FLOPPY_35_720K; ++media) {
            const core_machine_media_geometry *geometry = vm_profile_floppy_geometry_get(media);
            const lib_u16 step = drive == VM_PROFILE_FLOPPY_525_1200K &&
                media == VM_PROFILE_FLOPPY_525_360K ? 2u : 1u;
            const lib_u32 expected_rate = step == 2u ? 300000u :
                (media <= VM_PROFILE_FLOPPY_525_1200K ? 500000u : 250000u);
            for (lib_size rate = 0u; rate < sizeof(rates) / sizeof(rates[0]); ++rate) {
                for (lib_size position = 0u; position < sizeof(positions) / sizeof(positions[0]); ++position) {
                    lib_u16 cylinder = 0xffffu;
                    const lib_bool expected = (allowed[drive] & (1u << media)) != 0u &&
                        rates[rate] == expected_rate && positions[position] % step == 0u &&
                        positions[position] / step < geometry->cylinders;
                    const lib_bool actual = channel.sample(channel.context, 0u,
                        geometry, positions[position], rates[rate], LIB_TRUE, &cylinder);
                    if (actual != expected ||
                        (actual && cylinder != positions[position] / step) ||
                        channel.sample(channel.context, 0u, geometry,
                            positions[position], rates[rate], LIB_FALSE, &cylinder)) return 1;
                }
            }
        }
    }
    return 0;
}

static lib_bool model40_fdc_command(core_machine_board_state *board, core_machine *machine,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;
    for (index = 0u; index < count; ++index)
        if (core_machine_bus_write(machine, 0x03f5u, bytes[index]) != LIB_STATUS_OK)
            return LIB_FALSE;
    return test_board_fdc_advance_ticks(board, 1u);
}

static lib_i32 model40_fdc_result(core_machine_board_state *board, core_machine *machine,
    lib_u8 *result, lib_size count)
{
    lib_size index;
    lib_u32 value;
    if (!test_board_fdc_advance_ticks(board, 1u)) return LIB_FALSE;
    for (index = 0u; index < count; ++index) {
        if (core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK || (value &
            (TEST_FDC_MSR_RQM | TEST_FDC_MSR_DIO)) != (TEST_FDC_MSR_RQM | TEST_FDC_MSR_DIO))
            return LIB_FALSE;
        if (core_machine_bus_read(machine, 0x03f5u, &value) != LIB_STATUS_OK) return LIB_FALSE;
        result[index] = (lib_u8)value;
    }
    return core_machine_bus_read(machine, 0x03f4u, &value) == LIB_STATUS_OK &&
        (value & (TEST_FDC_MSR_CB | TEST_FDC_MSR_DIO)) == 0u;
}

static lib_bool model40_fdc_write_dma2(core_machine *machine, lib_u16 address,
    lib_u16 count)
{
    return core_machine_bus_write(machine, 0x000cu, 0u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0004u, address & 0xffu) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0004u, address >> 8u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0005u, count & 0xffu) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0005u, count >> 8u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0081u, 0u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x000bu, 0x46u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x000au, 0x02u) == LIB_STATUS_OK;
}
lib_i32 main(void)
{
    static lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 image[MODEL40_FDC_BYTES];
    static const lib_u8 specify[] = {0x03u, 0xdfu, 0x03u};
    static const lib_u8 specify_dma[] = {0x03u, 0xdfu, 0x02u};
    static const lib_u8 read_last[] = {0xe6u, 0u, 0u, 0u, 15u, 2u, 15u, 0x1bu, 0xffu};
    static const lib_u8 read_oob[] = {0xe6u, 0u, 0u, 0u, 16u, 2u, 16u, 0x1bu, 0xffu};
    vm_machine *session = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine *machine = LIB_NULL;
    lib_u8 result[7] = {0};
    lib_u32 index;
    lib_u32 value;
    lib_i32 failed = 1;
    static const lib_u8 boot_code[] = {
        0xfau, 0x31u, 0xc0u, 0x8eu, 0xd8u, 0xc6u, 0x06u, 0x00u, 0x05u, 0xa5u,
        0xf4u, 0xebu, 0xfdu
    };

    if (floppy_channel_matrix()) goto done;
    lib_memory_copy(image, boot_code, sizeof(boot_code));
    image[510u] = 0x55u;
    image[511u] = 0xaau;
    image[(15u - 1u) * 512u] = 0xa5u;
    if (vm_model40_fixture_create_bytes(even, odd, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || vm_machine_fdd_replace_bytes(session->floppy[0u], image,
            sizeof(image)) != LIB_FALSE) goto done;
    {
        board = session->board;
        machine = session->core_machine;
        if (core_machine_bus_write(machine, 0x0064u, 0xc0u) != LIB_STATUS_OK) goto done;
        test_board_kbc_advance(board, 1u);
        if (core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK ||
            value != 0xb4u) goto done;
        const core_machine_fdc_config config = test_board_fdc_connection_config(board);
        const core_machine_fdc_drive_bindings drives = test_board_fdc_drive_bindings(board);
        const lib_u64 byte_ticks = config.clock_ticks_per_second / 1000000u * 15u;
        if (config.irq != 6u || config.dma_channel != 2u ||
            config.ready_mask != 0x0fu ||
            config.clock_ticks_per_second != 16000000u ||
            drives.installed_mask != 0x03u ||
            drives.track_zero_active_low_mask != 0u ||
            session->construction.floppy_kind != VM_PROFILE_FLOPPY_525_1200K) goto done;
        static const struct { lib_u8 index; lib_u8 value; } cmos[] = {
            {0x14u, 0x41u}, {0x10u, 0x22u}, {0x12u, 0x80u},
            {0x19u, 0u}, {0x17u, 0u}, {0x18u, 0x04u}
        };
        for (index = 0u; index < sizeof(cmos) / sizeof(cmos[0]); ++index) {
            if (core_machine_bus_write(machine, 0x0070u, cmos[index].index) != LIB_STATUS_OK ||
                core_machine_bus_read(machine, 0x0071u, &value) != LIB_STATUS_OK ||
                value != cmos[index].value) goto done;
        }
        if (core_machine_bus_write(machine, 0x03f2u, 0x1cu) != LIB_STATUS_OK ||
            !test_board_fdc_advance_due(board) ||
            !test_board_fdc_irq_source_is_asserted(board)) goto done;
        for (index = 0u; index < 4u; ++index) {
            if (!model40_fdc_command(board, machine, (const lib_u8[]){0x08u}, 1u) ||
                !model40_fdc_result(board, machine, result, 2u) ||
                result[0] != (TEST_FDC_ST0_READY_CHANGE | index)) goto done;
        }
        if (!model40_fdc_command(board, machine, (const lib_u8[]){0x04u, 0x00u}, 2u) ||
            !model40_fdc_result(board, machine, result, 1u) || result[0] != 0x38u ||
            core_machine_bus_write(machine, 0x03f7u, 0u) != LIB_STATUS_OK ||
            !model40_fdc_command(board, machine, specify, sizeof(specify)) ||
            !model40_fdc_command(board, machine, read_last, sizeof(read_last)) ||
            vm_test_model40_observation(session).fdc_terminal_valid ||
            core_machine_bus_read(machine, 0x03f5u, &value) != LIB_STATUS_OK ||
            value != 0xa5u) goto done;
        for (index = 1u; index < 512u; ++index) {
            if (!test_board_fdc_advance_ticks(board, byte_ticks) ||
                core_machine_bus_read(machine, 0x03f5u, &value) != LIB_STATUS_OK) goto done;
        }
        if (!model40_fdc_result(board, machine, result, sizeof(result)) ||
            result[0] != TEST_FDC_ST0_NORMAL || result[1] != 0u ||
            result[5] != 16u || result[6] != 2u ||
            !vm_test_model40_observation(session).fdc_terminal_valid ||
            vm_test_model40_observation(session).fdc_terminal.command != 0xe6u ||
            vm_test_model40_observation(session).fdc_terminal.result[0] != result[0] ||
            vm_test_model40_observation(session).fdc_terminal.result[1] != result[1] ||
            !model40_fdc_command(board, machine, specify_dma, sizeof(specify_dma)) ||
            !model40_fdc_write_dma2(machine, 0x0600u, 511u) ||
            !model40_fdc_command(board, machine, read_last, sizeof(read_last))) goto done;
        for (index = 0u; index < 512u; ++index) {
            test_board_dma_transfers(board, machine, 1u, 2u);
            if (index + 1u < 512u && !test_board_fdc_advance_ticks(board, byte_ticks)) goto done;
        }
        if (core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK ||
            value != TEST_FDC_MSR_CB ||
            core_machine_memory_read(session->core_machine, 0x0600u, &result[0],
                sizeof(result[0])) != LIB_STATUS_OK || result[0] != 0xa5u ||
            !model40_fdc_result(board, machine, result, sizeof(result)) ||
            result[0] != TEST_FDC_ST0_NORMAL || result[1] != 0u ||
            !vm_test_model40_observation(session).fdc_terminal_valid ||
            !vm_test_model40_observation(session).fdc_terminal.successful ||
            vm_machine_reset(session) != LIB_STATUS_OK ||
            vm_test_model40_observation(session).fdc_terminal_valid ||
            !model40_fdc_command(board, machine, read_oob, sizeof(read_oob)) ||
            !model40_fdc_result(board, machine, result, sizeof(result)) ||
            result[0] != TEST_FDC_ST0_ABNORMAL || result[1] != 0x04u ||
            !vm_test_model40_observation(session).fdc_terminal_valid ||
            vm_test_model40_observation(session).fdc_terminal.successful ||
            vm_test_model40_observation(session).fdc_terminal.result[0] != result[0] ||
            vm_test_model40_observation(session).fdc_terminal.result[1] != result[1] ||
            vm_machine_fdd_remove_for(session->floppy[0u]) != LIB_FALSE) goto done;
        test_board_fdc_refresh(board);
        if (!model40_fdc_command(board, machine, read_last, sizeof(read_last)) ||
            core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK ||
            value != TEST_FDC_MSR_CB ||
            core_machine_bus_write(machine, 0x03f2u, 0u) != LIB_STATUS_OK ||
            core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK ||
            value != TEST_FDC_MSR_RQM || test_board_fdc_irq_source_is_asserted(board) ||
            core_machine_bus_write(machine, 0x03f2u, 0x1cu) != LIB_STATUS_OK ||
            !test_board_fdc_advance_due(board) ||
            !test_board_fdc_irq_source_is_asserted(board)) goto done;
        for (index = 0u; index < 4u; ++index) {
            if (!model40_fdc_command(board, machine, (const lib_u8[]){0x08u}, 1u) ||
                !model40_fdc_result(board, machine, result, 2u) ||
                result[0] != (TEST_FDC_ST0_READY_CHANGE | index)) goto done;
        }
        if (!model40_fdc_command(board, machine, (const lib_u8[]){0x08u}, 1u) ||
            !model40_fdc_result(board, machine, result, 1u) || result[0] != 0x80u ||
            !model40_fdc_command(board, machine, read_last, sizeof(read_last)) ||
            !test_board_fdc_advance_ticks(board, 1u) ||
            core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK || value !=
            (TEST_FDC_MSR_RQM | TEST_FDC_MSR_DIO | TEST_FDC_MSR_CB) ||
            !test_board_fdc_irq_source_is_asserted(board) ||
            !model40_fdc_result(board, machine, result, sizeof(result)) ||
            result[0] != TEST_FDC_ST0_ABNORMAL ||
            result[1] != 0x04u || result[2] != 0u ||
            !model40_fdc_command(board, machine, (const lib_u8[]){0x08u}, 1u) ||
            !model40_fdc_result(board, machine, result, 1u) ||
            result[0] != 0x80u || test_board_fdc_irq_source_is_asserted(board)) goto done;
    }
    {
        static const struct { lib_u8 bytes[9]; lib_u8 count; } commands[] = {
            {{0xe6u, 0u, 0u, 0u, 1u, 2u, 15u, 0x1bu, 0xffu}, 9u},
            {{0xe5u, 0u, 0u, 0u, 1u, 2u, 15u, 0x1bu, 0xffu}, 9u},
            {{0xecu, 0u, 0u, 0u, 1u, 2u, 15u, 0x1bu, 0xffu}, 9u},
            {{0xe9u, 0u, 0u, 0u, 1u, 2u, 15u, 0x1bu, 0xffu}, 9u},
            {{0x51u, 0u, 0u, 0u, 1u, 2u, 15u, 0x1bu, 1u}, 9u},
            {{0x59u, 0u, 0u, 0u, 1u, 2u, 15u, 0x1bu, 1u}, 9u},
            {{0x5du, 0u, 0u, 0u, 1u, 2u, 15u, 0x1bu, 1u}, 9u},
            {{0x42u, 0u, 0u, 0u, 1u, 2u, 15u, 0x1bu, 0xffu}, 9u},
            {{0x4au, 0u}, 2u},
            {{0x4du, 0u, 2u, 15u, 0x54u, 0xf6u}, 6u}
        };
        if (vm_machine_fdd_replace_bytes(session->floppy[0u], image, sizeof(image))) goto done;
        for (lib_u8 rate = 1u; rate <= 3u; ++rate) {
            for (lib_size command = 0u; command < sizeof(commands) / sizeof(commands[0]); ++command) {
                test_board_fdc_reset(board);
                if (core_machine_bus_write(machine, 0x03f2u, 0x1cu) != LIB_STATUS_OK ||
                    core_machine_bus_write(machine, 0x03f7u, rate) != LIB_STATUS_OK ||
                    !model40_fdc_command(board, machine, specify_dma, sizeof(specify_dma)) ||
                    !model40_fdc_command(board, machine, commands[command].bytes, commands[command].count) ||
                    !model40_fdc_result(board, machine, result, sizeof(result)) ||
                    result[0] != 0x40u || result[1] != 0x04u || result[2] != 0u) goto done;
            }
        }
        if (core_machine_bus_write(machine, 0x03f7u, 0u) != LIB_STATUS_OK ||
            !model40_fdc_command(board, machine, (const lib_u8[]){0x4au, 0u}, 2u) ||
            !model40_fdc_result(board, machine, result, sizeof(result)) ||
            result[0] != 0u || result[1] != 0u || result[2] != 0u) goto done;
    }
    {
        const vm_profile_model40_observation captured = vm_test_model40_observation(session);
        vm_profile_model40_observation detached = {0};

        if (!captured.d4.configured || !captured.fdc_terminal_valid ||
            vm_machine_finish_reset(session, LIB_STATUS_INTERNAL_ERROR) != LIB_STATUS_INTERNAL_ERROR ||
            !vm_test_model40_observation(session).fdc_terminal_valid) goto done;
        vm_machine_finalize(session);
        if (vm_profile_model40_observe(vm_test_profile_construction(session), &detached) !=
                LIB_STATUS_OK || detached.d4.configured || detached.fdc_terminal_valid ||
            !captured.d4.configured || !captured.fdc_terminal_valid) goto done;
    }
    failed = 0;
done:
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("FDC-12MB-LOGICAL:OK\n");
    printf("FDC-DMA2-IRQ6:OK\n");
    printf("MODEL40-FDC-BINDING:OK\n");
    printf("MODEL40-FDC-READY-MEDIA-SEPARATION:OK\n");
    printf("FLOPPY-CHANNEL-FORMAT-RATE-PITCH:OK\n");
    return 0;
}
