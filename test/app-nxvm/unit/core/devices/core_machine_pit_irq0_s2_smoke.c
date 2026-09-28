#include "support/pic_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/pic_bus.h"
#include "app-nxvm/devices/pit_bus.h"
#include "app-nxvm/devices/port.h"

typedef struct pit_irq0_fixture {
    core_machine_pic_bus master;
    core_machine_pic_bus slave;
    core_machine_pit_bus pit;
    t_port port;
    core_machine_pic_irq_source irq0;
} pit_irq0_fixture;

static lib_status pit_irq0_initialize(pit_irq0_fixture *fixture)
{
    core_machine_port_initialize(&fixture->port);
    core_machine_pic_initialize(&fixture->master, &fixture->slave, &fixture->port,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_pic_reset(&fixture->master, &fixture->slave);
    core_machine_port_write(&fixture->port, 0x0020u, 0x11u);
    core_machine_port_write(&fixture->port, 0x0021u, 0x08u);
    core_machine_port_write(&fixture->port, 0x0021u, 0x04u);
    core_machine_port_write(&fixture->port, 0x0021u, 0x01u);
    core_machine_port_write(&fixture->port, 0x00a0u, 0x11u);
    core_machine_port_write(&fixture->port, 0x00a1u, 0x70u);
    core_machine_port_write(&fixture->port, 0x00a1u, 0x02u);
    core_machine_port_write(&fixture->port, 0x00a1u, 0x01u);
    core_machine_pic_irq_source_bind(&fixture->irq0, &fixture->master,
        &fixture->slave, 0u);
    lib_status status = core_machine_pit_bus_create(&fixture->pit, &fixture->port,
        X86_PIT_PERSONALITY_8254, 0x0040u);
    if (status != LIB_STATUS_OK) {
        core_machine_pic_finalize(&fixture->master, &fixture->slave);
        core_machine_port_finalize(&fixture->port);
        return status;
    }
    x86_pit_reset(fixture->pit.device);
    x86_pit_set_output(fixture->pit.device, 0u,
        core_machine_pic_timer_output, &fixture->irq0);
    return LIB_STATUS_OK;
}

static void pit_irq0_finalize(pit_irq0_fixture *fixture)
{
    core_machine_pit_bus_destroy(&fixture->pit);
    core_machine_pic_finalize(&fixture->master, &fixture->slave);
    core_machine_port_finalize(&fixture->port);
}

static void pit_irq0_program(t_port *port, lib_u8 control,
    lib_u16 count)
{
    lib_u16 data_port = (lib_u16)(0x0040u +
        ((control >> 6u) & 0x03u));

    core_machine_port_write(port, 0x0043u, control);
    core_machine_port_write(port, data_port, count & 0xffu);
    core_machine_port_write(port, data_port, count >> 8u);
}

static lib_i32 pit_irq0_test_mode2_edge(void)
{
    pit_irq0_fixture fixture;
    lib_i32 failed = 0;

    if (pit_irq0_initialize(&fixture) != LIB_STATUS_OK) return 1;
    pit_irq0_program(&fixture.port, 0x34u, 3u);
    failed |= fixture.irq0.asserted ||
        !x86_pit_get_output(fixture.pit.device, 0u) ||
        core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    x86_pit_advance(fixture.pit.device, 4u);
    failed |= x86_pit_get_output(fixture.pit.device, 0u) ||
        fixture.irq0.asserted || core_machine_pic_scan_interrupt(&fixture.master,
            &fixture.slave);
    x86_pit_advance(fixture.pit.device, 1u);
    failed |= !x86_pit_get_output(fixture.pit.device, 0u) ||
        !fixture.irq0.asserted || fixture.master.asserted[0u] != 1u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x08u;
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    failed |= test_pic_read(&fixture.master, 0x0bu) != 0u ||
        core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    x86_pit_advance(fixture.pit.device, 2u);
    failed |= x86_pit_get_output(fixture.pit.device, 0u) || fixture.irq0.asserted ||
        fixture.master.asserted[0u] != 0u;
    pit_irq0_finalize(&fixture);
    return failed;
}

static lib_i32 pit_irq0_test_counter_forms(void)
{
    pit_irq0_fixture fixture;
    lib_u8 status;
    lib_i32 failed = 0;

    if (pit_irq0_initialize(&fixture) != LIB_STATUS_OK) return 1;
    core_machine_port_write(&fixture.port, 0x0043u, 0x70u);
    core_machine_port_write(&fixture.port, 0x0041u, 0x34u);
    core_machine_port_write(&fixture.port, 0x0043u, 0xeau);
    failed |= core_machine_port_read(&fixture.port, 0x0041u) != 0x70u;
    core_machine_port_write(&fixture.port, 0x0041u, 0x12u);
    core_machine_port_write(&fixture.port, 0x0043u, 0xeau);
    failed |= core_machine_port_read(&fixture.port, 0x0041u) != 0x70u;
    x86_pit_advance(fixture.pit.device, 1u);
    core_machine_port_write(&fixture.port, 0x0043u, 0xeau);
    failed |= core_machine_port_read(&fixture.port, 0x0041u) != 0x30u;
    core_machine_port_write(&fixture.port, 0x0043u, 0x00d8u);
    failed |= core_machine_port_read(&fixture.port, 0x0041u) != 0x34u ||
        core_machine_port_read(&fixture.port, 0x0041u) != 0x12u;
    pit_irq0_program(&fixture.port, 0x75u, 0x0003u);
    x86_pit_advance(fixture.pit.device, 4u);
    failed |= x86_pit_get_output(fixture.pit.device, 1u);
    x86_pit_advance(fixture.pit.device, 1u);
    failed |= !x86_pit_get_output(fixture.pit.device, 1u) ||
        core_machine_port_read(&fixture.port, 0x0041u) != 0x02u;
    failed |= core_machine_port_read(&fixture.port, 0x0041u) != 0x00u;
    core_machine_port_write(&fixture.port, 0x0043u, 0x00eau);
    status = (lib_u8)core_machine_port_read(&fixture.port, 0x0041u);
    failed |= status != 0xb5u;
    pit_irq0_finalize(&fixture);
    return failed;
}

static lib_i32 pit_irq0_test_gate_and_reset(void)
{
    pit_irq0_fixture fixture;
    lib_i32 failed = 0;

    if (pit_irq0_initialize(&fixture) != LIB_STATUS_OK) return 1;
    pit_irq0_program(&fixture.port, 0x32u, 3u);
    x86_pit_set_gate(fixture.pit.device, 0u, LIB_FALSE);
    x86_pit_advance(fixture.pit.device, 4u);
    failed |= !x86_pit_get_output(fixture.pit.device, 0u) || fixture.irq0.asserted;
    x86_pit_set_gate(fixture.pit.device, 0u, LIB_TRUE);
    x86_pit_advance(fixture.pit.device, 4u);
    failed |= !x86_pit_get_output(fixture.pit.device, 0u) || !fixture.irq0.asserted;
    core_machine_pic_reset(&fixture.master, &fixture.slave);
    x86_pit_reset(fixture.pit.device);
    failed |= fixture.irq0.asserted || fixture.master.asserted[0u] != 0u ||
        x86_pit_get_output(fixture.pit.device, 0u);
    pit_irq0_finalize(&fixture);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= pit_irq0_test_mode2_edge();
    failed |= pit_irq0_test_counter_forms();
    failed |= pit_irq0_test_gate_and_reset();
    if (failed != 0) return 1;
    printf("M5:T350:S2:PIT-IRQ0:OK\n");
    return 0;
}
