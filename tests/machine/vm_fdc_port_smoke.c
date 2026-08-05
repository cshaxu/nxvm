#include "type.h"

#include "core/machine/port.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/machine/fdc.h"
#include "vm/machine/fdd.h"

static C_VOID fdc_command(t_port *port, const type_unsigned_8 *bytes,
    STD_SIZE_T count)
{
    STD_SIZE_T index;
    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
}

static C_INT fdc_read_result(t_port *port, type_unsigned_8 *result,
    STD_SIZE_T count)
{
    STD_SIZE_T index;
    for (index = 0u; index < count; ++index) {
        result[index] = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) & (VFDC_MSR_CB | VFDC_MSR_DIO)) == 0u;
}

C_INT main(C_VOID)
{
    vm_session *session;
    t_port *port;
    type_unsigned_8 result[7];
    static const type_unsigned_8 specify_non_dma[] = { 0x03u, 0xdfu, 0x03u };
    static const type_unsigned_8 read_sector[] = {
        0xe6u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 format_track[] = {
        0x4du, 0x00u, 0x02u, 0x01u, 0x1bu, 0xa5u
    };
    type_unsigned_8 format_id[] = { 0x00u, 0x00u, 0x01u, 0x02u };
    C_INT failed = 0;

    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    port = session->fdc.connect.port;
    if (!session->active || port == STD_NULL) failed = 1;
    core_machine_port_write(port, 0x03f2u, 0x0cu);

    /* No image is an FDC result, not a host or BIOS shortcut. */
    fdc_command(port, read_sector, sizeof(read_sector));
    failed |= (core_machine_port_read(port, 0x03f4u) & VFDC_MSR_DIO) == 0u;
    failed |= !fdc_read_result(port, result, sizeof(result));
    failed |= (result[0] & VM_MACHINE_FDC_ST0_ABNORMAL) == 0u;

    vm_machine_fdd_create_for(&session->fdd);
    vm_machine_fdc_refresh(&session->fdc);
    failed |= (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u;
    fdc_command(port, (const type_unsigned_8[]){ 0x0fu, 0x00u, 0x00u }, 3u);
    fdc_command(port, (const type_unsigned_8[]){ 0x08u }, 1u);
    failed |= !fdc_read_result(port, result, 2u);
    vm_machine_fdc_refresh(&session->fdc);
    failed |= (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) != 0u;

    fdc_command(port, specify_non_dma, sizeof(specify_non_dma));
    fdc_command(port, format_track, sizeof(format_track));
    fdc_command(port, format_id, sizeof(format_id));
    failed |= !fdc_read_result(port, result, sizeof(result));
    failed |= result[0] != VM_MACHINE_FDC_ST0_NORMAL;

    fdc_command(port, read_sector, sizeof(read_sector));
    failed |= (core_machine_port_read(port, 0x03f4u) &
        (VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_NDM)) !=
        (VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_NDM);
    failed |= core_machine_port_read(port, 0x03f5u) != 0xa5u;
    for (type_unsigned_16 index = 1u; index < 512u; ++index) {
        (C_VOID)core_machine_port_read(port, 0x03f5u);
    }
    failed |= !fdc_read_result(port, result, sizeof(result));
    failed |= result[0] != VM_MACHINE_FDC_ST0_NORMAL;

    vm_session_finalize(session);
    STD_FREE(session);
    if (failed) return 1;
    puts("M5:T231:S1:FDC-PORT:OK");
    return 0;
}
