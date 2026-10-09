#include "pic_fixture.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "core/board-base/pic_bus.h"
#include "core/board-base/pit_bus_interface.h"
#include "core/x86/machine_interface.h"

typedef struct pit_irq0_fixture {
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
    x86_pit *pit;
    core_machine *machine;
    core_machine_pic_irq_source *irq0;
} pit_irq0_fixture;

static void pit_irq0_finalize(pit_irq0_fixture *fixture);

static lib_status pit_irq0_initialize(pit_irq0_fixture *fixture)
{
    const core_machine_executor_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    *fixture = (pit_irq0_fixture){0};
    lib_status status = core_machine_neutral_create(&config, &fixture->machine);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_pic_initialize(&fixture->master, &fixture->slave, fixture->machine,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    if (status != LIB_STATUS_OK) goto failed;
    core_machine_pic_reset(fixture->master, fixture->slave);
    if ((status = core_machine_pic_write_register(fixture->master, 0u, 0x11u)) != LIB_STATUS_OK ||
        (status = core_machine_pic_write_register(fixture->master, 1u, 0x08u)) != LIB_STATUS_OK ||
        (status = core_machine_pic_write_register(fixture->master, 1u, 0x04u)) != LIB_STATUS_OK ||
        (status = core_machine_pic_write_register(fixture->master, 1u, 0x01u)) != LIB_STATUS_OK ||
        (status = core_machine_pic_write_register(fixture->slave, 0u, 0x11u)) != LIB_STATUS_OK ||
        (status = core_machine_pic_write_register(fixture->slave, 1u, 0x70u)) != LIB_STATUS_OK ||
        (status = core_machine_pic_write_register(fixture->slave, 1u, 0x02u)) != LIB_STATUS_OK ||
        (status = core_machine_pic_write_register(fixture->slave, 1u, 0x01u)) != LIB_STATUS_OK)
        goto failed;
    status = core_machine_pic_irq_source_bind(&fixture->irq0, fixture->master,
        fixture->slave, 0u);
    if (status != LIB_STATUS_OK) goto failed;
    status = x86_pit_create(X86_PIT_PERSONALITY_8254, &fixture->pit);
    if (status == LIB_STATUS_OK) {
        status = core_machine_pit_install_ports(fixture->machine,
            fixture->pit, 0x0040u);
    }
    if (status != LIB_STATUS_OK) goto failed;
    status = core_machine_freeze_execution_providers(fixture->machine);
    if (status != LIB_STATUS_OK) goto failed;
    status = core_machine_reset(fixture->machine);
    if (status != LIB_STATUS_OK) goto failed;
    x86_pit_reset(fixture->pit);
    x86_pit_set_output(fixture->pit, 0u,
        core_machine_pic_timer_output, fixture->irq0);
    return LIB_STATUS_OK;
failed:
    pit_irq0_finalize(fixture);
    return status;
}

static void pit_irq0_finalize(pit_irq0_fixture *fixture)
{
    x86_pit_destroy(fixture->pit);
    core_machine_pic_finalize(fixture->master, fixture->slave);
    core_machine_destroy(fixture->machine);
}

static void pit_irq0_program(core_machine *port, lib_u8 control,
    lib_u16 count)
{
    lib_u16 data_port = (lib_u16)(0x0040u +
        ((control >> 6u) & 0x03u));

    test_pic_port_write(port, 0x0043u, control);
    test_pic_port_write(port, data_port, count & 0xffu);
    test_pic_port_write(port, data_port, count >> 8u);
}

static lib_i32 pit_irq0_test_mode2_edge(void)
{
    pit_irq0_fixture fixture;
    lib_i32 failed = 0;

    if (pit_irq0_initialize(&fixture) != LIB_STATUS_OK) return 1;
    pit_irq0_program(fixture.machine, 0x34u, 3u);
    failed |= core_machine_pic_irq_source_is_asserted(fixture.irq0) ||
        !x86_pit_get_output(fixture.pit, 0u) ||
        core_machine_pic_scan_interrupt(fixture.master, fixture.slave);
    x86_pit_advance(fixture.pit, 4u);
    failed |= x86_pit_get_output(fixture.pit, 0u) ||
        core_machine_pic_irq_source_is_asserted(fixture.irq0) || core_machine_pic_scan_interrupt(fixture.master,
            fixture.slave);
    x86_pit_advance(fixture.pit, 1u);
    failed |= !x86_pit_get_output(fixture.pit, 0u) ||
        !core_machine_pic_irq_source_is_asserted(fixture.irq0) || fixture.master->asserted[0u] != 1u ||
        core_machine_pic_get_interrupt(fixture.master, fixture.slave) != 0x08u;
    test_pic_port_write(fixture.machine, 0x0020u, 0x20u);
    failed |= test_pic_read(fixture.master, 0x0bu) != 0u ||
        core_machine_pic_scan_interrupt(fixture.master, fixture.slave);
    x86_pit_advance(fixture.pit, 2u);
    failed |= x86_pit_get_output(fixture.pit, 0u) || core_machine_pic_irq_source_is_asserted(fixture.irq0) ||
        fixture.master->asserted[0u] != 0u;
    pit_irq0_finalize(&fixture);
    return failed;
}

static lib_i32 pit_irq0_test_counter_forms(void)
{
    pit_irq0_fixture fixture;
    lib_u8 status;
    lib_i32 failed = 0;

    if (pit_irq0_initialize(&fixture) != LIB_STATUS_OK) return 1;
    test_pic_port_write(fixture.machine, 0x0043u, 0x70u);
    test_pic_port_write(fixture.machine, 0x0041u, 0x34u);
    test_pic_port_write(fixture.machine, 0x0043u, 0xeau);
    failed |= test_pic_port_read(fixture.machine, 0x0041u) != 0x70u;
    test_pic_port_write(fixture.machine, 0x0041u, 0x12u);
    test_pic_port_write(fixture.machine, 0x0043u, 0xeau);
    failed |= test_pic_port_read(fixture.machine, 0x0041u) != 0x70u;
    x86_pit_advance(fixture.pit, 1u);
    test_pic_port_write(fixture.machine, 0x0043u, 0xeau);
    failed |= test_pic_port_read(fixture.machine, 0x0041u) != 0x30u;
    test_pic_port_write(fixture.machine, 0x0043u, 0x00d8u);
    failed |= test_pic_port_read(fixture.machine, 0x0041u) != 0x34u ||
        test_pic_port_read(fixture.machine, 0x0041u) != 0x12u;
    pit_irq0_program(fixture.machine, 0x75u, 0x0003u);
    x86_pit_advance(fixture.pit, 4u);
    failed |= x86_pit_get_output(fixture.pit, 1u);
    x86_pit_advance(fixture.pit, 1u);
    failed |= !x86_pit_get_output(fixture.pit, 1u) ||
        test_pic_port_read(fixture.machine, 0x0041u) != 0x02u;
    failed |= test_pic_port_read(fixture.machine, 0x0041u) != 0x00u;
    test_pic_port_write(fixture.machine, 0x0043u, 0x00eau);
    status = (lib_u8)test_pic_port_read(fixture.machine, 0x0041u);
    failed |= status != 0xb5u;
    pit_irq0_finalize(&fixture);
    return failed;
}

static lib_i32 pit_irq0_test_gate_and_reset(void)
{
    pit_irq0_fixture fixture;
    lib_i32 failed = 0;

    if (pit_irq0_initialize(&fixture) != LIB_STATUS_OK) return 1;
    pit_irq0_program(fixture.machine, 0x32u, 3u);
    x86_pit_set_gate(fixture.pit, 0u, LIB_FALSE);
    x86_pit_advance(fixture.pit, 4u);
    failed |= !x86_pit_get_output(fixture.pit, 0u) || core_machine_pic_irq_source_is_asserted(fixture.irq0);
    x86_pit_set_gate(fixture.pit, 0u, LIB_TRUE);
    x86_pit_advance(fixture.pit, 4u);
    failed |= !x86_pit_get_output(fixture.pit, 0u) || !core_machine_pic_irq_source_is_asserted(fixture.irq0);
    core_machine_pic_reset(fixture.master, fixture.slave);
    x86_pit_reset(fixture.pit);
    failed |= core_machine_pic_irq_source_is_asserted(fixture.irq0) || fixture.master->asserted[0u] != 0u ||
        x86_pit_get_output(fixture.pit, 0u);
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
    lib_c_printf("PIT-IRQ0:OK\n");
    return 0;
}
