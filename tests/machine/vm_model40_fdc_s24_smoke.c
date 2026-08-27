#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/fdc.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/machine/fdd.h"
#include "../support/vm_model40_byob_fixture.h"

#define MODEL40_FDC_BYTES (80u * 2u * 15u * 512u)
static C_VOID model40_fdc_command(core_machine_fdc *fdc, t_port *port,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    STD_SIZE_T index;
    for (index = 0u; index < count; ++index)
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    core_machine_fdc_advance(fdc);
}

static C_INT model40_fdc_result(core_machine_fdc *fdc, t_port *port,
    type_unsigned_8 *result, STD_SIZE_T count)
{
    STD_SIZE_T index;
    core_machine_fdc_advance(fdc);
    for (index = 0u; index < count; ++index)
        result[index] = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
    return (core_machine_port_read(port, 0x03f4u) & (VFDC_MSR_CB | VFDC_MSR_DIO)) == 0u;
}

static C_VOID model40_fdc_write_dma2(t_port *port, type_unsigned_16 address,
    type_unsigned_16 count)
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
C_INT main(C_INT argc, C_CHAR **argv)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 image[MODEL40_FDC_BYTES];
    const vm_session_config byob_config = {
        .profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        .model40_firmware = {
            .even_path = argc == 6 ? argv[1] : STD_NULL,
            .even_sha256 = argc == 6 ? argv[2] : STD_NULL,
            .odd_path = argc == 6 ? argv[3] : STD_NULL,
            .odd_sha256 = argc == 6 ? argv[4] : STD_NULL,
            .provenance = argc == 6 ? argv[5] : STD_NULL
        }
    };
    static const type_unsigned_8 specify[] = {0x03u, 0xdfu, 0x03u};
    static const type_unsigned_8 specify_dma[] = {0x03u, 0xdfu, 0x02u};
    static const type_unsigned_8 read_last[] = {0xe6u, 0u, 0u, 0u, 15u, 2u, 15u, 0x1bu, 0xffu};
    static const type_unsigned_8 read_oob[] = {0xe6u, 0u, 0u, 0u, 16u, 2u, 16u, 0x1bu, 0xffu};
    vm_session *session = STD_NULL;
    core_machine_fdc *fdc;
    t_port *port;
    type_unsigned_8 result[7] = {0};
    type_unsigned_32 index;
    core_machine_run_result run;
    type_status create_status;
    type_unsigned_8 bios_marker = 0u;
    C_INT failed = argc != 1 && argc != 6;


    static const type_unsigned_8 boot_code[] = {
        0xfau, 0x31u, 0xc0u, 0x8eu, 0xd8u, 0xc6u, 0x06u, 0x00u, 0x05u, 0xa5u,
        0xf4u, 0xebu, 0xfdu
    };

    STD_MEMCPY(image, boot_code, sizeof(boot_code));
    image[510u] = 0x55u;
    image[511u] = 0xaau;
    image[(15u - 1u) * 512u] = 0xa5u;
    if (argc == 6) {
        create_status = (type_status)vm_session_create(&byob_config, &session);
    } else {
        create_status = vm_model40_fixture_create_bytes("t386-s24-even.bin", even,
            "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe",
            "t386-s24-odd.bin", odd,
            "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe",
            &session);
    }
    failed |= create_status != TYPE_STATUS_OK || session == STD_NULL || vm_machine_fdd_replace_bytes(&session->fdd, image,
        sizeof(image)) != TYPE_FALSE;
    if (!failed) {
        fdc = &session->core_machine->fdc;
        port = &session->core_machine->executor_port;
        failed |= fdc->connect.config.irq != 6u || fdc->connect.config.dma_channel != 2u ||
            fdc->connect.config.unready_read_policy !=
                CORE_MACHINE_FDC_UNREADY_READ_DESKPRO_REFERENCE ||
            session->floppy_kind != VM_PROFILE_FLOPPY_525_1200K;
        core_machine_port_write(port, 0x03f2u, 0x1cu);
        failed |= !fdc->connect.irq_source.asserted;
        model40_fdc_command(fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 2u) ||
            result[0] != core_machine_fdc_ST0_READY_CHANGE;
        core_machine_port_write(port, 0x03f7u, 0u);
        model40_fdc_command(fdc, port, specify, sizeof(specify));
        model40_fdc_command(fdc, port, read_last, sizeof(read_last));
        failed |= session->model40_fdc_terminal_observation_valid ||
            core_machine_port_read(port, 0x03f5u) != 0xa5u;
        for (index = 1u; index < 512u; ++index) {
            core_machine_fdc_advance_at(fdc, fdc->data.elapsed_ticks +
                128u);
            (C_VOID)core_machine_port_read(port, 0x03f5u);
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
            core_machine_dma_advance(&session->core_machine->shared_dma_latch,
                &session->core_machine->shared_dma_primary,
                &session->core_machine->shared_dma_secondary,
                &session->core_machine->executor_memory, 1u);
            if (index + 1u < 512u) core_machine_fdc_advance_at(fdc,
                fdc->data.elapsed_ticks + 128u);
        }
        failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMPLETE ||
            core_machine_memory_read(session->core_machine, 0x0600u, &result[0],
                sizeof(result[0])) != TYPE_STATUS_OK || result[0] != 0xa5u;
        failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != core_machine_fdc_ST0_NORMAL || result[1] != 0u ||
            !session->model40_fdc_terminal_observation_valid ||
            !session->model40_fdc_terminal_observation.successful;
        vm_session_reset(session);
        failed |= session->model40_fdc_terminal_observation_valid;
        model40_fdc_command(fdc, port, read_oob, sizeof(read_oob));
        failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != (core_machine_fdc_ST0_ABNORMAL |
                core_machine_fdc_ST0_NOT_READY) || result[1] != 0u ||
            !session->model40_fdc_terminal_observation_valid ||
            session->model40_fdc_terminal_observation.successful ||
            session->model40_fdc_terminal_observation.result[0] != result[0] ||
            session->model40_fdc_terminal_observation.result[1] != result[1];
        failed |= vm_machine_fdd_remove_for(&session->fdd, STD_NULL) != TYPE_FALSE;
        core_machine_fdc_refresh(fdc);
        model40_fdc_command(fdc, port, read_last, sizeof(read_last));
        failed |= fdc->data.phase != core_machine_fdc_PHASE_PENDING_COMPLETE;
        core_machine_port_write(port, 0x03f2u, 0u);
        failed |= fdc->data.phase != core_machine_fdc_PHASE_COMMAND ||
            fdc->connect.irq_source.asserted;
        core_machine_port_write(port, 0x03f2u, 0x1cu);
        for (index = 0u; index < CORE_MACHINE_FDC_DRIVE_COUNT; ++index) {
            model40_fdc_command(fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
            failed |= !model40_fdc_result(fdc, port, result, 2u) ||
                result[0] != (core_machine_fdc_ST0_READY_CHANGE | index);
        }
        model40_fdc_command(fdc, port, read_last, sizeof(read_last));
        core_machine_fdc_advance(fdc);
        failed |= fdc->data.phase != core_machine_fdc_PHASE_RESULT ||
            !fdc->connect.irq_source.asserted ||
            !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != (core_machine_fdc_ST0_ABNORMAL |
                core_machine_fdc_ST0_NOT_READY) || result[1] != 0u || result[2] != 0u;
        model40_fdc_command(fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 2u) ||
            result[0] != (core_machine_fdc_ST0_ABNORMAL |
                core_machine_fdc_ST0_NOT_READY) || fdc->connect.irq_source.asserted;
        if (argc == 6) {
            for (index = 0u; index < 400000u && bios_marker == 0u; index += 64u) {
                failed |= core_machine_run(session->core_machine,
                    (core_machine_run_budget){64u, 0u}, &run) != TYPE_STATUS_OK ||
                    run.reason == CORE_MACHINE_STOP_FAULT || core_machine_memory_read(
                        session->core_machine, 0x0500u, &bios_marker,
                        sizeof(bios_marker)) != TYPE_STATUS_OK;
                if (failed) break;
            }
            failed |= bios_marker != 0xa5u;
        }
    }
    if (failed && argc == 6) {
        STD_PRINTF("M5:T386:S25:BYOB-CONSUMER:NOT-REACHED\n");
    }
    vm_session_destroy(session);
    if (argc != 6) vm_model40_fixture_remove("t386-s24-even.bin", "t386-s24-odd.bin");
    if (failed) return 1;
    STD_PRINTF("M5:T386:S24:FDC-12MB-LOGICAL:OK\n");
    STD_PRINTF("M5:T386:S24:FDC-DMA2-IRQ6:OK\n");
    STD_PRINTF("M5:T386:S24:MODEL40-FDC-BINDING:OK\n");
    STD_PRINTF("M5:T431:S1:MODEL40-FDC-NOT-READY:OK\n");
    return 0;
}
