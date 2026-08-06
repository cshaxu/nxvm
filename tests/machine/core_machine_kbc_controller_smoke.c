#include "type.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/kbc.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"

static uint8_t core_machine_kbc_read_byte(t_port *port, uint16_t port_id)
{
    return (uint8_t)core_machine_port_read(port, port_id);
}

static C_VOID core_machine_kbc_initialize_pic(t_port *port)
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

C_INT main(C_VOID)
{
    t_kbc kbc;
    t_pic pic_master;
    t_pic pic_slave;
    t_ram memory = {0};
    core_machine_cpu_execution_context execution = {0};
    t_port port;
    C_INT failed = 0;
    uint8_t index;

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&pic_master, &pic_slave, &port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_bind_core_services(&kbc, &pic_master, &pic_slave,
        &memory, &execution);
    core_machine_kbc_initialize_pic(&port);

    failed |= core_machine_kbc_read_byte(&port, 0x0064u) != 0x14u;
    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) == 0u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x07u;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x01u);
    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x01u;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x41u);
    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x41u ||
        (kbc.data.command_byte & CORE_MACHINE_KBC_COMMAND_TRANSLATION) == 0u;

    core_machine_port_write(&port, 0x0064u, 0xadu);
    failed |= core_machine_kbc_submit_scan_code(&kbc, 0x1eu) !=
        TYPE_STATUS_INVALID_STATE;
    core_machine_port_write(&port, 0x0064u, 0xaeu);
    failed |= core_machine_kbc_submit_scan_code(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= (pic_master.data.irr & VPIC_IRR_IRQ(1u)) == 0u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    failed |= core_machine_kbc_submit_scan_code(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= core_machine_kbc_submit_scan_code(&kbc, 0x30u) != TYPE_STATUS_OK;
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x30u;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    core_machine_port_write(&port, 0x0060u, 0xf2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xabu;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x83u;
    core_machine_port_write(&port, 0x0064u, 0xaau);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x55u;
    core_machine_port_write(&port, 0x0064u, 0xabu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0u;

    core_machine_port_write(&port, 0x0060u, 0xf0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x00u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) !=
        CORE_MACHINE_KBC_SCAN_SET_1;
    core_machine_port_write(&port, 0x0060u, 0xf0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x02u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfeu ||
        kbc.data.scan_set != CORE_MACHINE_KBC_SCAN_SET_1;

    core_machine_port_write(&port, 0x0060u, 0xedu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x07u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.led_state != 0x07u;
    core_machine_port_write(&port, 0x0060u, 0xf3u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x1fu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.typematic != 0x1fu;

    core_machine_kbc_set_typematic_timing(&kbc, 0u, 0u);
    failed |= core_machine_kbc_submit_scan_code(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 1000000u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;

    core_machine_kbc_set_typematic_timing(&kbc, 3u, 2u);
    failed |= core_machine_kbc_submit_scan_code(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    failed |= core_machine_kbc_submit_scan_code(&kbc, 0x9eu) != TYPE_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x9eu;
    core_machine_kbc_advance(&kbc, 8u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;

    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    core_machine_port_write(&port, 0x0060u, 0xeeu);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xeeu;
    core_machine_kbc_set_command_response_timing(&kbc, 0u);

    core_machine_port_write(&port, 0x0060u, 0xf5u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_submit_scan_code(&kbc, 0x1eu) !=
            TYPE_STATUS_INVALID_STATE;
    core_machine_port_write(&port, 0x0060u, 0xf4u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_submit_scan_code(&kbc, 0x1eu) != TYPE_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_port_write(&port, 0x0060u, 0xf6u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.led_state != 0u || kbc.data.typematic != 0x20u ||
        !kbc.data.scanning_enabled;
    core_machine_port_write(&port, 0x0060u, 0xfeu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0xffu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau ||
        kbc.data.scan_set != CORE_MACHINE_KBC_SCAN_SET_1;
    core_machine_port_write(&port, 0x0060u, 0x00u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfeu;

    core_machine_port_write(&port, 0x0064u, 0xd1u);
    core_machine_port_write(&port, 0x0060u, 0x03u);
    failed |= !memory.data.flagA20;
    core_machine_port_write(&port, 0x0064u, 0xd0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x03u;
    core_machine_port_write(&port, 0x0064u, 0xd1u);
    core_machine_port_write(&port, 0x0060u, 0x00u);
    failed |= memory.data.flagA20 ||
        !core_machine_cpu_execution_consume_reset_request(&execution);

    for (index = 0u; index < CORE_MACHINE_KBC_FIFO_CAPACITY; ++index) {
        failed |= core_machine_kbc_submit_scan_code(&kbc, index) != TYPE_STATUS_OK;
    }
    failed |= core_machine_kbc_submit_scan_code(&kbc, 0xffu) !=
        TYPE_STATUS_INVALID_STATE;
    /* A command reply behind rapid typeahead remains KBC-owned until the
     * guest drains FIFO space; it is never lost merely because output is full. */
    core_machine_port_write(&port, 0x0060u, 0xf2u);
    for (index = 0u; index < CORE_MACHINE_KBC_FIFO_CAPACITY; ++index) {
        failed |= core_machine_kbc_read_byte(&port, 0x0060u) != index;
        core_machine_kbc_advance(&kbc, 0u);
    }
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xabu;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x83u;

    core_machine_kbc_finalize(&kbc);
    core_machine_pic_finalize(&pic_master, &pic_slave);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T227:S3:KBC-CONTROLLER:OK\n");
    return 0;
}
