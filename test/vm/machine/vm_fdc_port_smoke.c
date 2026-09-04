#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "core/machine/machine.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "core/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "../support/rom/session_assets.h"

static C_VOID fdc_command(core_machine_fdc *fdc, t_port *port,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    STD_SIZE_T index;
    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
    core_machine_fdc_advance(fdc);
}

static C_INT fdc_read_result(core_machine_fdc *fdc, t_port *port, type_unsigned_8 *result,
    STD_SIZE_T count)
{
    STD_SIZE_T index;
    core_machine_fdc_advance(fdc);
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
    static const type_unsigned_8 write_sector[] = {
        0xc5u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 format_track[] = {
        0x4du, 0x00u, 0x02u, 0x01u, 0x1bu, 0xa5u
    };
    type_unsigned_8 format_id[] = { 0x00u, 0x00u, 0x01u, 0x02u };
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->active ||
        (port = session->core_machine->fdc.connect.port) == STD_NULL) return 1;
    core_machine_port_write(port, 0x03f2u, 0x1cu);

    /* No image is an FDC result, not a host or BIOS shortcut. */
    fdc_command(&session->core_machine->fdc, port, read_sector, sizeof(read_sector));
    failed |= (core_machine_port_read(port, 0x03f4u) & VFDC_MSR_DIO) == 0u;
    failed |= !fdc_read_result(&session->core_machine->fdc, port, result, sizeof(result));
    failed |= (result[0] & core_machine_fdc_ST0_ABNORMAL) == 0u;

    vm_machine_fdd_create_for(&session->fdd);
    core_machine_fdc_refresh(&session->core_machine->fdc);
    failed |= (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) == 0u;
    fdc_command(&session->core_machine->fdc, port, (const type_unsigned_8[]){ 0x0fu, 0x00u, 0x00u }, 3u);
    fdc_command(&session->core_machine->fdc, port, (const type_unsigned_8[]){ 0x08u }, 1u);
    failed |= !fdc_read_result(&session->core_machine->fdc, port, result, 2u);
    core_machine_fdc_refresh(&session->core_machine->fdc);
    failed |= (core_machine_port_read(port, 0x03f7u) & VFDC_DIR_DC) != 0u;

    fdc_command(&session->core_machine->fdc, port, specify_non_dma, sizeof(specify_non_dma));
    fdc_command(&session->core_machine->fdc, port, format_track, sizeof(format_track));
    fdc_command(&session->core_machine->fdc, port, format_id, sizeof(format_id));
    core_machine_fdc_advance(&session->core_machine->fdc);
    failed |= !core_machine_pic_scan_interrupt(
        session->core_machine->fdc.connect.irq_source.master,
        session->core_machine->fdc.connect.irq_source.slave);
    failed |= !fdc_read_result(&session->core_machine->fdc, port, result, sizeof(result));
    failed |= result[0] != core_machine_fdc_ST0_NORMAL;
    fdc_command(&session->core_machine->fdc, port, (const type_unsigned_8[]){ 0x08u }, 1u);
    failed |= !fdc_read_result(&session->core_machine->fdc, port, result, 2u);

    session->fdd.connect.flagReadOnly = TYPE_TRUE;
    fdc_command(&session->core_machine->fdc, port, write_sector, sizeof(write_sector));
    core_machine_port_write(port, 0x03f5u, 0x5au);
    failed |= !fdc_read_result(&session->core_machine->fdc, port, result, sizeof(result));
    failed |= (result[1] & 0x02u) == 0u;
    session->fdd.connect.flagReadOnly = TYPE_FALSE;

    /* 03h is the unsupported 1 Mbps encoding; 01h is the valid 300 kbps
     * 8272A rate used by 360 KB media. */
    core_machine_port_write(port, 0x03f7u, 0x03u);
    fdc_command(&session->core_machine->fdc, port, read_sector, sizeof(read_sector));
    failed |= !fdc_read_result(&session->core_machine->fdc, port, result, sizeof(result));
    failed |= (result[1] & 0x04u) == 0u;
    core_machine_port_write(port, 0x03f7u, 0x02u);

    fdc_command(&session->core_machine->fdc, port, read_sector, sizeof(read_sector));
    failed |= (core_machine_port_read(port, 0x03f4u) &
        (VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_NDM)) !=
        (VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_NDM);
    failed |= core_machine_port_read(port, 0x03f5u) != 0xa5u;
    for (type_unsigned_16 index = 1u; index < 512u; ++index) {
        (C_VOID)core_machine_port_read(port, 0x03f5u);
    }
    failed |= !fdc_read_result(&session->core_machine->fdc, port, result, sizeof(result));
    failed |= result[0] != core_machine_fdc_ST0_NORMAL;

    vm_session_destroy(session);
    if (failed) return 1;
    puts("M5:T231:S3:FDC-PORT:OK");
    return 0;
}
