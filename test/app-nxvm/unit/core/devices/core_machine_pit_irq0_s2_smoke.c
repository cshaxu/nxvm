#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/pit.h"
#include "app-nxvm/devices/port.h"

typedef struct pit_irq0_fixture {
    t_pic master;
    t_pic slave;
    t_pit pit;
    t_port port;
    core_machine_pic_irq_source irq0;
} pit_irq0_fixture;

static void pit_irq0_initialize(pit_irq0_fixture *fixture)
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
    core_machine_pit_initialize(&fixture->pit, &fixture->port);
    core_machine_pit_reset(&fixture->pit);
    core_machine_pit_set_output(&fixture->pit, 0u,
        core_machine_pic_timer_output, &fixture->irq0);
}

static void pit_irq0_finalize(pit_irq0_fixture *fixture)
{
    core_machine_pit_finalize(&fixture->pit);
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

    pit_irq0_initialize(&fixture);
    pit_irq0_program(&fixture.port, 0x34u, 3u);
    failed |= fixture.irq0.asserted ||
        !core_machine_pit_get_output(&fixture.pit, 0u) ||
        core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_pit_advance(&fixture.pit, 4u);
    failed |= core_machine_pit_get_output(&fixture.pit, 0u) ||
        fixture.irq0.asserted || core_machine_pic_scan_interrupt(&fixture.master,
            &fixture.slave);
    core_machine_pit_advance(&fixture.pit, 1u);
    failed |= !core_machine_pit_get_output(&fixture.pit, 0u) ||
        !fixture.irq0.asserted || fixture.master.data.asserted[0u] != 1u ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x08u;
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    failed |= fixture.master.data.isr != 0u ||
        core_machine_pic_scan_interrupt(&fixture.master, &fixture.slave);
    core_machine_pit_advance(&fixture.pit, 2u);
    failed |= core_machine_pit_get_output(&fixture.pit, 0u) || fixture.irq0.asserted ||
        fixture.master.data.asserted[0u] != 0u;
    pit_irq0_finalize(&fixture);
    return failed;
}

static lib_i32 pit_irq0_test_counter_forms(void)
{
    pit_irq0_fixture fixture;
    lib_u8 status;
    lib_i32 failed = 0;

    pit_irq0_initialize(&fixture);
    core_machine_port_write(&fixture.port, 0x0043u, 0x70u);
    core_machine_port_write(&fixture.port, 0x0041u, 0x34u);
    failed |= fixture.pit.data.flagReady[1u] || fixture.pit.data.flagActive[1u] ||
        fixture.pit.data.flagWrite[1u] != VPIT_STATUS_RW_MSB;
    core_machine_port_write(&fixture.port, 0x0041u, 0x12u);
    failed |= fixture.pit.data.flagReady[1u] || !fixture.pit.data.flagLoadPending[1u] ||
        fixture.pit.data.count[1u] != 0u;
    core_machine_pit_advance(&fixture.pit, 1u);
    failed |= !fixture.pit.data.flagReady[1u] || fixture.pit.data.count[1u] != 0x1234u;
    core_machine_port_write(&fixture.port, 0x0043u, 0x00d8u);
    failed |= core_machine_port_read(&fixture.port, 0x0041u) != 0x34u ||
        core_machine_port_read(&fixture.port, 0x0041u) != 0x12u;
    pit_irq0_program(&fixture.port, 0x75u, 0x0003u);
    core_machine_pit_advance(&fixture.pit, 4u);
    failed |= core_machine_pit_get_output(&fixture.pit, 1u);
    core_machine_pit_advance(&fixture.pit, 1u);
    failed |= !core_machine_pit_get_output(&fixture.pit, 1u) ||
        fixture.pit.data.count[1u] != 0x0002u;
    core_machine_port_write(&fixture.port, 0x0043u, 0x00eau);
    status = (lib_u8)core_machine_port_read(&fixture.port, 0x0041u);
    failed |= (status & (VPIT_SB_OUT | VPIT_SB_NC | VPIT_SB_RW | VPIT_SB_M |
        VPIT_SB_BCD)) != (VPIT_SB_OUT | 0x30u | 0x04u | VPIT_SB_BCD);
    pit_irq0_finalize(&fixture);
    return failed;
}

static lib_i32 pit_irq0_test_gate_and_reset(void)
{
    pit_irq0_fixture fixture;
    lib_i32 failed = 0;

    pit_irq0_initialize(&fixture);
    pit_irq0_program(&fixture.port, 0x32u, 3u);
    core_machine_pit_set_gate(&fixture.pit, 0u, LIB_FALSE);
    core_machine_pit_advance(&fixture.pit, 4u);
    failed |= !core_machine_pit_get_output(&fixture.pit, 0u) || fixture.irq0.asserted;
    core_machine_pit_set_gate(&fixture.pit, 0u, LIB_TRUE);
    core_machine_pit_advance(&fixture.pit, 4u);
    failed |= !core_machine_pit_get_output(&fixture.pit, 0u) || !fixture.irq0.asserted;
    core_machine_pic_reset(&fixture.master, &fixture.slave);
    core_machine_pit_reset(&fixture.pit);
    failed |= fixture.irq0.asserted || fixture.master.data.asserted[0u] != 0u ||
        core_machine_pit_get_output(&fixture.pit, 0u);
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
