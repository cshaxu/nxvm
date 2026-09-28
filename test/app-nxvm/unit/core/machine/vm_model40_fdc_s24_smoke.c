#include "../devices/support/dma_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/dma_bus.h"
#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/media/fdd.h"
#include "support/rom/model40_session_assets.h"

#define MODEL40_FDC_BYTES (80u * 2u * 15u * 512u)
static lib_i32 floppy_channel_matrix(void)
{
    static const lib_u8 allowed[] = {0x0fu, 0x06u, 0x04u, 0x08u};
    static const lib_u32 rates[] = {500000u, 300000u, 250000u};
    static const lib_u16 positions[] = {0u, 1u, 2u, 39u, 40u, 78u, 79u, 80u};
    lib_i32 failed = 0;

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
                    failed |= actual != expected ||
                        (actual && cylinder != positions[position] / step);
                    failed |= channel.sample(channel.context, 0u, geometry,
                        positions[position], rates[rate], LIB_FALSE, &cylinder);
                }
            }
        }
    }
    return failed;
}

static void model40_fdc_command(core_machine_fdc *fdc, t_port *port,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;
    for (index = 0u; index < count; ++index)
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    core_machine_fdc_advance(fdc);
}

static lib_i32 model40_fdc_result(core_machine_fdc *fdc, t_port *port,
    lib_u8 *result, lib_size count)
{
    lib_size index;
    core_machine_fdc_advance(fdc);
    for (index = 0u; index < count; ++index) {
        if ((core_machine_port_read(port, 0x03f4u) &
            (VFDC_MSR_RQM | VFDC_MSR_DIO)) != (VFDC_MSR_RQM | VFDC_MSR_DIO))
            return LIB_FALSE;
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) & (VFDC_MSR_CB | VFDC_MSR_DIO)) == 0u;
}

static void model40_fdc_write_dma2(t_port *port, lib_u16 address,
    lib_u16 count)
{
    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, 0x0004u, address & 0xffu);
    core_machine_port_write(port, 0x0004u, address >> 8u);
    core_machine_port_write(port, 0x0005u, count & 0xffu);
    core_machine_port_write(port, 0x0005u, count >> 8u);
    core_machine_port_write(port, 0x0081u, 0u);
    core_machine_port_write(port, 0x000bu, 0x46u);
    core_machine_port_write(port, 0x000au, 0x02u);
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
    core_machine_fdc *fdc = LIB_NULL;
    t_port *port = LIB_NULL;
    lib_u8 result[7] = {0};
    lib_u32 index;
    lib_status create_status;
    lib_i32 failed = floppy_channel_matrix();


    static const lib_u8 boot_code[] = {
        0xfau, 0x31u, 0xc0u, 0x8eu, 0xd8u, 0xc6u, 0x06u, 0x00u, 0x05u, 0xa5u,
        0xf4u, 0xebu, 0xfdu
    };

    lib_memory_copy(image, boot_code, sizeof(boot_code));
    image[510u] = 0x55u;
    image[511u] = 0xaau;
    image[(15u - 1u) * 512u] = 0xa5u;
    create_status = vm_model40_fixture_create_bytes(even, odd, &session);
    failed |= create_status != LIB_STATUS_OK || session == LIB_NULL || vm_machine_fdd_replace_bytes(&session->fdd, image,
        sizeof(image)) != LIB_FALSE;
    if (!failed) {
        fdc = &session->core_machine->fdc;
        port = &session->core_machine->executor_port;
        core_machine_port_write(port, 0x0064u, 0xc0u);
        core_machine_kbc_advance(&session->core_machine->shared_kbc, 1u);
        failed |= core_machine_port_read(port, 0x0060u) != 0xb4u;
        failed |= fdc->connect.config.irq != 6u || fdc->connect.config.dma_channel != 2u ||
            fdc->connect.config.ready_mask != 0x0fu ||
            fdc->connect.config.clock_ticks_per_second != 8000000u ||
            fdc->connect.drives.installed_mask != 0x03u ||
            fdc->connect.drives.track_zero_active_low_mask != 0u ||
            session->floppy_kind != VM_PROFILE_FLOPPY_525_1200K;
        core_machine_port_write(port, 0x0070u, 0x14u);
        failed |= core_machine_port_read(port, 0x0071u) != 0x41u;
        core_machine_port_write(port, 0x0070u, 0x10u);
        failed |= core_machine_port_read(port, 0x0071u) != 0x22u;
        core_machine_port_write(port, 0x0070u, 0x12u);
        failed |= core_machine_port_read(port, 0x0071u) != 0x80u;
        core_machine_port_write(port, 0x0070u, 0x19u);
        failed |= core_machine_port_read(port, 0x0071u) != 0u;
        core_machine_port_write(port, 0x0070u, 0x17u);
        failed |= core_machine_port_read(port, 0x0071u) != 0u;
        core_machine_port_write(port, 0x0070u, 0x18u);
        failed |= core_machine_port_read(port, 0x0071u) != 0x04u;
        core_machine_port_write(port, 0x03f2u, 0x1cu);
        core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
        failed |= !fdc->connect.irq_source.asserted;
        model40_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 2u) ||
            result[0] != core_machine_fdc_ST0_READY_CHANGE;
        model40_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 2u) ||
            result[0] != (core_machine_fdc_ST0_READY_CHANGE | 1u);
        model40_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 2u) ||
            result[0] != (core_machine_fdc_ST0_READY_CHANGE | 2u);
        model40_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 2u) ||
            result[0] != (core_machine_fdc_ST0_READY_CHANGE | 3u);
        model40_fdc_command(fdc, port, (const lib_u8[]){0x04u, 0x00u}, 2u);
        failed |= !model40_fdc_result(fdc, port, result, 1u) || result[0] != 0x38u;
        core_machine_port_write(port, 0x03f7u, 0u);
        model40_fdc_command(fdc, port, specify, sizeof(specify));
        model40_fdc_command(fdc, port, read_last, sizeof(read_last));
        failed |= session->model40_fdc_terminal_observation_valid ||
            core_machine_port_read(port, 0x03f5u) != 0xa5u;
        for (index = 1u; index < 512u; ++index) {
            core_machine_fdc_advance_at(fdc, fdc->data.elapsed_ticks +
                128u);
            (void)core_machine_port_read(port, 0x03f5u);
        }
        failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != core_machine_fdc_ST0_NORMAL || result[1] != 0u ||
            result[5] != 16u || result[6] != 2u ||
            !session->model40_fdc_terminal_observation_valid ||
            session->model40_fdc_terminal_observation.command != 0xe6u ||
            session->model40_fdc_terminal_observation.result[0] != result[0] ||
            session->model40_fdc_terminal_observation.result[1] != result[1];
        model40_fdc_command(fdc, port, specify_dma, sizeof(specify_dma));
        model40_fdc_write_dma2(port, 0x0600u, 511u);
        model40_fdc_command(fdc, port, read_last, sizeof(read_last));
        for (index = 0u; index < 512u; ++index) {
            test_dma_transfers(&session->core_machine->shared_dma_latch,
                &session->core_machine->shared_dma_primary,
                &session->core_machine->shared_dma_secondary,
                &session->core_machine->executor_memory, &session->core_machine->executor_port, 1u);
            if (index + 1u < 512u) core_machine_fdc_advance_at(fdc,
                fdc->data.elapsed_ticks + 128u);
        }
        failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMPLETE ||
            core_machine_memory_read(session->core_machine, 0x0600u, &result[0],
                sizeof(result[0])) != LIB_STATUS_OK || result[0] != 0xa5u;
        failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != core_machine_fdc_ST0_NORMAL || result[1] != 0u ||
            !session->model40_fdc_terminal_observation_valid ||
            !session->model40_fdc_terminal_observation.successful;
        vm_machine_reset(session);
        failed |= session->model40_fdc_terminal_observation_valid;
        model40_fdc_command(fdc, port, read_oob, sizeof(read_oob));
        failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u ||
            !session->model40_fdc_terminal_observation_valid ||
            session->model40_fdc_terminal_observation.successful ||
            session->model40_fdc_terminal_observation.result[0] != result[0] ||
            session->model40_fdc_terminal_observation.result[1] != result[1];
        failed |= vm_machine_fdd_remove_for(&session->fdd) != LIB_FALSE;
        core_machine_fdc_refresh(fdc);
        model40_fdc_command(fdc, port, read_last, sizeof(read_last));
        failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMPLETE;
        core_machine_port_write(port, 0x03f2u, 0u);
        failed |= fdc->data.phase != core_machine_fdc_PHASE_COMMAND ||
            fdc->connect.irq_source.asserted;
        core_machine_port_write(port, 0x03f2u, 0x1cu);
        core_machine_fdc_advance_at(fdc, fdc->data.reset_due_tick);
        failed |= !fdc->connect.irq_source.asserted;
        for (index = 0u; index < 4u; ++index) {
            model40_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
            failed |= !model40_fdc_result(fdc, port, result, 2u) ||
                result[0] != (core_machine_fdc_ST0_READY_CHANGE | index);
        }
        model40_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 1u) || result[0] != 0x80u;
        model40_fdc_command(fdc, port, read_last, sizeof(read_last));
        core_machine_fdc_advance(fdc);
        failed |= fdc->data.phase != core_machine_fdc_PHASE_RESULT ||
            !fdc->connect.irq_source.asserted ||
            !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != core_machine_fdc_ST0_ABNORMAL ||
            result[1] != 0x04u || result[2] != 0u;
        model40_fdc_command(fdc, port, (const lib_u8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 1u) ||
            result[0] != 0x80u || fdc->connect.irq_source.asserted;
    }
    if (session != LIB_NULL && fdc != LIB_NULL) {
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
        failed |= vm_machine_fdd_replace_bytes(&session->fdd, image, sizeof(image));
        for (lib_u8 rate = 1u; rate <= 3u; ++rate) {
            for (lib_size command = 0u; command < sizeof(commands) / sizeof(commands[0]); ++command) {
                core_machine_fdc_reset(fdc);
                core_machine_port_write(port, 0x03f2u, 0x1cu);
                core_machine_port_write(port, 0x03f7u, rate);
                model40_fdc_command(fdc, port, specify_dma, sizeof(specify_dma));
                model40_fdc_command(fdc, port, commands[command].bytes, commands[command].count);
                failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
                    result[0] != 0x40u || result[1] != 0x04u || result[2] != 0u;
            }
        }
        core_machine_port_write(port, 0x03f7u, 0u);
        model40_fdc_command(fdc, port, (const lib_u8[]){0x4au, 0u}, 2u);
        failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != 0u || result[1] != 0u || result[2] != 0u;
    }
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("M5:T386:S24:FDC-12MB-LOGICAL:OK\n");
    printf("M5:T386:S24:FDC-DMA2-IRQ6:OK\n");
    printf("M5:T386:S24:MODEL40-FDC-BINDING:OK\n");
    printf("M5:T539:S12:MODEL40-FDC-READY-MEDIA-SEPARATION:OK\n");
    printf("M5:T539:S12:FLOPPY-CHANNEL-FORMAT-RATE-PITCH:OK\n");
    return 0;
}
