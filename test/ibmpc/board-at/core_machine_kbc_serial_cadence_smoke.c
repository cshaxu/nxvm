#include "lib/types/types_interface.h"
#include <stdio.h>
#include "ibmpc/board-at/kbc.h"
#include "kbc_fixture.h"

lib_i32 main(void)
{
    static const lib_u8 bytes[] = { 0x1eu, 0x30u };
    t_kbc kbc;
    core_machine *port = test_kbc_create(&kbc);
    lib_i32 failed = 0;
    lib_u64 ticks = 0u;

    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x07u);
    core_machine_kbc_set_serial_delivery_timing(&kbc, 2u);
    failed |= core_machine_kbc_submit_native_bytes(&kbc, bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        (test_kbc_port_read(port, 0x64u) & VKBC_STATUS_OBF) != 0u;
    failed |= core_machine_kbc_ticks_until_event(&kbc, &ticks) != LIB_STATUS_OK || ticks != 2u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (test_kbc_port_read(port, 0x64u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (lib_u8)test_kbc_port_read(port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (test_kbc_port_read(port, 0x64u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (lib_u8)test_kbc_port_read(port, 0x0060u) != 0x30u;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x2eu) != LIB_STATUS_OK;
    core_machine_kbc_reset(&kbc);
    failed |= (test_kbc_port_read(port, 0x64u) & VKBC_STATUS_OBF) != 0u ||
        core_machine_kbc_ticks_until_event(&kbc, &ticks) != LIB_STATUS_INVALID_STATE;
    core_machine_kbc_advance(&kbc, 10u);
    failed |= (test_kbc_port_read(port, 0x64u) & VKBC_STATUS_OBF) != 0u;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x05u) != LIB_STATUS_OK;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (test_kbc_port_read(port, 0x64u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (lib_u8)test_kbc_port_read(port, 0x60u) != 0x3bu;
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    if (failed) return 1;
    printf("M5:T406:S1:KBC-SERIAL-CADENCE:OK\n");
    return 0;
}
