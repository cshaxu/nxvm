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
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x05u;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x01u);
    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x01u;

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
    for (index = 0u; index < CORE_MACHINE_KBC_FIFO_CAPACITY; ++index) {
        failed |= core_machine_kbc_read_byte(&port, 0x0060u) != index;
    }

    core_machine_kbc_finalize(&kbc);
    core_machine_pic_finalize(&pic_master, &pic_slave);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T216:S5:KBC-CONTROLLER:OK\n");
    return 0;
}
