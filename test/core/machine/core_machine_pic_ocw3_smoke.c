#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/port.h"

typedef struct pic_ocw3_fixture {
    t_pic master;
    t_pic slave;
    t_port port;
} pic_ocw3_fixture;

static C_VOID pic_ocw3_initialize(pic_ocw3_fixture *fixture,
    type_unsigned_8 master_icw4)
{
    core_machine_port_initialize(&fixture->port);
    core_machine_pic_initialize(&fixture->master, &fixture->slave, &fixture->port,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_pic_reset(&fixture->master, &fixture->slave);
    core_machine_port_write(&fixture->port, 0x0020u, 0x11u);
    core_machine_port_write(&fixture->port, 0x0021u, 0x08u);
    core_machine_port_write(&fixture->port, 0x0021u, 0x04u);
    core_machine_port_write(&fixture->port, 0x0021u, master_icw4);
    core_machine_port_write(&fixture->port, 0x00a0u, 0x11u);
    core_machine_port_write(&fixture->port, 0x00a1u, 0x70u);
    core_machine_port_write(&fixture->port, 0x00a1u, 0x02u);
    core_machine_port_write(&fixture->port, 0x00a1u, 0x01u);
}

static C_VOID pic_ocw3_finalize(pic_ocw3_fixture *fixture)
{
    core_machine_pic_finalize(&fixture->master, &fixture->slave);
    core_machine_port_finalize(&fixture->port);
}

static C_VOID pic_ocw3_raise(pic_ocw3_fixture *fixture,
    core_machine_pic_irq_source *source, type_unsigned_8 irq)
{
    core_machine_pic_irq_source_bind(source, &fixture->master, &fixture->slave,
        irq);
    core_machine_pic_irq_source_assert(source);
    core_machine_pic_irq_source_deassert(source);
}

static C_INT pic_ocw3_test_read_and_poll(C_VOID)
{
    pic_ocw3_fixture fixture;
    core_machine_pic_irq_source irq4;
    core_machine_pic_irq_source irq5;
    core_machine_pic_irq_source irq14;
    C_INT failed = 0;

    pic_ocw3_initialize(&fixture, 0x01u);
    pic_ocw3_raise(&fixture, &irq4, 4u);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) != VPIC_IRR_IRQ(4u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x0cu);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) !=
        (VPIC_POLL_I | 4u) || fixture.master.data.irr != 0u ||
        fixture.master.data.isr != VPIC_ISR_IRQ(4u) ||
        TYPE_GET_BIT(fixture.master.data.ocw3, VPIC_OCW3_P);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) != 0u;
    core_machine_port_write(&fixture.port, 0x0020u, 0x0bu);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) != VPIC_ISR_IRQ(4u) ||
        core_machine_port_read(&fixture.port, 0x0020u) != VPIC_ISR_IRQ(4u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x0cu);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) != 0u;

    pic_ocw3_finalize(&fixture);
    pic_ocw3_initialize(&fixture, 0x03u);
    pic_ocw3_raise(&fixture, &irq5, 5u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x0cu);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) !=
        (VPIC_POLL_I | 5u) || fixture.master.data.isr != 0u;
    pic_ocw3_finalize(&fixture);

    pic_ocw3_initialize(&fixture, 0x01u);
    pic_ocw3_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    core_machine_port_write(&fixture.port, 0x0020u, 0x0cu);
    failed |= core_machine_port_read(&fixture.port, 0x0020u) !=
        (VPIC_POLL_I | 2u) || fixture.master.data.cascade_irr != 0u ||
        fixture.master.data.isr != VPIC_ISR_IRQ(2u);
    core_machine_port_write(&fixture.port, 0x00a0u, 0x0cu);
    failed |= core_machine_port_read(&fixture.port, 0x00a0u) !=
        (VPIC_POLL_I | 6u) || fixture.slave.data.isr != VPIC_ISR_IRQ(6u);
    pic_ocw3_finalize(&fixture);
    return failed;
}

static C_INT pic_ocw3_test_special_mask(C_VOID)
{
    pic_ocw3_fixture fixture;
    core_machine_pic_irq_source irq1;
    core_machine_pic_irq_source irq5;
    C_INT failed = 0;

    pic_ocw3_initialize(&fixture, 0x01u);
    pic_ocw3_raise(&fixture, &irq1, 1u);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x09u;
    pic_ocw3_raise(&fixture, &irq5, 5u);
    failed |= core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_port_write(&fixture.port, 0x0020u, 0x68u);
    core_machine_port_write(&fixture.port, 0x0021u, VPIC_OCW1_IMR(1u));
    failed |= fixture.master.data.isr != VPIC_ISR_IRQ(1u) ||
        !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave) ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0du;
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    failed |= fixture.master.data.isr != VPIC_ISR_IRQ(1u);
    core_machine_port_write(&fixture.port, 0x0021u, 0u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    failed |= fixture.master.data.isr != 0u;
    core_machine_port_write(&fixture.port, 0x0020u, 0x48u);
    failed |= TYPE_GET_BIT(fixture.master.data.ocw3, VPIC_OCW3_SMM);
    pic_ocw3_finalize(&fixture);
    return failed;
}

static C_INT pic_ocw3_test_sfnm(C_VOID)
{
    pic_ocw3_fixture fixture;
    core_machine_pic_irq_source irq8;
    core_machine_pic_irq_source irq14;
    C_INT failed = 0;

    pic_ocw3_initialize(&fixture, 0x01u);
    pic_ocw3_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u;
    pic_ocw3_raise(&fixture, &irq8, 8u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    pic_ocw3_finalize(&fixture);

    pic_ocw3_initialize(&fixture, 0x11u);
    pic_ocw3_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u;
    pic_ocw3_raise(&fixture, &irq8, 8u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave) ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x70u;
    pic_ocw3_finalize(&fixture);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failed = 0;

    failed |= pic_ocw3_test_read_and_poll();
    failed |= pic_ocw3_test_special_mask();
    failed |= pic_ocw3_test_sfnm();
    if (failed != 0) return 1;
    STD_PRINTF("M5:T349:S3:PIC-OCW3:OK\n");
    return 0;
}
