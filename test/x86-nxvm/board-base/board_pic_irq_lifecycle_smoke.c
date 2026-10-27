#include "pic_fixture.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-core/machine/machine_interface.h"

static lib_bool initialize_pic(x86_core_machine *port, lib_u8 icw1)
{
    return x86_core_machine_bus_write(port, 0x0020u, icw1) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x0021u, 0x08u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x0021u, 0x04u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x0021u, 0x01u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x00a0u, icw1) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x00a1u, 0x70u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x00a1u, 0x02u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x00a1u, 0x01u) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    x86_nxvm_board_pic_bus *master = LIB_NULL;
    x86_nxvm_board_pic_bus *slave = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine *port;
    x86_nxvm_board_pic_irq_source *irq1 = LIB_NULL;
    x86_nxvm_board_pic_irq_source *irq6 = LIB_NULL;
    x86_nxvm_board_pic_irq_source *irq14 = LIB_NULL;
    x86_nxvm_board_pic_irq_source *peers[2] = {LIB_NULL, LIB_NULL};
    lib_i32 failed = 0;

    const x86_core_machine_executor_config config = { .cpu_profile = X86_CORE_CPU_PROFILE_80386, .memory_bytes = (2u * 1024u * 1024u) };
    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    port = machine;
    failed = x86_nxvm_board_pic_initialize(&master, &slave, machine,
        X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (failed) goto cleanup;
    x86_nxvm_board_pic_reset(master, slave);
    failed = !initialize_pic(port, 0x11u) ||
        x86_nxvm_board_pic_irq_source_bind(&irq1, master, slave, 1u) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_irq_source_bind(&irq6, master, slave, 6u) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_irq_source_bind(&irq14, master, slave, 14u) != LIB_STATUS_OK;
    if (failed) goto cleanup;

    /* Shared sources only create an edge when their combined line rises. */
    for (lib_u8 index = 0u; index < 2u; ++index) {
        x86_nxvm_board_pic_irq_source *source = index == 0u ? irq6 : irq14;
        const lib_u8 irq = index == 0u ? 6u : 14u;
        const lib_u8 vector = index == 0u ? 0x0eu : 0x76u;
        failed |= x86_nxvm_board_pic_irq_source_bind(&peers[index], master,
            slave, irq) != LIB_STATUS_OK;
        if (failed) goto cleanup;
        x86_nxvm_board_pic_irq_source_assert(source);
        failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != vector;
        x86_nxvm_board_pic_irq_source_assert(peers[index]);
        if (index != 0u) test_pic_port_write(port, 0x00a0u, 0x20u);
        test_pic_port_write(port, 0x0020u, 0x20u);
        failed |= x86_nxvm_board_pic_scan_interrupt(master, slave);
        x86_nxvm_board_pic_irq_source_deassert(source);
        x86_nxvm_board_pic_irq_source_assert(source);
        failed |= x86_nxvm_board_pic_scan_interrupt(master, slave);
        x86_nxvm_board_pic_irq_source_deassert(peers[index]);
        x86_nxvm_board_pic_irq_source_deassert(source);
        x86_nxvm_board_pic_irq_source_assert(source);
        failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != vector;
        x86_nxvm_board_pic_irq_source_deassert(source);
        if (index != 0u) test_pic_port_write(port, 0x00a0u, 0x20u);
        test_pic_port_write(port, 0x0020u, 0x20u);
        failed |= x86_nxvm_board_pic_scan_interrupt(master, slave);
        x86_nxvm_board_pic_reset(master, slave);
        failed |= !initialize_pic(port, 0x11u);
    }
    if (failed) goto cleanup;

    /* PIC reset does not change the producers' physical input levels. */
    for (lib_u8 level = 0u; level < 2u; ++level) {
        for (lib_u8 index = 0u; index < 2u; ++index) {
            x86_nxvm_board_pic_irq_source *source = index == 0u ? irq6 : irq14;
            const lib_u8 vector = index == 0u ? 0x0eu : 0x76u;
            x86_nxvm_board_pic_irq_source_assert(source);
            x86_nxvm_board_pic_irq_source_assert(peers[index]);
            x86_nxvm_board_pic_reset(master, slave);
            failed |= !initialize_pic(port, level ? 0x19u : 0x11u);
            failed |= !x86_nxvm_board_pic_irq_source_is_asserted(source) ||
                !x86_nxvm_board_pic_irq_source_is_asserted(peers[index]);
            failed |= x86_nxvm_board_pic_scan_interrupt(master, slave) != level;
            x86_nxvm_board_pic_irq_source_deassert(source);
            failed |= x86_nxvm_board_pic_scan_interrupt(master, slave) != level;
            x86_nxvm_board_pic_irq_source_deassert(peers[index]);
            failed |= x86_nxvm_board_pic_scan_interrupt(master, slave);
            x86_nxvm_board_pic_irq_source_assert(source);
            failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != vector;
            x86_nxvm_board_pic_irq_source_deassert(source);
            if (index != 0u) test_pic_port_write(port, 0x00a0u, 0x20u);
            test_pic_port_write(port, 0x0020u, 0x20u);
            failed |= x86_nxvm_board_pic_scan_interrupt(master, slave);
        }
    }
    x86_nxvm_board_pic_reset(master, slave);
    failed |= !initialize_pic(port, 0x11u);
    if (failed) goto cleanup;

    x86_nxvm_board_pic_irq_source_assert(irq6);
    failed |= !x86_nxvm_board_pic_scan_interrupt(master, slave);
    failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != 0x0eu;
    x86_nxvm_board_pic_irq_source_deassert(irq6);
    failed |= (test_pic_read(master, 0x0bu) & VPIC_ISR_IRQ(6u)) == 0u;
    failed |= x86_core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;

    x86_nxvm_board_pic_irq_source_assert(irq6);
    x86_nxvm_board_pic_irq_source_assert(irq1);
    failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != 0x09u;
    x86_nxvm_board_pic_irq_source_deassert(irq1);
    failed |= x86_core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;
    failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != 0x0eu;
    x86_nxvm_board_pic_irq_source_deassert(irq6);
    failed |= x86_core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;

    failed |= x86_core_machine_bus_write(port, 0x0021u, VPIC_OCW1_IMR(6u)) != LIB_STATUS_OK;
    x86_nxvm_board_pic_irq_source_assert(irq6);
    failed |= x86_nxvm_board_pic_scan_interrupt(master, slave);
    failed |= x86_core_machine_bus_write(port, 0x0021u, 0u) != LIB_STATUS_OK;
    failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != 0x0eu;
    x86_nxvm_board_pic_irq_source_deassert(irq6);
    failed |= x86_core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;

    x86_nxvm_board_pic_irq_source_assert(irq14);
    x86_nxvm_board_pic_refresh(master, slave);
    failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != 0x76u;
    x86_nxvm_board_pic_irq_source_deassert(irq14);
    failed |= (test_pic_read(master, 0x0bu) & VPIC_ISR_IRQ(2u)) == 0u ||
        (test_pic_read(slave, 0x0bu) & VPIC_ISR_IRQ(6u)) == 0u;
    failed |= x86_core_machine_bus_write(port, 0x00a0u, 0x20u) != LIB_STATUS_OK;
    failed |= x86_core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;

    x86_nxvm_board_pic_reset(master, slave);
    failed |= !initialize_pic(port, 0x19u) ||
        x86_nxvm_board_pic_irq_source_bind(&irq1, master, slave, 1u) != LIB_STATUS_OK;
    if (failed) goto cleanup;
    x86_nxvm_board_pic_irq_source_assert(irq1);
    failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != 0x09u;
    failed |= x86_core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;
    x86_nxvm_board_pic_refresh(master, slave);
    failed |= !x86_nxvm_board_pic_scan_interrupt(master, slave);
    x86_nxvm_board_pic_irq_source_deassert(irq1);
    x86_nxvm_board_pic_refresh(master, slave);
    failed |= x86_nxvm_board_pic_scan_interrupt(master, slave);

    x86_nxvm_board_pic_reset(master, slave);
    failed |= !initialize_pic(port, 0x19u) ||
        x86_nxvm_board_pic_irq_source_bind(&irq14, master, slave, 14u) != LIB_STATUS_OK;
    if (failed) goto cleanup;
    x86_nxvm_board_pic_irq_source_assert(irq14);
    x86_nxvm_board_pic_refresh(master, slave);
    failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != 0x76u;
    failed |= (test_pic_read(master, 0x0bu) & VPIC_ISR_IRQ(2u)) == 0u ||
        (test_pic_read(slave, 0x0bu) & VPIC_ISR_IRQ(6u)) == 0u;
    failed |= x86_core_machine_bus_write(port, 0x00a0u, 0x20u) != LIB_STATUS_OK;
    failed |= x86_core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;
    x86_nxvm_board_pic_refresh(master, slave);
    failed |= !x86_nxvm_board_pic_scan_interrupt(master, slave);
    failed |= x86_nxvm_board_pic_get_interrupt(master, slave) != 0x76u;
    failed |= x86_core_machine_bus_write(port, 0x00a0u, 0x20u) != LIB_STATUS_OK;
    failed |= x86_core_machine_bus_write(port, 0x0020u, 0x20u) != LIB_STATUS_OK;
    x86_nxvm_board_pic_irq_source_deassert(irq14);
    x86_nxvm_board_pic_refresh(master, slave);
    failed |= x86_nxvm_board_pic_scan_interrupt(master, slave);

cleanup:
    x86_nxvm_board_pic_finalize(master, slave);
    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("PIC-IRQ-LIFECYCLE:OK\n");
    return 0;
}
