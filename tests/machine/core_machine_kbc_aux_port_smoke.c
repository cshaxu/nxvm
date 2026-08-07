#include "type.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/kbc.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"

static uint8_t read_port(t_port *port, uint16_t id)
{
    return (uint8_t)core_machine_port_read(port, id);
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
    uint8_t expected)
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

static C_VOID send_aux_command(t_port *port, uint8_t command)
{
    core_machine_port_write(port, 0x0064u, 0xd4u);
    core_machine_port_write(port, 0x0060u, command);
}

static C_VOID send_aux_parameter(t_port *port, uint8_t value)
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

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&master, &slave, &port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_bind_core_services(&kbc, &master, &slave, &memory,
        &execution);
    initialize_pic(&port);

    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= (read_port(&port, 0x0064u) & VKBC_STATUS_AUX) != 0u;
    failed |= read_port(&port, 0x0060u) != 0x07u;
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
    failed |= core_machine_kbc_submit_scan_code(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= (read_port(&port, 0x0064u) & VKBC_STATUS_AUX) != 0u;
    core_machine_pic_refresh(&master, &slave);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x09u;
    failed |= read_port(&port, 0x0060u) != 0x1eu;
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

    core_machine_kbc_finalize(&kbc);
    core_machine_pic_finalize(&master, &slave);
    core_machine_port_finalize(&port);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T267:AUX:PORT:FAIL:STAGE=%d\n", stage);
        return 1;
    }
    STD_PRINTF("M5:T267:S1:AUX:PORT:OK\n");
    return 0;
}
