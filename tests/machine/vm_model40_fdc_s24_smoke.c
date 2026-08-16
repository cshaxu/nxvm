#include "type.h"

#include "core/machine/fdc.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/session.h"
#include "vm/machine/fdd.h"

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

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 image[MODEL40_FDC_BYTES];
    const vm_profile_model40_external_rom rom = { even, odd, sizeof(even) };
    static const type_unsigned_8 specify[] = {0x03u, 0xdfu, 0x03u};
    static const type_unsigned_8 read_last[] = {0xe6u, 0u, 0u, 0u, 15u, 2u, 15u, 0x1bu, 0xffu};
    static const type_unsigned_8 read_oob[] = {0xe6u, 0u, 0u, 0u, 16u, 2u, 16u, 0x1bu, 0xffu};
    vm_session *session = STD_NULL;
    core_machine_fdc *fdc;
    t_port *port;
    type_unsigned_8 result[7] = {0};
    type_unsigned_32 index;
    C_INT failed = 0;


    image[(15u - 1u) * 512u] = 0xa5u;
    failed |= vm_session_create_model40_private(&rom, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || vm_machine_fdd_replace_bytes(&session->fdd, image,
        sizeof(image)) != TYPE_FALSE;
    if (!failed) {
        fdc = &session->core_machine->fdc;
        port = &session->core_machine->executor_port;
        failed |= fdc->connect.config.irq != 6u || fdc->connect.config.dma_channel != 2u ||
            session->floppy_kind != VM_PROFILE_FLOPPY_525_1200K;
        core_machine_port_write(port, 0x03f2u, 0x1cu);
        failed |= !fdc->connect.irq_source.asserted;
        model40_fdc_command(fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
        failed |= !model40_fdc_result(fdc, port, result, 2u) ||
            result[0] != core_machine_fdc_ST0_READY_CHANGE;
        core_machine_port_write(port, 0x03f7u, 0u);
        model40_fdc_command(fdc, port, specify, sizeof(specify));
        model40_fdc_command(fdc, port, read_last, sizeof(read_last));
        failed |= core_machine_port_read(port, 0x03f5u) != 0xa5u;
        for (index = 1u; index < 512u; ++index) {
            core_machine_fdc_advance_at(fdc, fdc->data.elapsed_ticks +
                CORE_MACHINE_FDC_500K_BYTE_TICKS);
            (C_VOID)core_machine_port_read(port, 0x03f5u);
        }
        failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != core_machine_fdc_ST0_NORMAL || result[1] != 0u ||
            result[5] != 16u || result[6] != 2u;
        model40_fdc_command(fdc, port, read_oob, sizeof(read_oob));
        failed |= !model40_fdc_result(fdc, port, result, sizeof(result)) ||
            result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u;
    }
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T386:S24:FDC-12MB-LOGICAL:OK\n");
    STD_PRINTF("M5:T386:S24:FDC-DMA2-IRQ6:OK\n");
    STD_PRINTF("M5:T386:S24:MODEL40-FDC-BINDING:OK\n");
    return 0;
}