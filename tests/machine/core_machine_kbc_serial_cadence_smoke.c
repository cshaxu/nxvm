#include "type.h"
#include "core/machine/kbc.h"
#include "core/machine/port.h"

C_INT main(C_VOID)
{
    static const type_unsigned_8 bytes[] = { 0x1eu, 0x30u };
    t_kbc kbc;
    t_port port;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x07u);
    core_machine_kbc_set_serial_delivery_timing(&kbc, 2u);
    failed |= core_machine_kbc_submit_native_bytes(&kbc, bytes, sizeof(bytes)) != TYPE_STATUS_OK || kbc.data.fifo_count != 0u || kbc.data.keyboard_serial_count != sizeof(bytes);
    core_machine_kbc_advance(&kbc, 1u);
    failed |= kbc.data.fifo_count != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (type_unsigned_8)core_machine_port_read(&port, 0x0060u) != 0x1eu || kbc.data.keyboard_serial_count != 1u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= kbc.data.fifo_count != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (type_unsigned_8)core_machine_port_read(&port, 0x0060u) != 0x30u || kbc.data.keyboard_serial_count != 0u;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x2eu) != TYPE_STATUS_OK;
    core_machine_kbc_reset(&kbc);
    failed |= kbc.data.keyboard_serial_count != 0u || kbc.data.fifo_count != 0u || kbc.data.serial_delivery_ticks != 2u || kbc.data.serial_delivery_remaining_ticks != 0u;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T406:S1:KBC-SERIAL-CADENCE:OK\n");
    return 0;
}