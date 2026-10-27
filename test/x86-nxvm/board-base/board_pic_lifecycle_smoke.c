#include "lib/types/test.h"
#include "pic_fixture.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/pic_bus.h"
#include "x86-nxvm/board-base/pit_bus_interface.h"
#include "x86-core/machine/machine_interface.h"

static lib_bool pic_lifecycle_fail_allocation;

static void *pic_lifecycle_allocate_zero(lib_size count, lib_size bytes)
{
    if (pic_lifecycle_fail_allocation) return LIB_NULL;
    return lib_allocate_zero(count, bytes);
}

/* Same-owner construction failure seam; no production test hook or ABI. */
#define lib_allocate_zero pic_lifecycle_allocate_zero
#include "x86-nxvm/board-base/pic_bus.c"
#undef lib_allocate_zero

typedef struct pic_lifecycle_fixture {
    x86_nxvm_board_pic_bus *master;
    x86_nxvm_board_pic_bus *slave;
    x86_core_machine *machine;
} pic_lifecycle_fixture;

static void pic_lifecycle_initialize(pic_lifecycle_fixture *fixture,
    lib_u8 icw1, x86_pit *pit)
{
    const x86_core_machine_executor_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .memory_bytes = (2u * 1024u * 1024u)
    };
    *fixture = (pic_lifecycle_fixture){0};
    if (x86_core_machine_neutral_create(&config, &fixture->machine) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_initialize(&fixture->master, &fixture->slave, fixture->machine,
            X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        (pit != LIB_NULL && x86_nxvm_board_pit_install_ports(fixture->machine, pit,
            0x0040u) != LIB_STATUS_OK) ||
        x86_core_machine_freeze_execution_providers(fixture->machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(fixture->machine) != LIB_STATUS_OK) {
        x86_nxvm_board_pic_finalize(fixture->master, fixture->slave);
        x86_core_machine_destroy(fixture->machine);
        lib_test_assert(LIB_FALSE);
    }
    x86_nxvm_board_pic_reset(fixture->master, fixture->slave);
    test_pic_port_write(fixture->machine, 0x0020u, icw1);
    test_pic_port_write(fixture->machine, 0x0021u, 0x08u);
    test_pic_port_write(fixture->machine, 0x0021u, 0x04u);
    test_pic_port_write(fixture->machine, 0x0021u, 0x01u);
    test_pic_port_write(fixture->machine, 0x00a0u, icw1);
    test_pic_port_write(fixture->machine, 0x00a1u, 0x70u);
    test_pic_port_write(fixture->machine, 0x00a1u, 0x02u);
    test_pic_port_write(fixture->machine, 0x00a1u, 0x01u);
}

static void pic_lifecycle_finalize(pic_lifecycle_fixture *fixture)
{
    x86_nxvm_board_pic_finalize(fixture->master, fixture->slave);
    x86_core_machine_destroy(fixture->machine);
}

static void pic_lifecycle_eoi(pic_lifecycle_fixture *fixture,
    lib_bool slave)
{
    if (slave) test_pic_port_write(fixture->machine, 0x00a0u, 0x20u);
    test_pic_port_write(fixture->machine, 0x0020u, 0x20u);
}

static void pic_lifecycle_bind(pic_lifecycle_fixture *fixture,
    x86_nxvm_board_pic_irq_source **source, lib_u8 irq)
{
    if (x86_nxvm_board_pic_irq_source_bind(source, fixture->master, fixture->slave,
            irq) != LIB_STATUS_OK) {
        pic_lifecycle_finalize(fixture);
        lib_test_assert(LIB_FALSE);
    }
}

static lib_i32 pic_lifecycle_test_master_level(void)
{
    pic_lifecycle_fixture fixture;
    x86_nxvm_board_pic_irq_source *first = LIB_NULL;
    x86_nxvm_board_pic_irq_source *second = LIB_NULL;
    lib_i32 failed = 0;

    pic_lifecycle_initialize(&fixture, 0x19u, LIB_NULL);
    pic_lifecycle_bind(&fixture, &first, 5u);
    pic_lifecycle_bind(&fixture, &second, 5u);
    x86_nxvm_board_pic_irq_source_assert(first);
    x86_nxvm_board_pic_irq_source_assert(second);
    failed |= fixture.master->asserted[5u] != 2u ||
        x86_nxvm_board_pic_get_interrupt(fixture.master, fixture.slave) != 0x0du;
    pic_lifecycle_eoi(&fixture, LIB_FALSE);
    x86_nxvm_board_pic_refresh(fixture.master, fixture.slave);
    failed |= !x86_nxvm_board_pic_scan_interrupt(fixture.master, fixture.slave);
    x86_nxvm_board_pic_irq_source_deassert(first);
    failed |= fixture.master->asserted[5u] != 1u;
    (void)x86_nxvm_board_pic_get_interrupt(fixture.master, fixture.slave);
    pic_lifecycle_eoi(&fixture, LIB_FALSE);
    x86_nxvm_board_pic_refresh(fixture.master, fixture.slave);
    failed |= !x86_nxvm_board_pic_scan_interrupt(fixture.master, fixture.slave);
    x86_nxvm_board_pic_irq_source_deassert(second);
    x86_nxvm_board_pic_refresh(fixture.master, fixture.slave);
    failed |= x86_nxvm_board_pic_scan_interrupt(fixture.master, fixture.slave);
    pic_lifecycle_finalize(&fixture);
    return failed;
}

static lib_i32 pic_lifecycle_test_slave_level(void)
{
    pic_lifecycle_fixture fixture;
    x86_nxvm_board_pic_irq_source *first = LIB_NULL;
    x86_nxvm_board_pic_irq_source *second = LIB_NULL;
    lib_i32 failed = 0;

    pic_lifecycle_initialize(&fixture, 0x19u, LIB_NULL);
    pic_lifecycle_bind(&fixture, &first, 14u);
    pic_lifecycle_bind(&fixture, &second, 14u);
    x86_nxvm_board_pic_irq_source_assert(first);
    x86_nxvm_board_pic_irq_source_assert(second);
    x86_nxvm_board_pic_refresh(fixture.master, fixture.slave);
    failed |= fixture.slave->asserted[6u] != 2u ||
        x86_nxvm_board_pic_get_interrupt(fixture.master, fixture.slave) != 0x76u;
    pic_lifecycle_eoi(&fixture, LIB_TRUE);
    x86_nxvm_board_pic_refresh(fixture.master, fixture.slave);
    failed |= !x86_nxvm_board_pic_scan_interrupt(fixture.master, fixture.slave);
    x86_nxvm_board_pic_irq_source_deassert(first);
    (void)x86_nxvm_board_pic_get_interrupt(fixture.master, fixture.slave);
    pic_lifecycle_eoi(&fixture, LIB_TRUE);
    x86_nxvm_board_pic_refresh(fixture.master, fixture.slave);
    failed |= fixture.slave->asserted[6u] != 1u ||
        !x86_nxvm_board_pic_scan_interrupt(fixture.master, fixture.slave);
    x86_nxvm_board_pic_irq_source_deassert(second);
    x86_nxvm_board_pic_refresh(fixture.master, fixture.slave);
    failed |= x86_nxvm_board_pic_scan_interrupt(fixture.master, fixture.slave);
    pic_lifecycle_finalize(&fixture);
    return failed;
}

static lib_i32 pic_lifecycle_test_pit_reset(void)
{
    pic_lifecycle_fixture fixture;
    x86_pit *pit = LIB_NULL;
    x86_nxvm_board_pic_irq_source *irq0 = LIB_NULL;
    lib_i32 failed = 0;

    if (x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK) return 1;
    pic_lifecycle_initialize(&fixture, 0x11u, pit);
    pic_lifecycle_bind(&fixture, &irq0, 0u);
    x86_pit_reset(pit);
    x86_pit_set_output(pit, 0u, x86_nxvm_board_pic_timer_output, irq0);
    test_pic_port_write(fixture.machine, 0x0043u, 0x34u);
    test_pic_port_write(fixture.machine, 0x0040u, 3u);
    test_pic_port_write(fixture.machine, 0x0040u, 0u);
    /* The first clock transfers CR to CE; the IRQ0 edge is the fifth clock. */
    x86_pit_advance(pit, 5u);
    failed |= !irq0->asserted || fixture.master->asserted[0u] != 1u;
    x86_nxvm_board_pic_reset(fixture.master, fixture.slave);
    x86_pit_reset(pit);
    failed |= irq0->asserted || fixture.master->asserted[0u] != 0u;
    test_pic_port_write(fixture.machine, 0x0043u, 0x34u);
    test_pic_port_write(fixture.machine, 0x0040u, 3u);
    test_pic_port_write(fixture.machine, 0x0040u, 0u);
    x86_pit_advance(pit, 5u);
    failed |= !irq0->asserted || fixture.master->asserted[0u] != 1u ||
        (test_pic_read(fixture.master, 0x0au) & VPIC_IRR_IRQ(0u)) == 0u;
    x86_pit_destroy(pit);
    failed |= irq0->asserted || fixture.master->asserted[0u] != 0u;
    pic_lifecycle_finalize(&fixture);
    return failed;
}

static lib_i32 pic_lifecycle_test_edge_empty_and_bind(void)
{
    pic_lifecycle_fixture fixture;
    x86_nxvm_board_pic_irq_source *source = LIB_NULL;
    lib_i32 failed = 0;

    pic_lifecycle_initialize(&fixture, 0x11u, LIB_NULL);
    lib_memory_set(&source, 0u, sizeof(source));
    failed |= x86_nxvm_board_pic_irq_source_bind(&source, fixture.master,
        fixture.slave, 2u) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= source != LIB_NULL || x86_nxvm_board_pic_irq_source_is_asserted(source);
    failed |= x86_nxvm_board_pic_irq_source_bind(&source, fixture.master,
        fixture.slave, 16u) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= source != LIB_NULL || x86_nxvm_board_pic_irq_source_is_asserted(source);
    failed |= x86_nxvm_board_pic_get_interrupt(fixture.master, fixture.slave) != 0x0fu ||
        test_pic_read(fixture.master, 0x0bu) != 0u || test_pic_read(fixture.slave, 0x0bu) != 0u;
    pic_lifecycle_bind(&fixture, &source, 1u);
    x86_nxvm_board_pic_irq_source_assert(source);
    failed |= x86_nxvm_board_pic_get_interrupt(fixture.master, fixture.slave) != 0x09u;
    x86_nxvm_board_pic_irq_source_deassert(source);
    pic_lifecycle_eoi(&fixture, LIB_FALSE);
    failed |= x86_nxvm_board_pic_scan_interrupt(fixture.master, fixture.slave);
    pic_lifecycle_finalize(&fixture);
    return failed;
}

static lib_i32 pic_lifecycle_test_allocation_rollback(void)
{
    const x86_core_machine_executor_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .memory_bytes = (2u * 1024u * 1024u)
    };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_pic_bus *master = LIB_NULL;
    x86_nxvm_board_pic_bus *slave = LIB_NULL;
    x86_nxvm_board_pic_irq_source *source = LIB_NULL;
    x86_nxvm_board_pic_irq_source *original;
    lib_i32 failed = 0;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    pic_lifecycle_fail_allocation = LIB_TRUE;
    failed |= x86_nxvm_board_pic_initialize(&master, &slave, machine,
        X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_NO_MEMORY;
    failed |= master != LIB_NULL || slave != LIB_NULL;
    pic_lifecycle_fail_allocation = LIB_FALSE;
    if (failed || x86_nxvm_board_pic_initialize(&master, &slave, machine,
        X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return 1;
    }
    pic_lifecycle_fail_allocation = LIB_TRUE;
    failed |= x86_nxvm_board_pic_irq_source_bind(&source, master, slave, 1u) !=
        LIB_STATUS_NO_MEMORY;
    failed |= source != LIB_NULL || master->sources != LIB_NULL;
    pic_lifecycle_fail_allocation = LIB_FALSE;
    failed |= x86_nxvm_board_pic_irq_source_bind(&source, master, slave, 1u) !=
        LIB_STATUS_OK;
    original = source;
    pic_lifecycle_fail_allocation = LIB_TRUE;
    failed |= x86_nxvm_board_pic_irq_source_bind(&source, master, slave, 1u) !=
        LIB_STATUS_OK || source != original;
    pic_lifecycle_fail_allocation = LIB_FALSE;
    failed |= x86_nxvm_board_pic_irq_source_bind(&source, master, slave, 2u) !=
        LIB_STATUS_INVALID_ARGUMENT || source != original;
    x86_nxvm_board_pic_finalize(master, slave);
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_status pic_lifecycle_existing_port(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    (void)port;
    (void)tick;
    *out_value = *(lib_u8 *)owner;
    return LIB_STATUS_OK;
}

static lib_i32 pic_lifecycle_test_route_rollback(void)
{
    const x86_core_machine_executor_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .memory_bytes = (2u * 1024u * 1024u)
    };
    const x86_nxvm_board_pic_topology topologies[] = {
        X86_NXVM_BOARD_PIC_TOPOLOGY_SINGLE, X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED
    };
    lib_i32 failed = 0;
    for (lib_size index = 0u; index < sizeof(topologies) / sizeof(topologies[0]); ++index) {
        x86_core_machine *machine = LIB_NULL;
        x86_nxvm_board_pic_bus *master = LIB_NULL;
        x86_nxvm_board_pic_bus *slave = LIB_NULL;
        lib_u8 existing_value = 0x5au;
        lib_u32 value = 0u;
        const lib_u16 collision = topologies[index] == X86_NXVM_BOARD_PIC_TOPOLOGY_SINGLE ?
            0x21u : 0xa1u;
        const x86_core_machine_port_route existing = {
            .address = collision, .read = pic_lifecycle_existing_port,
            .owner = &existing_value
        };
        if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
        failed |= x86_core_machine_install_port_routes(machine, &existing, 1u) != LIB_STATUS_OK;
        failed |= x86_nxvm_board_pic_initialize(&master, &slave, machine,
            topologies[index]) != LIB_STATUS_INVALID_STATE;
        failed |= master != LIB_NULL || slave != LIB_NULL;
        failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= x86_core_machine_bus_read(machine, collision, &value) != LIB_STATUS_OK ||
            value != existing_value;
        failed |= x86_core_machine_bus_read(machine, 0x20u, &value) != LIB_STATUS_UNSUPPORTED;
        failed |= x86_core_machine_bus_read(machine, 0xa0u, &value) != LIB_STATUS_UNSUPPORTED;
        if (collision != 0x21u)
            failed |= x86_core_machine_bus_read(machine, 0x21u, &value) != LIB_STATUS_UNSUPPORTED;
        x86_core_machine_destroy(machine);
    }
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= pic_lifecycle_test_master_level();
    failed |= pic_lifecycle_test_slave_level();
    failed |= pic_lifecycle_test_pit_reset();
    failed |= pic_lifecycle_test_edge_empty_and_bind();
    failed |= pic_lifecycle_test_allocation_rollback();
    failed |= pic_lifecycle_test_route_rollback();
    if (failed != 0) return 1;
    lib_c_printf("PIC-LIFECYCLE:OK\n");
    return 0;
}
