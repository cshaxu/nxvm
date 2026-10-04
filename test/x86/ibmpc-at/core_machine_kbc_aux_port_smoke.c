#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86/ibmpc-at/kbc.h"
#include "x86/ibmpc-common/pic_bus_interface.h"
#include "kbc_fixture.h"
#include "../ibmpc-common/kbc_irq_fixture.h"

static lib_u8 read_port(core_machine *port, lib_u16 id)
{
    return (lib_u8)test_kbc_port_read(port, id);
}

static void initialize_pic(core_machine *port)
{
    test_kbc_port_write(port, 0x0020u, 0x11u);
    test_kbc_port_write(port, 0x0021u, 0x08u);
    test_kbc_port_write(port, 0x0021u, 0x04u);
    test_kbc_port_write(port, 0x0021u, 0x01u);
    test_kbc_port_write(port, 0x00a0u, 0x11u);
    test_kbc_port_write(port, 0x00a1u, 0x70u);
    test_kbc_port_write(port, 0x00a1u, 0x02u);
    test_kbc_port_write(port, 0x00a1u, 0x01u);
}

static lib_i32 take_aux_byte(core_machine *port, core_machine_pic_bus *master, core_machine_pic_bus *slave,
    lib_u8 expected)
{
    core_machine_pic_refresh(master, slave);
    if ((read_port(port, 0x0064u) & (VKBC_STATUS_OBF | VKBC_STATUS_AUX)) !=
        (VKBC_STATUS_OBF | VKBC_STATUS_AUX) ||
        core_machine_pic_get_interrupt(master, slave) != 0x74u ||
        read_port(port, 0x0060u) != expected) return 0;
    test_kbc_port_write(port, 0x00a0u, 0x20u);
    test_kbc_port_write(port, 0x0020u, 0x20u);
    return 1;
}

static void send_aux_command(core_machine *port, lib_u8 command)
{
    test_kbc_port_write(port, 0x0064u, 0xd4u);
    test_kbc_port_write(port, 0x0060u, command);
}

static void send_aux_parameter(core_machine *port, lib_u8 value)
{
    test_kbc_port_write(port, 0x0064u, 0xd4u);
    test_kbc_port_write(port, 0x0060u, value);
}

lib_i32 main(void)
{
    t_kbc kbc;
    test_kbc_irq_wiring irq = {0};
    core_machine_pic_bus *master = LIB_NULL;
    core_machine_pic_bus *slave = LIB_NULL;
    core_machine *machine = test_kbc_create(&kbc);
    core_machine *port = machine;
    lib_i32 failed = 0;
    lib_i32 stage = 1;
    lib_u8 index;

    if (core_machine_pic_initialize(&master, &slave, machine,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        test_kbc_bind_irq(&irq, master, slave) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_kbc_bind_core_services(&kbc, test_kbc_irq_output, &irq,
        LIB_NULL, LIB_NULL, LIB_NULL, LIB_NULL, LIB_TRUE);
    initialize_pic(port);

    test_kbc_port_write(port, 0x0064u, 0x20u);
    failed |= (read_port(port, 0x0064u) & VKBC_STATUS_AUX) != 0u;
    failed |= read_port(port, 0x0060u) != 0x43u;
    test_kbc_port_write(port, 0x0064u, 0xa9u);
    failed |= (read_port(port, 0x0064u) & VKBC_STATUS_AUX) != 0u;
    failed |= core_machine_pic_scan_interrupt(master, slave);
    failed |= read_port(port, 0x0060u) != 0x00u;

    send_aux_command(port, 0xf2u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    failed |= !take_aux_byte(port, master, slave, 0x00u);
    stage = 2;
    send_aux_command(port, 0xf4u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 5, -3, 0x01u) !=
        LIB_STATUS_OK;
    failed |= !take_aux_byte(port, master, slave, 0x29u);
    failed |= !take_aux_byte(port, master, slave, 0x05u);
    failed |= !take_aux_byte(port, master, slave, 0xfdu);

    test_kbc_port_write(port, 0x0064u, 0xa7u);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) !=
        LIB_STATUS_INVALID_STATE;
    core_machine_pic_refresh(master, slave);
    failed |= core_machine_pic_scan_interrupt(master, slave);
    test_kbc_port_write(port, 0x0064u, 0xa8u);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) !=
        LIB_STATUS_OK;
    failed |= !take_aux_byte(port, master, slave, 0x09u);
    failed |= !take_aux_byte(port, master, slave, 0x01u);
    failed |= !take_aux_byte(port, master, slave, 0x01u);

    send_aux_command(port, 0xf5u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0u) !=
        LIB_STATUS_INVALID_STATE;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= (read_port(port, 0x0064u) & VKBC_STATUS_AUX) != 0u;
    core_machine_pic_refresh(master, slave);
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x09u;
    failed |= read_port(port, 0x0060u) != 0x03u;
    test_kbc_port_write(port, 0x0020u, 0x20u);

    send_aux_command(port, 0xf4u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    send_aux_command(port, 0xf3u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    send_aux_parameter(port, 200u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    send_aux_command(port, 0xf3u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    send_aux_parameter(port, 15u);
    failed |= !take_aux_byte(port, master, slave, 0xfeu);
    send_aux_command(port, 0xe8u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    send_aux_parameter(port, 3u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    send_aux_command(port, 0xe8u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    send_aux_parameter(port, 4u);
    failed |= !take_aux_byte(port, master, slave, 0xfeu);

    if (failed) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:CONFIG\n");
        goto done;
    }
    stage = 3;
    send_aux_command(port, 0xe9u);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) !=
        LIB_STATUS_OK;
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    failed |= !take_aux_byte(port, master, slave, 0x21u);
    failed |= !take_aux_byte(port, master, slave, 0x03u);
    failed |= !take_aux_byte(port, master, slave, 200u);
    failed |= !take_aux_byte(port, master, slave, 0x09u);
    failed |= !take_aux_byte(port, master, slave, 0x01u);
    failed |= !take_aux_byte(port, master, slave, 0x01u);
    if (failed) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:E9-ORDER:status=%02X:bat=%u\n",
            (unsigned int)read_port(port, 0x64u),
            (unsigned int)x86_keyboard_get_signals(kbc.connect.keyboard).bat_ready);
        goto done;
    }
    stage = 4;
    send_aux_command(port, 0xe9u);
    failed |= !take_aux_byte(port, master, slave, 0xfau);
    failed |= !take_aux_byte(port, master, slave, 0x21u);
    failed |= !take_aux_byte(port, master, slave, 0x03u);
    failed |= !take_aux_byte(port, master, slave, 200u);
    if (failed) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:E9-BUTTONS\n");
        goto done;
    }
    stage = 5;
    send_aux_command(port, 0xf6u);
    if (!take_aux_byte(port, master, slave, 0xfau)) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:F6\n");
        failed = 1;
    }
    send_aux_command(port, 0xe9u);
    if (!take_aux_byte(port, master, slave, 0xfau)) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:E9-ACK\n");
        failed = 1;
    }
    if (!take_aux_byte(port, master, slave, 0x00u)) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:E9-STATUS\n");
        failed = 1;
    }
    if (!take_aux_byte(port, master, slave, 0x02u)) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:E9-RESOLUTION\n");
        failed = 1;
    }
    if (!take_aux_byte(port, master, slave, 100u)) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:E9-RATE\n");
        failed = 1;
    }

    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x05u);
    send_aux_command(port, 0xf2u);
    failed |= (read_port(port, 0x0064u) & (VKBC_STATUS_OBF | VKBC_STATUS_AUX)) !=
        (VKBC_STATUS_OBF | VKBC_STATUS_AUX) ||
        core_machine_pic_scan_interrupt(master, slave) ||
        read_port(port, 0x0060u) != 0xfau ||
        read_port(port, 0x0060u) != 0x00u;
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x07u);

    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    send_aux_command(port, 0xf2u);
    failed |= (read_port(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (read_port(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= !take_aux_byte(port, master, slave, 0xfau) ||
        !take_aux_byte(port, master, slave, 0x00u);
    core_machine_kbc_set_command_response_timing(&kbc, 0u);

    send_aux_command(port, 0xf4u);
    failed |= !take_aux_byte(port, master, slave, 0xfau) ||
        core_machine_kbc_submit_aux_report(&kbc, 0, 0, 0u) != LIB_STATUS_OK ||
        (read_port(port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        core_machine_kbc_submit_aux_report(&kbc, 300, -300, 0u) != LIB_STATUS_OK ||
        !take_aux_byte(port, master, slave, 0xe8u) ||
        !take_aux_byte(port, master, slave, 0xffu) ||
        !take_aux_byte(port, master, slave, 0x00u);

    send_aux_command(port, 0xefu);
    failed |= !take_aux_byte(port, master, slave, 0xfeu);
    send_aux_command(port, 0xffu);
    failed |= !take_aux_byte(port, master, slave, 0xfau) ||
        !take_aux_byte(port, master, slave, 0xaau) ||
        !take_aux_byte(port, master, slave, 0x00u);
    send_aux_command(port, 0xe9u);
    failed |= !take_aux_byte(port, master, slave, 0xfau) ||
        !take_aux_byte(port, master, slave, 0x00u) ||
        !take_aux_byte(port, master, slave, 0x02u) ||
        !take_aux_byte(port, master, slave, 100u) ||
        core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0u) !=
            LIB_STATUS_INVALID_STATE;

    send_aux_command(port, 0xf4u);
    failed |= !take_aux_byte(port, master, slave, 0xfau) ||
        core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0u) != LIB_STATUS_OK ||
        !core_machine_pic_irq_source_is_asserted(irq.auxiliary);
    /* Keyboard scan bytes and AUX packets now have distinct device-side
     * admission queues.  Fill the KBC-visible AUX FIFO independently: 3
     * initial packet bytes plus these packets leave one byte, so the next
     * complete packet must be rejected atomically. */
    for (index = 0u; index < 20u; ++index) {
        failed |= core_machine_kbc_submit_aux_report(&kbc,
            (lib_i16)(index + 1u), 0, 0u) != LIB_STATUS_OK;
    }
    failed |= core_machine_kbc_submit_aux_report(&kbc, 2, 2, 1u) !=
        LIB_STATUS_INVALID_STATE ||
        core_machine_kbc_submit_aux_report(&kbc, 0, 0, 0u) != LIB_STATUS_OK;

    /* A saturated KBC output FIFO must not make keyboard serial input look
     * like AUX state.  Its private queue has its own bounded admission. */
    for (index = 0u; index < 64u; ++index) {
        failed |= core_machine_kbc_submit_native_byte(&kbc, index) != LIB_STATUS_OK;
    }
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0u) != LIB_STATUS_NO_MEMORY;
done:
    core_machine_kbc_finalize(&kbc);
    failed |= core_machine_pic_irq_source_is_asserted(irq.auxiliary);

    core_machine_pic_finalize(master, slave);
    core_machine_destroy(machine);
    if (failed) {
        fprintf(stderr, "M5:T267:AUX:PORT:FAIL:STAGE=%d\n", stage);
        return 1;
    }
    printf("M5:T267:S1:AUX:PORT:OK\n");
    return 0;
}
