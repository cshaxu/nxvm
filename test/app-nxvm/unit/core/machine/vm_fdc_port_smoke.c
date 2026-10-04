#include "lib/types/types_interface.h"
#include "../../../../x86/ibmpc-common/controller_fixture.h"
#include <stdio.h>

#include "x86/ibmpc-common/pic_bus_interface.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/media/fdd.h"
#include "support/rom/session_assets.h"

static lib_bool fdc_command(core_machine_board_state *board, core_machine *machine,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;
    for (index = 0u; index < count; ++index)
        if (core_machine_bus_write(machine, 0x03f5u, bytes[index]) != LIB_STATUS_OK)
            return LIB_FALSE;
    return test_board_fdc_advance_ticks(board, 1u) &&
        test_board_fdc_advance_ticks(board, 1u);
}

static lib_bool fdc_read_result(core_machine_board_state *board, core_machine *machine, lib_u8 *result,
    lib_size count)
{
    lib_size index;
    lib_u32 value;
    if (!test_board_fdc_advance_ticks(board, 1u)) return LIB_FALSE;
    for (index = 0u; index < count; ++index) {
        if (core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK ||
            (value & TEST_FDC_MSR_READY_READ) !=
            TEST_FDC_MSR_READY_READ) return LIB_FALSE;
        if (core_machine_bus_read(machine, 0x03f5u, &value) != LIB_STATUS_OK) return LIB_FALSE;
        result[index] = (lib_u8)value;
    }
    return core_machine_bus_read(machine, 0x03f4u, &value) == LIB_STATUS_OK &&
        (value & (TEST_FDC_MSR_CB | TEST_FDC_MSR_DIO)) == 0u;
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    core_machine *machine;
    lib_u32 value;
    lib_u8 result[7];
    static const lib_u8 specify_non_dma[] = { 0x03u, 0xdfu, 0x03u };
    static const lib_u8 read_sector[] = {
        0xe6u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 write_sector[] = {
        0xc5u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 format_track[] = {
        0x4du, 0x00u, 0x02u, 0x01u, 0x1bu, 0xa5u
    };
    lib_u8 format_id[] = { 0x00u, 0x00u, 0x01u, 0x02u };
    lib_i32 failed = 1;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !session->active ||
        session->core_machine == LIB_NULL) goto done;
    machine = session->core_machine;
    if (core_machine_bus_write(machine, 0x03f2u, 0x1cu) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x03f7u, 0x00u) != LIB_STATUS_OK) goto done;
    /* 1.44MB: 500 kbps. */

    /* No image is an FDC result, not a host or BIOS shortcut. */
    if (!fdc_command(session->board, machine, read_sector, sizeof(read_sector)) ||
        core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK ||
        (value & TEST_FDC_MSR_DIO) == 0u ||
        !fdc_read_result(session->board, machine, result, sizeof(result)) ||
        (result[0] & TEST_FDC_ST0_ABNORMAL) == 0u) goto done;

    vm_machine_fdd_create_for(&session->fdd);
    test_board_fdc_refresh(session->board);
    if (core_machine_bus_read(machine, 0x03f7u, &value) != LIB_STATUS_OK ||
        (value & 0x80u) == 0u) goto done;
    /* A real STEP clears disk-change; SEEK to the current PCN does not. */
    if (!fdc_command(session->board, machine, (const lib_u8[]){ 0x0fu, 0x00u, 0x01u }, 3u) ||
        !fdc_command(session->board, machine, (const lib_u8[]){ 0x08u }, 1u) ||
        !fdc_read_result(session->board, machine, result, 2u) ||
        !fdc_command(session->board, machine, (const lib_u8[]){ 0x0fu, 0x00u, 0x00u }, 3u) ||
        !fdc_command(session->board, machine, (const lib_u8[]){ 0x08u }, 1u) ||
        !fdc_read_result(session->board, machine, result, 2u)) goto done;
    test_board_fdc_refresh(session->board);
    if (core_machine_bus_read(machine, 0x03f7u, &value) != LIB_STATUS_OK ||
        (value & 0x80u) != 0u) goto done;

    if (!fdc_command(session->board, machine, specify_non_dma, sizeof(specify_non_dma)) ||
        !fdc_command(session->board, machine, format_track, sizeof(format_track)) ||
        !fdc_command(session->board, machine, format_id, sizeof(format_id)) ||
        !test_board_fdc_advance_ticks(session->board, 1u) ||
        !test_board_controller_scan_interrupt(session->board) ||
        !fdc_read_result(session->board, machine, result, sizeof(result)) ||
        result[0] != TEST_FDC_ST0_NORMAL ||
        !fdc_command(session->board, machine, (const lib_u8[]){ 0x08u }, 1u) ||
        !fdc_read_result(session->board, machine, result, 1u) ||
        result[0] != 0x80u) goto done;

    session->fdd.connect.flagReadOnly = LIB_TRUE;
    if (!fdc_command(session->board, machine, write_sector, sizeof(write_sector)) ||
        core_machine_bus_write(machine, 0x03f5u, 0x5au) != LIB_STATUS_OK ||
        !fdc_read_result(session->board, machine, result, sizeof(result)) ||
        (result[1] & 0x02u) == 0u) goto done;
    session->fdd.connect.flagReadOnly = LIB_FALSE;

    /* Reserved rate is rejected; restore this medium's 500-kbps rate. */
    if (core_machine_bus_write(machine, 0x03f7u, 0x03u) != LIB_STATUS_OK ||
        !fdc_command(session->board, machine, read_sector, sizeof(read_sector)) ||
        !fdc_read_result(session->board, machine, result, sizeof(result)) ||
        (result[1] & 0x04u) == 0u ||
        core_machine_bus_write(machine, 0x03f7u, 0x00u) != LIB_STATUS_OK) goto done;

    if (!fdc_command(session->board, machine, read_sector, sizeof(read_sector)) ||
        core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK || (value &
        (TEST_FDC_MSR_RQM | TEST_FDC_MSR_DIO | TEST_FDC_MSR_NDM)) !=
        (TEST_FDC_MSR_RQM | TEST_FDC_MSR_DIO | TEST_FDC_MSR_NDM) ||
        core_machine_bus_read(machine, 0x03f5u, &value) != LIB_STATUS_OK || value != 0xa5u)
        goto done;
    for (lib_u16 index = 1u; index < 512u; ++index)
        if (core_machine_bus_read(machine, 0x03f5u, &value) != LIB_STATUS_OK) goto done;
    if (!fdc_read_result(session->board, machine, result, sizeof(result)) ||
        result[0] != TEST_FDC_ST0_NORMAL) goto done;
    failed = 0;
done:
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T231:S3:FDC-PORT:OK");
    return 0;
}
