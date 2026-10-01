#include "support/pic_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/pic_bus.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/port.h"

typedef struct pic_command_priority_fixture {
    core_machine_pic_bus master;
    core_machine_pic_bus slave;
    core_machine machine;
} pic_command_priority_fixture;

static void pic_command_priority_program(t_port *port,
    lib_u8 master_icw1, lib_u8 master_icw3,
    lib_u8 master_icw4, lib_u8 slave_icw1,
    lib_u8 slave_icw3, lib_u8 slave_icw4)
{
    core_machine_port_write(port, 0x0020u, master_icw1);
    core_machine_port_write(port, 0x0021u, 0x08u);
    if (!CORE_MACHINE_BIT_IS_SET(master_icw1, VPIC_ICW1_SNGL)) {
        core_machine_port_write(port, 0x0021u, master_icw3);
    }
    if (CORE_MACHINE_BIT_IS_SET(master_icw1, VPIC_ICW1_IC4)) {
        core_machine_port_write(port, 0x0021u, master_icw4);
    }
    core_machine_port_write(port, 0x00a0u, slave_icw1);
    core_machine_port_write(port, 0x00a1u, 0x70u);
    if (!CORE_MACHINE_BIT_IS_SET(slave_icw1, VPIC_ICW1_SNGL)) {
        core_machine_port_write(port, 0x00a1u, slave_icw3);
    }
    if (CORE_MACHINE_BIT_IS_SET(slave_icw1, VPIC_ICW1_IC4)) {
        core_machine_port_write(port, 0x00a1u, slave_icw4);
    }
}

static void pic_command_priority_initialize(pic_command_priority_fixture *fixture,
    lib_u8 master_icw4, lib_u8 slave_icw4)
{
    lib_memory_set(&fixture->machine, 0, sizeof(fixture->machine));
    fixture->machine.lifecycle = CORE_MACHINE_INITIALIZED;
    core_machine_port_initialize(&fixture->machine.executor_port);
    core_machine_pic_initialize(&fixture->master, &fixture->slave, &fixture->machine,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_pic_reset(&fixture->master, &fixture->slave);
    pic_command_priority_program(&fixture->machine.executor_port, 0x11u, 0x04u, master_icw4,
        0x11u, 0x02u, slave_icw4);
}

static void pic_command_priority_finalize(pic_command_priority_fixture *fixture)
{
    core_machine_pic_finalize(&fixture->master, &fixture->slave);
    core_machine_port_finalize(&fixture->machine.executor_port);
}

static void pic_command_priority_raise(pic_command_priority_fixture *fixture,
    core_machine_pic_irq_source *source, lib_u8 irq)
{
    core_machine_pic_irq_source_bind(source, &fixture->master, &fixture->slave, irq);
    core_machine_pic_irq_source_assert(source);
    core_machine_pic_irq_source_deassert(source);
}

static lib_i32 pic_command_priority_test_cascade_selection(void)
{
    pic_command_priority_fixture fixture;
    core_machine_pic_irq_source irq3;
    core_machine_pic_irq_source irq14;
    core_machine_pic_irq_source irq15;
    lib_i32 failed = 0;

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u;
    core_machine_port_write(&fixture.machine.executor_port, 0x0020u, 0x20u);
    pic_command_priority_raise(&fixture, &irq15, 15u);
    pic_command_priority_raise(&fixture, &irq3, 3u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave) ||
        core_machine_pic_peek_interrupt(&fixture.master, &fixture.slave) != 0x0bu ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0bu;
    core_machine_port_write(&fixture.machine.executor_port, 0x0020u, 0x20u);
    core_machine_port_write(&fixture.machine.executor_port, 0x00a0u, 0x66u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x77u;
    core_machine_port_write(&fixture.machine.executor_port, 0x00a0u, 0x20u);
    core_machine_port_write(&fixture.machine.executor_port, 0x0020u, 0x20u);

    pic_command_priority_finalize(&fixture);
    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_port_write(&fixture.machine.executor_port, 0x00a1u, VPIC_OCW1_IMR(6u));
    pic_command_priority_raise(&fixture, &irq3, 3u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0bu ||
        CORE_MACHINE_BIT_IS_SET(test_pic_read(&fixture.master, 0x0bu), VPIC_ISR_IRQ(2u));
    core_machine_port_write(&fixture.machine.executor_port, 0x0020u, 0x20u);
    pic_command_priority_finalize(&fixture);
    return failed;
}

static lib_i32 pic_command_priority_test_programmed_cascade(void)
{
    pic_command_priority_fixture fixture;
    core_machine_pic_irq_source irq14;
    lib_i32 failed = 0;

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= test_pic_read(&fixture.master, 0x0au) != VPIC_IRR_IRQ(2u);
    core_machine_port_write(&fixture.machine.executor_port, 0x0020u, 0x13u);
    failed |= test_pic_read(&fixture.master, 0x0au) != 0u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0u;
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u ||
        !CORE_MACHINE_BIT_IS_SET(test_pic_read(&fixture.master, 0x0bu), VPIC_ISR_IRQ(2u)) ||
        !CORE_MACHINE_BIT_IS_SET(test_pic_read(&fixture.slave, 0x0bu), VPIC_ISR_IRQ(6u));
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_program(&fixture.machine.executor_port, 0x11u, VPIC_ICW3_S(5u), 0x01u,
        0x11u, 5u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= test_pic_read(&fixture.master, 0x0au) != VPIC_IRR_IRQ(5u) ||
        core_machine_pic_peek_interrupt(&fixture.master, &fixture.slave) != 0x76u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u ||
        !CORE_MACHINE_BIT_IS_SET(test_pic_read(&fixture.master, 0x0bu), VPIC_ISR_IRQ(5u)) ||
        !CORE_MACHINE_BIT_IS_SET(test_pic_read(&fixture.slave, 0x0bu), VPIC_ISR_IRQ(6u));
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_program(&fixture.machine.executor_port, 0x11u, VPIC_ICW3_S(4u), 0x01u,
        0x11u, 5u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= test_pic_read(&fixture.master, 0x0au) != 0u || core_machine_pic_scan_interrupt(
        &fixture.master, &fixture.slave);
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_program(&fixture.machine.executor_port, 0x13u, 0u, 0x01u,
        0x11u, 2u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= test_pic_read(&fixture.master, 0x0au) != 0u || core_machine_pic_scan_interrupt(
        &fixture.master, &fixture.slave);
    core_machine_port_write(&fixture.machine.executor_port, 0x0020u, 0x13u);
    failed |= test_pic_read(&fixture.master, 0x0au) != 0u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0u;
    pic_command_priority_finalize(&fixture);
    return failed;
}

static lib_i32 pic_command_priority_test_immediate_cascade(void)
{
    pic_command_priority_fixture fixture;
    core_machine_pic_irq_source irq14;
    lib_i32 failed = 0;

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_raise(&fixture, &irq14, 14u);
    failed |= test_pic_read(&fixture.master, 0x0au) != VPIC_IRR_IRQ(2u) ||
        !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_port_write(&fixture.machine.executor_port, 0x00a1u, VPIC_OCW1_IMR(6u));
    failed |= test_pic_read(&fixture.master, 0x0au) != 0u ||
        core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_port_write(&fixture.machine.executor_port, 0x00a1u, 0u);
    failed |= test_pic_read(&fixture.master, 0x0au) != VPIC_IRR_IRQ(2u) ||
        !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_pic_reset(&fixture.master, &fixture.slave);
    failed |= test_pic_read(&fixture.master, 0x0au) != 0u ||
        test_pic_read(&fixture.slave, 0x0au) != 0u;
    pic_command_priority_finalize(&fixture);

    pic_command_priority_initialize(&fixture, 0x01u, 0x01u);
    pic_command_priority_program(&fixture.machine.executor_port, 0x19u, 0x04u, 0x01u,
        0x19u, 0x02u, 0x01u);
    core_machine_pic_irq_source_bind(&irq14, &fixture.master, &fixture.slave, 14u);
    core_machine_pic_irq_source_assert(&irq14);
    failed |= test_pic_read(&fixture.master, 0x0au) != VPIC_IRR_IRQ(2u);
    core_machine_pic_irq_source_deassert(&irq14);
    failed |= test_pic_read(&fixture.master, 0x0au) != 0u ||
        core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    pic_command_priority_finalize(&fixture);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= pic_command_priority_test_cascade_selection();
    failed |= pic_command_priority_test_programmed_cascade();
    failed |= pic_command_priority_test_immediate_cascade();
    if (failed != 0) return 1;
    printf("M5:T349:S2:PIC-COMMAND-PRIORITY:OK\n");
    return 0;
}
