#include "pic_fixture.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86/ibmpc-common/pic_bus_interface.h"
#include "x86/core/machine_interface.h"

static lib_bool initialize_pic(core_machine *port, lib_u8 icw1)
{
    return core_machine_bus_write(port, 0x0020u, icw1) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x0021u, 0x08u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x0021u, 0x04u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x0021u, 0x01u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x00a0u, icw1) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x00a1u, 0x70u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x00a1u, 0x02u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x00a1u, 0x01u) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    core_machine_pic_bus *master = LIB_NULL;
    core_machine_pic_bus *slave = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine *port;
    core_machine_pic_irq_source *irq1 = LIB_NULL;
    core_machine_pic_irq_source *irq6 = LIB_NULL;
    core_machine_pic_irq_source *irq14 = LIB_NULL;
    lib_i32 failed = 0;

    const core_machine_executor_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    port = machine;
    failed = core_machine_pic_initialize(&master, &slave, machine,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK;
    if (failed) goto cleanup;
    core_machine_pic_reset(master, slave);
    failed = !initialize_pic(port, 0x11u) ||
        core_machine_pic_irq_source_bind(&irq1, master, slave, 1u) != LIB_STATUS_OK ||
        core_machine_pic_irq_source_bind(&irq6, master, slave, 6u) != LIB_STATUS_OK ||
        core_machine_pic_irq_source_bind(&irq14, master, slave, 14u) != LIB_STATUS_OK;
    if (failed) goto cleanup;

    core_machine_pic_irq_source_assert(irq6);
    core_machine_pic_irq_source_deassert(irq6);
    failed |= !core_machine_pic_scan_interrupt(master, slave);
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x0eu;
    failed |= (test_pic_read(master, 0x0bu) & VPIC_ISR_IRQ(6u)) == 0u;
    failed |= core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;

    core_machine_pic_irq_source_assert(irq6);
    core_machine_pic_irq_source_deassert(irq6);
    core_machine_pic_irq_source_assert(irq1);
    core_machine_pic_irq_source_deassert(irq1);
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x09u;
    failed |= core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x0eu;
    failed |= core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;

    failed |= core_machine_bus_write(port, 0x0021u, VPIC_OCW1_IMR(6u)) != LIB_STATUS_OK;
    core_machine_pic_irq_source_assert(irq6);
    core_machine_pic_irq_source_deassert(irq6);
    failed |= core_machine_pic_scan_interrupt(master, slave);
    failed |= core_machine_bus_write(port, 0x0021u, 0u) != LIB_STATUS_OK;
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x0eu;
    failed |= core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;

    core_machine_pic_irq_source_assert(irq14);
    core_machine_pic_irq_source_deassert(irq14);
    core_machine_pic_refresh(master, slave);
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x76u;
    failed |= (test_pic_read(master, 0x0bu) & VPIC_ISR_IRQ(2u)) == 0u ||
        (test_pic_read(slave, 0x0bu) & VPIC_ISR_IRQ(6u)) == 0u;
    failed |= core_machine_bus_write(port, 0x00a0u, 0x20u) != LIB_STATUS_OK;
    failed |= core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;

    core_machine_pic_reset(master, slave);
    failed |= !initialize_pic(port, 0x19u) ||
        core_machine_pic_irq_source_bind(&irq1, master, slave, 1u) != LIB_STATUS_OK;
    if (failed) goto cleanup;
    core_machine_pic_irq_source_assert(irq1);
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x09u;
    failed |= core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;
    core_machine_pic_refresh(master, slave);
    failed |= !core_machine_pic_scan_interrupt(master, slave);
    core_machine_pic_irq_source_deassert(irq1);
    core_machine_pic_refresh(master, slave);
    failed |= core_machine_pic_scan_interrupt(master, slave);

    core_machine_pic_reset(master, slave);
    failed |= !initialize_pic(port, 0x19u) ||
        core_machine_pic_irq_source_bind(&irq14, master, slave, 14u) != LIB_STATUS_OK;
    if (failed) goto cleanup;
    core_machine_pic_irq_source_assert(irq14);
    core_machine_pic_refresh(master, slave);
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x76u;
    failed |= (test_pic_read(master, 0x0bu) & VPIC_ISR_IRQ(2u)) == 0u ||
        (test_pic_read(slave, 0x0bu) & VPIC_ISR_IRQ(6u)) == 0u;
    failed |= core_machine_bus_write(port, 0x00a0u, 0x20u) != LIB_STATUS_OK;
    failed |= core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;
    core_machine_pic_refresh(master, slave);
    failed |= !core_machine_pic_scan_interrupt(master, slave);
    failed |= core_machine_pic_get_interrupt(master, slave) != 0x76u;
    failed |= core_machine_bus_write(port, 0x00a0u, 0x20u) != LIB_STATUS_OK;
    failed |= core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;
    core_machine_pic_irq_source_deassert(irq14);
    core_machine_pic_refresh(master, slave);
    failed |= core_machine_pic_scan_interrupt(master, slave);

cleanup:
    core_machine_pic_finalize(master, slave);
    core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("M5:T216:S1:PIC-IRQ-LIFECYCLE:OK\n");
    return 0;
}
