#include "type.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/kbc.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"

static type_unsigned_8 read_port(t_port *port, type_unsigned_16 id)
{
    return (type_unsigned_8)core_machine_port_read(port, id);
}

static C_VOID initialize_pic(t_port *port)
{
    core_machine_port_write(port, 0x0020u, 0x11u);
    core_machine_port_write(port, 0x0021u, 0x08u);
    core_machine_port_write(port, 0x0021u, 0x04u);
    core_machine_port_write(port, 0x0021u, 0x01u);
    core_machine_port_write(port, 0x00a0u, 0x11u);
    core_machine_port_write(port, 0x00a1u, 0x70u);
    core_machine_port_write(port, 0x00a1u, 0x02u);
    core_machine_port_write(port, 0x00a1u, 0x01u);
}

static C_INT take_aux_byte(t_port *port, t_pic *master, t_pic *slave,
    type_unsigned_8 expected)
{
    core_machine_pic_refresh(master, slave);
    if ((read_port(port, 0x0064u) & (VKBC_STATUS_OBF | VKBC_STATUS_AUX)) !=
        (VKBC_STATUS_OBF | VKBC_STATUS_AUX) ||
        core_machine_pic_get_interrupt(master, slave) != 0x74u ||
        read_port(port, 0x0060u) != expected) return 0;
    core_machine_port_write(port, 0x00a0u, 0x20u);
    core_machine_port_write(port, 0x0020u, 0x20u);
    return 1;
}

static C_VOID send_aux_command(t_port *port, type_unsigned_8 command)
{
    core_machine_port_write(port, 0x0064u, 0xd4u);
    core_machine_port_write(port, 0x0060u, command);
}

static C_VOID send_aux_parameter(t_port *port, type_unsigned_8 value)
{
    core_machine_port_write(port, 0x0064u, 0xd4u);
    core_machine_port_write(port, 0x0060u, value);
}

C_INT main(C_VOID)
{
    t_kbc kbc;
    t_pic master;
    t_pic slave;
    t_ram memory = {0};
    core_machine_cpu_execution_context execution = {0};
    t_port port;
    C_INT failed = 0;
    C_INT stage = 1;
    type_unsigned_8 index;

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&master, &slave, &port, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_bind_core_services(&kbc, &master, &slave, &memory,
        &execution, TYPE_TRUE);
    initialize_pic(&port);

    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= (read_port(&port, 0x0064u) & VKBC_STATUS_AUX) != 0u;
    failed |= read_port(&port, 0x0060u) != 0x47u;
    core_machine_port_write(&port, 0x0064u, 0xa9u);
    failed |= (read_port(&port, 0x0064u) & VKBC_STATUS_AUX) != 0u;
    failed |= core_machine_pic_scan_interrupt(&master, &slave);
    failed |= read_port(&port, 0x0060u) != 0x00u;

    send_aux_command(&port, 0xf2u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    failed |= !take_aux_byte(&port, &master, &slave, 0x00u);
    stage = 2;
    send_aux_command(&port, 0xf4u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 5, -3, 0x01u) !=
        TYPE_STATUS_OK;
    failed |= !take_aux_byte(&port, &master, &slave, 0x29u);
    failed |= !take_aux_byte(&port, &master, &slave, 0x05u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfdu);

    core_machine_port_write(&port, 0x0064u, 0xa7u);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) !=
        TYPE_STATUS_INVALID_STATE;
    core_machine_pic_refresh(&master, &slave);
    failed |= core_machine_pic_scan_interrupt(&master, &slave);
    core_machine_port_write(&port, 0x0064u, 0xa8u);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) !=
        TYPE_STATUS_OK;
    failed |= !take_aux_byte(&port, &master, &slave, 0x09u);
    failed |= !take_aux_byte(&port, &master, &slave, 0x01u);
    failed |= !take_aux_byte(&port, &master, &slave, 0x01u);

    send_aux_command(&port, 0xf5u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0u) !=
        TYPE_STATUS_INVALID_STATE;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= (read_port(&port, 0x0064u) & VKBC_STATUS_AUX) != 0u;
    core_machine_pic_refresh(&master, &slave);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x09u;
    failed |= read_port(&port, 0x0060u) != 0x03u;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    send_aux_command(&port, 0xf4u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    send_aux_command(&port, 0xf3u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    send_aux_parameter(&port, 200u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    send_aux_command(&port, 0xf3u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    send_aux_parameter(&port, 15u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfeu);
    send_aux_command(&port, 0xe8u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    send_aux_parameter(&port, 3u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    send_aux_command(&port, 0xe8u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    send_aux_parameter(&port, 4u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfeu);

    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:CONFIG\n");
        return 1;
    }
    stage = 3;
    send_aux_command(&port, 0xe9u);
    failed |= core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) !=
        TYPE_STATUS_OK;
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    failed |= !take_aux_byte(&port, &master, &slave, 0x21u);
    failed |= !take_aux_byte(&port, &master, &slave, 0x03u);
    failed |= !take_aux_byte(&port, &master, &slave, 200u);
    failed |= !take_aux_byte(&port, &master, &slave, 0x09u);
    failed |= !take_aux_byte(&port, &master, &slave, 0x01u);
    failed |= !take_aux_byte(&port, &master, &slave, 0x01u);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:E9-ORDER\n");
        return 1;
    }
    stage = 4;
    send_aux_command(&port, 0xe9u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau);
    failed |= !take_aux_byte(&port, &master, &slave, 0x21u);
    failed |= !take_aux_byte(&port, &master, &slave, 0x03u);
    failed |= !take_aux_byte(&port, &master, &slave, 200u);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:E9-BUTTONS\n");
        return 1;
    }
    stage = 5;
    send_aux_command(&port, 0xf6u);
    if (!take_aux_byte(&port, &master, &slave, 0xfau)) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:F6\n");
        failed = 1;
    }
    send_aux_command(&port, 0xe9u);
    if (!take_aux_byte(&port, &master, &slave, 0xfau)) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:E9-ACK\n");
        failed = 1;
    }
    if (!take_aux_byte(&port, &master, &slave, 0x00u)) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:E9-STATUS\n");
        failed = 1;
    }
    if (!take_aux_byte(&port, &master, &slave, 0x02u)) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:E9-RESOLUTION\n");
        failed = 1;
    }
    if (!take_aux_byte(&port, &master, &slave, 100u)) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:E9-RATE\n");
        failed = 1;
    }

    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x05u);
    send_aux_command(&port, 0xf2u);
    failed |= (read_port(&port, 0x0064u) & (VKBC_STATUS_OBF | VKBC_STATUS_AUX)) !=
        (VKBC_STATUS_OBF | VKBC_STATUS_AUX) ||
        core_machine_pic_scan_interrupt(&master, &slave) ||
        read_port(&port, 0x0060u) != 0xfau ||
        read_port(&port, 0x0060u) != 0x00u;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x07u);

    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    send_aux_command(&port, 0xf2u);
    failed |= (read_port(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (read_port(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau) ||
        !take_aux_byte(&port, &master, &slave, 0x00u);
    core_machine_kbc_set_command_response_timing(&kbc, 0u);

    send_aux_command(&port, 0xf4u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau) ||
        core_machine_kbc_submit_aux_report(&kbc, 0, 0, 0u) != TYPE_STATUS_OK ||
        (read_port(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        core_machine_kbc_submit_aux_report(&kbc, 300, -300, 0u) != TYPE_STATUS_OK ||
        !take_aux_byte(&port, &master, &slave, 0xe8u) ||
        !take_aux_byte(&port, &master, &slave, 0xffu) ||
        !take_aux_byte(&port, &master, &slave, 0x00u);

    send_aux_command(&port, 0xefu);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfeu);
    send_aux_command(&port, 0xffu);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau) ||
        !take_aux_byte(&port, &master, &slave, 0xaau) ||
        !take_aux_byte(&port, &master, &slave, 0x00u) ||
        kbc.data.aux_reporting_enabled || kbc.data.aux_resolution != 2u ||
        kbc.data.aux_sample_rate != 100u;

    send_aux_command(&port, 0xf4u);
    failed |= !take_aux_byte(&port, &master, &slave, 0xfau) ||
        core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0u) != TYPE_STATUS_OK ||
        !kbc.data.irq12_asserted;
    for (index = 0u; index < CORE_MACHINE_KBC_FIFO_CAPACITY - 3u; ++index) {
        failed |= core_machine_kbc_submit_native_byte(&kbc, index) != TYPE_STATUS_OK;
    }
    failed |= core_machine_kbc_submit_aux_report(&kbc, 2, 2, 1u) !=
        TYPE_STATUS_INVALID_STATE || kbc.data.fifo_count !=
            CORE_MACHINE_KBC_FIFO_CAPACITY || kbc.data.aux_button_state != 0u;
    core_machine_kbc_finalize(&kbc);
    failed |= kbc.data.irq12_asserted;

    core_machine_pic_finalize(&master, &slave);
    core_machine_port_finalize(&port);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:STAGE=%d\n", stage);
        return 1;
    }
    STD_PRINTF("M5:T267:S1:AUX:PORT:OK\n");
    return 0;
}
