#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/port.h"

typedef struct pic_command_priority_fixture {
    t_pic master;
    t_pic slave;
    t_port port;
} pic_command_priority_fixture;

static C_VOID pic_command_priority_program(t_port *port,
    type_unsigned_8 master_icw1, type_unsigned_8 master_icw3,
    type_unsigned_8 master_icw4, type_unsigned_8 slave_icw1,
    type_unsigned_8 slave_icw3, type_unsigned_8 slave_icw4)
{
    core_machine_port_write(port, 0x0020u, master_icw1);
    core_machine_port_write(port, 0x0021u, 0x08u);
    if (!TYPE_GET_BIT(master_icw1, VPIC_ICW1_SNGL)) {
        core_machine_port_write(port, 0x0021u, master_icw3);
    }
    if (TYPE_GET_BIT(master_icw1, VPIC_ICW1_IC4)) {
        core_machine_port_write(port, 0x0021u, master_icw4);
    }
    core_machine_port_write(port, 0x00a0u, slave_icw1);
    core_machine_port_write(port, 0x00a1u, 0x70u);
    if (!TYPE_GET_BIT(slave_icw1, VPIC_ICW1_SNGL)) {
        core_machine_port_write(port, 0x00a1u, slave_icw3);
    }
    if (TYPE_GET_BIT(slave_icw1, VPIC_ICW1_IC4)) {
        core_machine_port_write(port, 0x00a1u, slave_icw4);
    }
}

static C_VOID pic_command_priority_initialize(pic_command_priority_fixture *fixture,
    type_unsigned_8 master_icw4, type_unsigned_8 slave_icw4)
{
    core_machine_port_initialize(&fixture->port);
    core_machine_pic_initialize(&fixture->master, &fixture->slave, &fixture->port,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_pic_reset(&fixture->master, &fixture->slave);
    pic_command_priority_program(&fixture->port, 0x11u, 0x04u, master_icw4,
        0x11u, 0x02u, slave_icw4);
}

static C_VOID pic_command_priority_finalize(pic_command_priority_fixture *fixture)
{
    core_machine_pic_finalize(&fixture->master, &fixture->slave);
    core_machine_port_finalize(&fixture->port);
}

static C_VOID pic_command_priority_raise(pic_command_priority_fixture *fixture,
    core_machine_pic_irq_source *source, type_unsigned_8 irq)
{
    core_machine_pic_irq_source_bind(source, &fixture->master, &fixture->slave, irq);
    core_machine_pic_irq_source_assert(source);
    core_machine_pic_irq_source_deassert(source);
}

static C_INT pic_command_priority_test_initialization_and_registers(C_VOID)
{
    pic_command_priority_fixture fixture;
    core_machine_pic_irq_source irq5;
    C_INT failed = 0;

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq5, 5u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x0au);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) != VPIC_IRR_IRQ(5u);
    failed |= !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0du;
    core_machine_port_write(&fixture.port, 0x0020u, 0x0bu);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) != VPIC_ISR_IRQ(5u);
    core_machine_port_write(&fixture.port, 0x0021u, VPIC_OCW1_IMR(1u));
    core_machine_port_write(&fixture.port, 0x0020u, 0x11u);
    failed |= fixture.master.data.irr != 0u || fixture.master.data.imr != 0u ||
        fixture.master.data.isr != 0u || fixture.master.data.irx != 0u ||
        fixture.master.data.icw2 != 0u || fixture.master.data.icw3 != 0u ||
        fixture.master.data.icw4 != 0u || fixture.master.data.imr != 0u ||
        fixture.master.data.ocw2 != 0u ||
        fixture.master.data.ocw3 != VPIC_OCW3_RR ||
        fixture.master.data.status != ICW2;
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0u ||
        fixture.master.data.isr != 0u;
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    failed |= core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave) ||
        core_machine_pic_peek_interrupt(&fixture.master, &fixture.slave) != 0u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0fu ||
        fixture.master.data.irr != 0u || fixture.master.data.isr != 0u;
    pic_command_priority_finalize(&fixture);
    return failed;
}

static C_INT pic_command_priority_test_eoi_and_rotation(C_VOID)
{
    pic_command_priority_fixture fixture;
    core_machine_pic_irq_source irq0;
    core_machine_pic_irq_source irq3;
    core_machine_pic_irq_source irq5;
    C_INT failed = 0;

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq3, 3u);
    pic_command_priority_raise(&fixture, &irq5, 5u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0bu;
    failed |= core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_port_write(&fixture.port, 0x0020u, 0x63u);
    failed |= fixture.master.data.isr != 0u;
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0du;
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);

    pic_command_priority_finalize(&fixture);
    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq0, 0u);
    pic_command_priority_raise(&fixture, &irq3, 3u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x08u;
    core_machine_port_write(&fixture.port, 0x0020u, 0xa0u);
    failed |= fixture.master.data.irx != 1u || fixture.master.data.isr != 0u;
    pic_command_priority_raise(&fixture, &irq0, 0u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0bu;
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x08u;
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq5, 5u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0du;
    pic_command_priority_raise(&fixture, &irq3, 3u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0bu;
    core_machine_port_write(&fixture.port, 0x0020u, 0xe5u);
    failed |= !TYPE_GET_BIT(fixture.master.data.isr, VPIC_ISR_IRQ(3u)) ||
        TYPE_GET_BIT(fixture.master.data.isr, VPIC_ISR_IRQ(5u)) ||
        fixture.master.data.irx != 6u;
    core_machine_port_write(&fixture.port, 0x0020u, 0x63u);
    pic_command_priority_finalize(&fixture);
    return failed;
}

static C_INT pic_command_priority_test_aeoi(C_VOID)
{
    pic_command_priority_fixture fixture;
    core_machine_pic_irq_source irq1;
    core_machine_pic_irq_source irq4;
    C_INT failed = 0;

    pic_command_priority_initialize(&fixture, 0x03u, 0x01u);
    pic_command_priority_raise(&fixture, &irq1, 1u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x09u ||
        fixture.master.data.isr != 0u;
    pic_command_priority_raise(&fixture, &irq4, 4u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0cu ||
        fixture.master.data.isr != 0u;
    pic_command_priority_finalize(&fixture);
    return failed;
}

static C_INT pic_command_priority_test_cascade_selection(C_VOID)
{
    pic_command_priority_fixture fixture;
    core_machine_pic_irq_source irq3;
    core_machine_pic_irq_source irq14;
    core_machine_pic_irq_source irq15;
    C_INT failed = 0;

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u;
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    pic_command_priority_raise(&fixture, &irq15, 15u);
    pic_command_priority_raise(&fixture, &irq3, 3u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave) ||
        core_machine_pic_peek_interrupt(&fixture.master, &fixture.slave) != 0x0bu ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0bu;
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    core_machine_port_write(&fixture.port, 0x00a0u, 0x66u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x77u;
    core_machine_port_write(&fixture.port, 0x00a0u, 0x20u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);

    pic_command_priority_finalize(&fixture);
    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_port_write(&fixture.port, 0x00a1u, VPIC_OCW1_IMR(6u));
    pic_command_priority_raise(&fixture, &irq3, 3u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0bu ||
        TYPE_GET_BIT(fixture.master.data.isr, VPIC_ISR_IRQ(2u));
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    pic_command_priority_finalize(&fixture);
    return failed;
}

static C_INT pic_command_priority_test_programmed_cascade(C_VOID)
{
    pic_command_priority_fixture fixture;
    core_machine_pic_irq_source irq14;
    C_INT failed = 0;

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= fixture.master.data.cascade_irr != VPIC_IRR_IRQ(2u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x13u);
    failed |= fixture.master.data.cascade_irr != 0u || fixture.master.data.status !=
        ICW2;
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u ||
        !TYPE_GET_BIT(fixture.master.data.isr, VPIC_ISR_IRQ(2u)) ||
        !TYPE_GET_BIT(fixture.slave.data.isr, VPIC_ISR_IRQ(6u));
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_program(&fixture.port, 0x11u, VPIC_ICW3_S(5u), 0x01u,
        0x11u, 5u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= fixture.master.data.cascade_irr != VPIC_IRR_IRQ(5u) ||
        core_machine_pic_peek_interrupt(&fixture.master, &fixture.slave) != 0x76u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u ||
        !TYPE_GET_BIT(fixture.master.data.isr, VPIC_ISR_IRQ(5u)) ||
        !TYPE_GET_BIT(fixture.slave.data.isr, VPIC_ISR_IRQ(6u));
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_program(&fixture.port, 0x11u, VPIC_ICW3_S(4u), 0x01u,
        0x11u, 5u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= fixture.master.data.cascade_irr != 0u || core_machine_pic_scan_interrupt(
        &fixture.master, &fixture.slave);
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_program(&fixture.port, 0x13u, 0u, 0x01u,
        0x11u, 2u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= fixture.master.data.cascade_irr != 0u || core_machine_pic_scan_interrupt(
        &fixture.master, &fixture.slave);
    core_machine_port_write(&fixture.port, 0x0020u, 0x13u);
    failed |= fixture.master.data.cascade_irr != 0u || fixture.master.data.status !=
        ICW2;
    pic_command_priority_finalize(&fixture);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failed = 0;

    failed |= pic_command_priority_test_initialization_and_registers();
    failed |= pic_command_priority_test_eoi_and_rotation();
    failed |= pic_command_priority_test_aeoi();
    failed |= pic_command_priority_test_cascade_selection();
    failed |= pic_command_priority_test_programmed_cascade();
    if (failed != 0) return 1;
    STD_PRINTF("M5:T349:S2:PIC-COMMAND-PRIORITY:OK\n");
    return 0;
}
