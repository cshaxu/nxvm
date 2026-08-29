#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/port.h"

static C_VOID initialize_pic(t_pic *master, t_pic *slave, t_port *port,
    type_unsigned_8 icw1)
{
    core_machine_port_write(port, 0x0020u, icw1);
    core_machine_port_write(port, 0x0021u, 0x08u);
    core_machine_port_write(port, 0x0021u, 0x04u);
    core_machine_port_write(port, 0x0021u, 0x01u);
    core_machine_port_write(port, 0x00a0u, icw1);
    core_machine_port_write(port, 0x00a1u, 0x70u);
    core_machine_port_write(port, 0x00a1u, 0x02u);
    core_machine_port_write(port, 0x00a1u, 0x01u);
    (C_VOID)master;
    (C_VOID)slave;
}

C_INT main(C_VOID)
{
    t_pic master;
    t_pic slave;
    t_port port;
    core_machine_pic_irq_source irq1;
    core_machine_pic_irq_source irq6;
    core_machine_pic_irq_source irq14;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&master, &slave, &port,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_pic_reset(&master, &slave);
    initialize_pic(&master, &slave, &port, 0x11u);
    core_machine_pic_irq_source_bind(&irq1, &master, &slave, 1u);
    core_machine_pic_irq_source_bind(&irq6, &master, &slave, 6u);
    core_machine_pic_irq_source_bind(&irq14, &master, &slave, 14u);

    core_machine_pic_irq_source_assert(&irq6);
    core_machine_pic_irq_source_deassert(&irq6);
    failed |= !core_machine_pic_scan_interrupt(&master, &slave);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x0eu;
    failed |= (master.data.isr & VPIC_ISR_IRQ(6u)) == 0u;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    core_machine_pic_irq_source_assert(&irq6);
    core_machine_pic_irq_source_deassert(&irq6);
    core_machine_pic_irq_source_assert(&irq1);
    core_machine_pic_irq_source_deassert(&irq1);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x09u;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x0eu;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    core_machine_port_write(&port, 0x0021u, VPIC_OCW1_IMR(6u));
    core_machine_pic_irq_source_assert(&irq6);
    core_machine_pic_irq_source_deassert(&irq6);
    failed |= core_machine_pic_scan_interrupt(&master, &slave);
    core_machine_port_write(&port, 0x0021u, 0u);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x0eu;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    core_machine_pic_irq_source_assert(&irq14);
    core_machine_pic_irq_source_deassert(&irq14);
    core_machine_pic_refresh(&master, &slave);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x76u;
    failed |= (master.data.isr & VPIC_ISR_IRQ(2u)) == 0u ||
        (slave.data.isr & VPIC_ISR_IRQ(6u)) == 0u;
    core_machine_port_write(&port, 0x00a0u, 0x20u);
    core_machine_port_write(&port, 0x0020u, 0x20u);

    core_machine_pic_reset(&master, &slave);
    initialize_pic(&master, &slave, &port, 0x19u);
    core_machine_pic_irq_source_bind(&irq1, &master, &slave, 1u);
    core_machine_pic_irq_source_assert(&irq1);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x09u;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    core_machine_pic_refresh(&master, &slave);
    failed |= !core_machine_pic_scan_interrupt(&master, &slave);
    core_machine_pic_irq_source_deassert(&irq1);
    core_machine_pic_refresh(&master, &slave);
    failed |= core_machine_pic_scan_interrupt(&master, &slave);

    core_machine_pic_reset(&master, &slave);
    initialize_pic(&master, &slave, &port, 0x19u);
    core_machine_pic_irq_source_bind(&irq14, &master, &slave, 14u);
    core_machine_pic_irq_source_assert(&irq14);
    core_machine_pic_refresh(&master, &slave);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x76u;
    failed |= (master.data.isr & VPIC_ISR_IRQ(2u)) == 0u ||
        (slave.data.isr & VPIC_ISR_IRQ(6u)) == 0u;
    core_machine_port_write(&port, 0x00a0u, 0x20u);
    core_machine_port_write(&port, 0x0020u, 0x20u);
    core_machine_pic_refresh(&master, &slave);
    failed |= !core_machine_pic_scan_interrupt(&master, &slave);
    failed |= core_machine_pic_get_interrupt(&master, &slave) != 0x76u;
    core_machine_port_write(&port, 0x00a0u, 0x20u);
    core_machine_port_write(&port, 0x0020u, 0x20u);
    core_machine_pic_irq_source_deassert(&irq14);
    core_machine_pic_refresh(&master, &slave);
    failed |= core_machine_pic_scan_interrupt(&master, &slave);

    core_machine_pic_finalize(&master, &slave);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T216:S1:PIC-IRQ-LIFECYCLE:OK\n");
    return 0;
}
