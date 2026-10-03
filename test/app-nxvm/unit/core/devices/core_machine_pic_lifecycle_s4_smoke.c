#include "support/pic_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/pic_bus.h"
#include "x86/ibmpc-common/pit_bus_interface.h"
#include "x86/core/machine.h"
#include "x86/core/port.h"

typedef struct pic_lifecycle_fixture {
    core_machine_pic_bus master;
    core_machine_pic_bus slave;
    core_machine machine;
} pic_lifecycle_fixture;

static void pic_lifecycle_initialize(pic_lifecycle_fixture *fixture,
    lib_u8 icw1)
{
    lib_memory_set(&fixture->machine, 0, sizeof(fixture->machine));
    fixture->machine.lifecycle = CORE_MACHINE_INITIALIZED;
    core_machine_port_initialize(&fixture->machine.executor_port);
    core_machine_pic_initialize(&fixture->master, &fixture->slave, &fixture->machine,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_pic_reset(&fixture->master, &fixture->slave);
    core_machine_port_write(&fixture->machine.executor_port, 0x0020u, icw1);
    core_machine_port_write(&fixture->machine.executor_port, 0x0021u, 0x08u);
    core_machine_port_write(&fixture->machine.executor_port, 0x0021u, 0x04u);
    core_machine_port_write(&fixture->machine.executor_port, 0x0021u, 0x01u);
    core_machine_port_write(&fixture->machine.executor_port, 0x00a0u, icw1);
    core_machine_port_write(&fixture->machine.executor_port, 0x00a1u, 0x70u);
    core_machine_port_write(&fixture->machine.executor_port, 0x00a1u, 0x02u);
    core_machine_port_write(&fixture->machine.executor_port, 0x00a1u, 0x01u);
}

static void pic_lifecycle_finalize(pic_lifecycle_fixture *fixture)
{
    core_machine_pic_finalize(&fixture->master, &fixture->slave);
    core_machine_port_finalize(&fixture->machine.executor_port);
}

static void pic_lifecycle_eoi(pic_lifecycle_fixture *fixture,
    lib_u8 slave)
{
    if (slave) core_machine_port_write(&fixture->machine.executor_port, 0x00a0u, 0x20u);
    core_machine_port_write(&fixture->machine.executor_port, 0x0020u, 0x20u);
}

static lib_i32 pic_lifecycle_test_master_level(void)
{
    pic_lifecycle_fixture fixture;
    core_machine_pic_irq_source first;
    core_machine_pic_irq_source second;
    lib_i32 failed = 0;

    pic_lifecycle_initialize(&fixture, 0x19u);
    core_machine_pic_irq_source_bind(&first, &fixture.master, &fixture.slave, 5u);
    core_machine_pic_irq_source_bind(&second, &fixture.master, &fixture.slave, 5u);
    core_machine_pic_irq_source_assert(&first);
    core_machine_pic_irq_source_assert(&second);
    failed |= fixture.master.asserted[5u] != 2u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0du;
    pic_lifecycle_eoi(&fixture, LIB_FALSE);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_pic_irq_source_deassert(&first);
    failed |= fixture.master.asserted[5u] != 1u;
    (void)core_machine_pic_get_interrupt(&fixture.master, &fixture.slave);
    pic_lifecycle_eoi(&fixture, LIB_FALSE);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_pic_irq_source_deassert(&second);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    pic_lifecycle_finalize(&fixture);
    return failed;
}

static lib_i32 pic_lifecycle_test_slave_level(void)
{
    pic_lifecycle_fixture fixture;
    core_machine_pic_irq_source first;
    core_machine_pic_irq_source second;
    lib_i32 failed = 0;

    pic_lifecycle_initialize(&fixture, 0x19u);
    core_machine_pic_irq_source_bind(&first, &fixture.master, &fixture.slave, 14u);
    core_machine_pic_irq_source_bind(&second, &fixture.master, &fixture.slave, 14u);
    core_machine_pic_irq_source_assert(&first);
    core_machine_pic_irq_source_assert(&second);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= fixture.slave.asserted[6u] != 2u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x76u;
    pic_lifecycle_eoi(&fixture, LIB_TRUE);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_pic_irq_source_deassert(&first);
    (void)core_machine_pic_get_interrupt(&fixture.master, &fixture.slave);
    pic_lifecycle_eoi(&fixture, LIB_TRUE);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= fixture.slave.asserted[6u] != 1u ||
        !core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_pic_irq_source_deassert(&second);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    pic_lifecycle_finalize(&fixture);
    return failed;
}

static lib_i32 pic_lifecycle_test_pit_reset(void)
{
    pic_lifecycle_fixture fixture;
    x86_pit *pit = LIB_NULL;
    core_machine_pic_irq_source irq0;
    lib_i32 failed = 0;

    pic_lifecycle_initialize(&fixture, 0x11u);
    core_machine_pic_irq_source_bind(&irq0, &fixture.master, &fixture.slave, 0u);
    if (x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK ||
        core_machine_pit_install_ports(&fixture.machine,
            pit, 0x0040u) != LIB_STATUS_OK) {
        x86_pit_destroy(pit);
        pic_lifecycle_finalize(&fixture);
        return 1;
    }
    x86_pit_reset(pit);
    x86_pit_set_output(pit, 0u, core_machine_pic_timer_output, &irq0);
    core_machine_port_write(&fixture.machine.executor_port, 0x0043u, 0x34u);
    core_machine_port_write(&fixture.machine.executor_port, 0x0040u, 3u);
    core_machine_port_write(&fixture.machine.executor_port, 0x0040u, 0u);
    /* The first clock transfers CR to CE; the IRQ0 edge is the fifth clock. */
    x86_pit_advance(pit, 5u);
    failed |= !irq0.asserted || fixture.master.asserted[0u] != 1u;
    core_machine_pic_reset(&fixture.master, &fixture.slave);
    x86_pit_reset(pit);
    failed |= irq0.asserted || fixture.master.asserted[0u] != 0u;
    core_machine_port_write(&fixture.machine.executor_port, 0x0043u, 0x34u);
    core_machine_port_write(&fixture.machine.executor_port, 0x0040u, 3u);
    core_machine_port_write(&fixture.machine.executor_port, 0x0040u, 0u);
    x86_pit_advance(pit, 5u);
    failed |= !irq0.asserted || fixture.master.asserted[0u] != 1u ||
        (test_pic_read(&fixture.master, 0x0au) & VPIC_IRR_IRQ(0u)) == 0u;
    x86_pit_destroy(pit);
    failed |= irq0.asserted || fixture.master.asserted[0u] != 0u;
    pic_lifecycle_finalize(&fixture);
    return failed;
}

static lib_i32 pic_lifecycle_test_edge_empty_and_bind(void)
{
    pic_lifecycle_fixture fixture;
    core_machine_pic_irq_source source;
    lib_i32 failed = 0;

    pic_lifecycle_initialize(&fixture, 0x11u);
    lib_memory_set(&source, 0u, sizeof(source));
    core_machine_pic_irq_source_bind(&source, &fixture.master, &fixture.slave, 2u);
    failed |= source.master != LIB_NULL || source.slave != LIB_NULL || source.asserted;
    core_machine_pic_irq_source_bind(&source, &fixture.master, &fixture.slave, 16u);
    failed |= source.master != LIB_NULL || source.slave != LIB_NULL || source.asserted;
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x0fu ||
        test_pic_read(&fixture.master, 0x0bu) != 0u || test_pic_read(&fixture.slave, 0x0bu) != 0u;
    core_machine_pic_irq_source_bind(&source, &fixture.master, &fixture.slave, 1u);
    core_machine_pic_irq_source_assert(&source);
    core_machine_pic_irq_source_deassert(&source);
    failed |= core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x09u;
    pic_lifecycle_eoi(&fixture, LIB_FALSE);
    failed |= core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    pic_lifecycle_finalize(&fixture);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= pic_lifecycle_test_master_level();
    failed |= pic_lifecycle_test_slave_level();
    failed |= pic_lifecycle_test_pit_reset();
    failed |= pic_lifecycle_test_edge_empty_and_bind();
    if (failed != 0) return 1;
    printf("M5:T349:S4:PIC-LIFECYCLE:OK\n");
    return 0;
}
