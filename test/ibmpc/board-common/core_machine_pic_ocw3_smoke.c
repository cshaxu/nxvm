#include "lib/types/test.h"
#include "pic_fixture.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "ibmpc/board-common/pic_bus_interface.h"
#include "x86/core/machine_interface.h"

typedef struct pic_ocw3_fixture {
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
    core_machine *machine;
} pic_ocw3_fixture;

static void pic_ocw3_initialize(pic_ocw3_fixture *fixture,
    lib_u8 master_icw4)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    *fixture = (pic_ocw3_fixture){0};
    if (core_machine_neutral_create(&config, &fixture->machine) != LIB_STATUS_OK ||
        core_machine_pic_initialize(&fixture->master, &fixture->slave, fixture->machine,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(fixture->machine) != LIB_STATUS_OK ||
        core_machine_reset(fixture->machine) != LIB_STATUS_OK) {
        core_machine_pic_finalize(fixture->master, fixture->slave);
        core_machine_destroy(fixture->machine);
        lib_test_assert(LIB_FALSE);
    }
    core_machine_pic_reset(fixture->master, fixture->slave);
    test_pic_port_write(fixture->machine, 0x0020u, 0x11u);
    test_pic_port_write(fixture->machine, 0x0021u, 0x08u);
    test_pic_port_write(fixture->machine, 0x0021u, 0x04u);
    test_pic_port_write(fixture->machine, 0x0021u, master_icw4);
    test_pic_port_write(fixture->machine, 0x00a0u, 0x11u);
    test_pic_port_write(fixture->machine, 0x00a1u, 0x70u);
    test_pic_port_write(fixture->machine, 0x00a1u, 0x02u);
    test_pic_port_write(fixture->machine, 0x00a1u, 0x01u);
}

static void pic_ocw3_finalize(pic_ocw3_fixture *fixture)
{
    core_machine_pic_finalize(fixture->master, fixture->slave);
    core_machine_destroy(fixture->machine);
}

static void pic_ocw3_raise(pic_ocw3_fixture *fixture,
    core_machine_pic_irq_source **source, lib_u8 irq)
{
    if (core_machine_pic_irq_source_bind(source, fixture->master, fixture->slave,
            irq) != LIB_STATUS_OK) {
        pic_ocw3_finalize(fixture);
        lib_test_assert(LIB_FALSE);
    }
    core_machine_pic_irq_source_assert(*source);
    core_machine_pic_irq_source_deassert(*source);
}

static lib_i32 pic_ocw3_test_read_and_poll(void)
{
    pic_ocw3_fixture fixture;
    core_machine_pic_irq_source *irq4 = LIB_NULL;
    core_machine_pic_irq_source *irq5 = LIB_NULL;
    core_machine_pic_irq_source *irq14 = LIB_NULL;
    lib_i32 failed = 0;

    pic_ocw3_initialize(&fixture, 0x01u);
    pic_ocw3_raise(&fixture, &irq4, 4u);
    failed |= test_pic_port_read(fixture.machine, 0x0020u) != VPIC_IRR_IRQ(4u);
    test_pic_port_write(fixture.machine, 0x0020u, 0x0cu);
    failed |= test_pic_port_read(fixture.machine, 0x0020u) !=
        (VPIC_POLL_I | 4u) || test_pic_read(fixture.master, 0x0au) != 0u ||
        test_pic_read(fixture.master, 0x0bu) != VPIC_ISR_IRQ(4u);
    failed |= test_pic_port_read(fixture.machine, 0x0020u) != 0u;
    test_pic_port_write(fixture.machine, 0x0020u, 0x0bu);
    failed |= test_pic_port_read(fixture.machine, 0x0020u) != VPIC_ISR_IRQ(4u) ||
        test_pic_port_read(fixture.machine, 0x0020u) != VPIC_ISR_IRQ(4u);
    test_pic_port_write(fixture.machine, 0x0020u, 0x20u);
    test_pic_port_write(fixture.machine, 0x0020u, 0x0cu);
    failed |= test_pic_port_read(fixture.machine, 0x0020u) != 0u;

    pic_ocw3_finalize(&fixture);
    irq4 = irq5 = irq14 = LIB_NULL;
    pic_ocw3_initialize(&fixture, 0x03u);
    pic_ocw3_raise(&fixture, &irq5, 5u);
    test_pic_port_write(fixture.machine, 0x0020u, 0x0cu);
    failed |= test_pic_port_read(fixture.machine, 0x0020u) !=
        (VPIC_POLL_I | 5u) || test_pic_read(fixture.master, 0x0bu) != 0u;
    pic_ocw3_finalize(&fixture);
    irq4 = irq5 = irq14 = LIB_NULL;

    pic_ocw3_initialize(&fixture, 0x01u);
    pic_ocw3_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(fixture.master, fixture.slave);
    test_pic_port_write(fixture.machine, 0x0020u, 0x0cu);
    failed |= test_pic_port_read(fixture.machine, 0x0020u) !=
        (VPIC_POLL_I | 2u) || test_pic_read(fixture.master, 0x0au) != 0u ||
        test_pic_read(fixture.master, 0x0bu) != VPIC_ISR_IRQ(2u);
    test_pic_port_write(fixture.machine, 0x00a0u, 0x0cu);
    failed |= test_pic_port_read(fixture.machine, 0x00a0u) !=
        (VPIC_POLL_I | 6u) || test_pic_read(fixture.slave, 0x0bu) != VPIC_ISR_IRQ(6u);
    pic_ocw3_finalize(&fixture);
    irq4 = irq5 = irq14 = LIB_NULL;
    return failed;
}

static lib_i32 pic_ocw3_test_special_mask(void)
{
    pic_ocw3_fixture fixture;
    core_machine_pic_irq_source *irq1 = LIB_NULL;
    core_machine_pic_irq_source *irq5 = LIB_NULL;
    lib_i32 failed = 0;

    pic_ocw3_initialize(&fixture, 0x01u);
    pic_ocw3_raise(&fixture, &irq1, 1u);
    failed |= core_machine_pic_get_interrupt(fixture.master, fixture.slave) != 0x09u;
    pic_ocw3_raise(&fixture, &irq5, 5u);
    failed |= core_machine_pic_scan_interrupt(fixture.master, fixture.slave);
    test_pic_port_write(fixture.machine, 0x0020u, 0x68u);
    test_pic_port_write(fixture.machine, 0x0021u, VPIC_OCW1_IMR(1u));
    failed |= test_pic_read(fixture.master, 0x0bu) != VPIC_ISR_IRQ(1u) ||
        !core_machine_pic_scan_interrupt(fixture.master, fixture.slave) ||
        core_machine_pic_get_interrupt(fixture.master, fixture.slave) != 0x0du;
    test_pic_port_write(fixture.machine, 0x0020u, 0x20u);
    failed |= test_pic_read(fixture.master, 0x0bu) != VPIC_ISR_IRQ(1u);
    test_pic_port_write(fixture.machine, 0x0021u, 0u);
    test_pic_port_write(fixture.machine, 0x0020u, 0x20u);
    failed |= test_pic_read(fixture.master, 0x0bu) != 0u;
    test_pic_port_write(fixture.machine, 0x0020u, 0x48u);
    /* Exact OCW3 bit retention is covered by the independent chip corpus. */
    pic_ocw3_finalize(&fixture);
    irq1 = irq5 = LIB_NULL;
    return failed;
}

static lib_i32 pic_ocw3_test_sfnm(void)
{
    pic_ocw3_fixture fixture;
    core_machine_pic_irq_source *irq8 = LIB_NULL;
    core_machine_pic_irq_source *irq14 = LIB_NULL;
    lib_i32 failed = 0;

    pic_ocw3_initialize(&fixture, 0x01u);
    pic_ocw3_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(fixture.master, fixture.slave);
    failed |= core_machine_pic_get_interrupt(fixture.master, fixture.slave) != 0x76u;
    pic_ocw3_raise(&fixture, &irq8, 8u);
    core_machine_pic_refresh(fixture.master, fixture.slave);
    failed |= core_machine_pic_scan_interrupt(fixture.master, fixture.slave);
    pic_ocw3_finalize(&fixture);
    irq8 = irq14 = LIB_NULL;

    pic_ocw3_initialize(&fixture, 0x11u);
    pic_ocw3_raise(&fixture, &irq14, 14u);
    core_machine_pic_refresh(fixture.master, fixture.slave);
    failed |= core_machine_pic_get_interrupt(fixture.master, fixture.slave) != 0x76u;
    pic_ocw3_raise(&fixture, &irq8, 8u);
    core_machine_pic_refresh(fixture.master, fixture.slave);
    failed |= !core_machine_pic_scan_interrupt(fixture.master, fixture.slave) ||
        core_machine_pic_get_interrupt(fixture.master, fixture.slave) != 0x70u;
    pic_ocw3_finalize(&fixture);
    irq8 = irq14 = LIB_NULL;
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= pic_ocw3_test_read_and_poll();
    failed |= pic_ocw3_test_special_mask();
    failed |= pic_ocw3_test_sfnm();
    if (failed != 0) return 1;
    lib_c_printf("PIC-OCW3:OK\n");
    return 0;
}
