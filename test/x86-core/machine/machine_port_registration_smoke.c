#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine.h"
#include "x86-core/machine/port.h"

typedef struct machine_port_probe {
    lib_u32 value;
    lib_u64 read_tick;
    lib_u32 reads;
    lib_bool inconsistent_tick;
    lib_u8 write_bytes;
    lib_u32 writes;
} machine_port_probe;

static lib_status machine_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    machine_port_probe *state = (machine_port_probe *)owner;

    (void)port;
    if (state == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (state->reads != 0u && state->read_tick != tick)
        state->inconsistent_tick = LIB_TRUE;
    state->read_tick = tick;
    ++state->reads;
    *out_value = state->value;
    return LIB_STATUS_OK;
}

static lib_status machine_port_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    machine_port_probe *state = (machine_port_probe *)owner;

    (void)port;
    if (state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    state->value = value;
    state->write_bytes = access_bytes;
    ++state->writes;
    return LIB_STATUS_OK;
}

static lib_i32 machine_port_fresh_create(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    x86_core_machine *machine = LIB_NULL;
    const lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 machine_port_range_transaction(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_machine_port_provider provider = { machine_port_read, machine_port_write };
    x86_core_machine_port_test_allocation allocation = { 3u, 0u };
    machine_port_probe state = {0u};
    x86_core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        x86_core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= x86_core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_NO_MEMORY;
        failed |= x86_core_machine_port_has_read(&machine->executor_port, 0x00e0u) ||
            x86_core_machine_port_has_write(&machine->executor_port, 0x00e0u) ||
            x86_core_machine_port_has_read(&machine->executor_port, 0x00e1u) ||
            x86_core_machine_port_has_write(&machine->executor_port, 0x00e1u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= x86_core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_OK;
        failed |= x86_core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_INVALID_STATE;
        failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00e0u, 0x5au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00e0u, &value) != LIB_STATUS_OK ||
            value != 0x5au;
    }
    x86_core_machine_destroy(machine);
    return failed || machine_port_fresh_create();
}

static lib_i32 machine_port_batch_transaction(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_machine_port_provider existing = {
        machine_port_read, machine_port_write};
    machine_port_probe state = {0u};
    x86_core_machine_port_test_allocation allocation = {3u, 0u};
    x86_core_machine_port_route routes[] = {
        {0x00e2u, machine_port_read, machine_port_write, &state, LIB_FALSE, 0u},
        {0x00e4u, machine_port_read, machine_port_write, &state, LIB_FALSE, 0u}
    };
    x86_core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        failed |= x86_core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
            &existing, &state) != LIB_STATUS_OK;
        x86_core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= x86_core_machine_install_port_routes(machine, routes, 2u) !=
            LIB_STATUS_NO_MEMORY;
        failed |= !x86_core_machine_port_has_read(&machine->executor_port, 0x00e0u) ||
            x86_core_machine_port_has_read(&machine->executor_port, 0x00e2u) ||
            x86_core_machine_port_has_write(&machine->executor_port, 0x00e2u) ||
            x86_core_machine_port_has_read(&machine->executor_port, 0x00e4u) ||
            x86_core_machine_port_has_write(&machine->executor_port, 0x00e4u);
        allocation.fail_at = 0u;
        failed |= x86_core_machine_install_port_routes(machine, routes, 2u) !=
            LIB_STATUS_OK;
        failed |= x86_core_machine_install_port_routes(machine, routes, 2u) !=
            LIB_STATUS_INVALID_STATE;
        failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00e4u, 0x3cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00e2u, &value) != LIB_STATUS_OK ||
            value != 0x3cu;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 machine_port_read_time(void)
{
    t_port ports = {0};
    machine_port_probe lane = {.value = 0x5au};
    machine_port_probe contributor = {.value = 0x80u};
    const x86_core_machine_port_route bank = {
        .address = 0x100u, .read = machine_port_read,
        .write = machine_port_write, .owner = &lane,
        .byte_lane_end = 0x104u
    };
    const x86_core_machine_port_route wired_or = {
        .address = 0x100u, .read = machine_port_read, .owner = &contributor,
        .wired_or_read = LIB_TRUE
    };
    const x86_core_machine_port_route unsplit = {
        .address = 0x200u, .write = machine_port_write, .owner = &contributor
    };
    const lib_u64 tick = 0x100000003ull;
    lib_i32 failed;

    x86_core_machine_port_initialize(&ports);
    failed = x86_core_machine_port_add_route(&ports, &bank) != LIB_STATUS_OK ||
        x86_core_machine_port_add_route(&ports, &wired_or) != LIB_STATUS_OK ||
        x86_core_machine_port_add_route(&ports, &unsplit) != LIB_STATUS_OK ||
        x86_core_machine_port_execute_read_width(&ports, 0x100u, 4u, tick) !=
            LIB_STATUS_OK || ports.data.ioDWord != 0x5a5a5adau ||
        lane.reads != 4u || contributor.reads != 1u ||
        lane.read_tick != tick || contributor.read_tick != tick ||
        lane.inconsistent_tick || contributor.inconsistent_tick;
    /* The existing fixture convenience is explicitly zero-time, not live time. */
    lane.reads = contributor.reads = 0u;
    failed |= x86_core_machine_port_read(&ports, 0x100u) != 0xdau ||
        lane.read_tick != 0u || contributor.read_tick != 0u;
    for (lib_u8 bytes = 1u; bytes <= 4u; bytes *= 2u) {
        const lib_u32 prior_writes = contributor.writes;

        ports.data.ioDWord = 0x44332211u;
        failed |= x86_core_machine_port_execute_write_width(&ports,
            0x200u, bytes) != LIB_STATUS_OK ||
            contributor.write_bytes != bytes ||
            contributor.writes != prior_writes + 1u ||
            contributor.value != 0x44332211u;
    }
    ports.data.ioDWord = 0x44332211u;
    failed |= x86_core_machine_port_execute_write_width(&ports, 0x100u,
        4u) != LIB_STATUS_OK || lane.write_bytes != 1u ||
        lane.writes != 4u || lane.value != 0x44u;
    x86_core_machine_port_finalize(&ports);
    return failed;
}

static lib_i32 machine_port_allocation_prefixes(void)
{
    static const lib_size counts[] = {2u, 3u, 7u, 8u, 15u, 18u, 19u, 27u, 22u,
        113u, 120u, 61u};
    x86_core_machine_port_route routes[120];
    machine_port_probe sentinel = {.value = 0x5au};
    machine_port_probe candidate = {.value = 0xa5u};
    const x86_core_machine_port_route existing = {
        .address = 0x80u, .read = machine_port_read, .owner = &sentinel
    };
    lib_i32 failed = 0;

    for (lib_size index = 0u; index < sizeof(routes) / sizeof(routes[0]); ++index) {
        routes[index] = (x86_core_machine_port_route){
            .address = (lib_u16)(0x100u + index),
            .write = machine_port_write, .owner = &candidate
        };
    }
    for (lib_size variant = 0u; variant < sizeof(counts) / sizeof(counts[0]); ++variant) {
        const lib_size count = counts[variant];

        for (lib_size fail_at = 1u; fail_at <= count; ++fail_at) {
            x86_core_machine machine = {.lifecycle = X86_CORE_MACHINE_INITIALIZED};
            x86_core_machine_port_test_allocation allocation = {fail_at, 0u};
            t_port *port = &machine.executor_port;

            x86_core_machine_port_initialize(port);
            failed |= x86_core_machine_install_port_routes(&machine, &existing, 1u) != LIB_STATUS_OK;
            x86_core_machine_port_set_test_allocation(port, &allocation);
            failed |= x86_core_machine_install_port_routes(&machine, routes, count) != LIB_STATUS_NO_MEMORY ||
                allocation.attempts != fail_at ||
                x86_core_machine_port_read(port, 0x80u) != 0x5au;
            for (lib_size index = 0u; index < count; ++index)
                failed |= x86_core_machine_port_has_write(port, routes[index].address);
            allocation.fail_at = count + 1u;
            allocation.attempts = 0u;
            failed |= x86_core_machine_install_port_routes(&machine, routes, count) != LIB_STATUS_OK ||
                allocation.attempts != count;
            for (lib_size index = 0u; index < count; ++index)
                failed |= !x86_core_machine_port_has_write(port, routes[index].address);
            failed |= x86_core_machine_remove_port_routes(&machine, &candidate) != LIB_STATUS_OK ||
                x86_core_machine_port_read(port, 0x80u) != 0x5au;
            for (lib_size index = 0u; index < count; ++index)
                failed |= x86_core_machine_port_has_write(port, routes[index].address);
            x86_core_machine_port_finalize(port);
        }
    }
    return failed;
}

lib_i32 main(void)
{
    if (machine_port_range_transaction() || machine_port_batch_transaction() ||
        machine_port_read_time() || machine_port_allocation_prefixes()) return 1;
    lib_c_printf("PORT-REGISTRATION:OK\n");
    lib_c_printf("PORT-READ-TIMESTAMP:OK\n");
    lib_c_printf("PORT-WRITE-WIDTH:OK\n");
    return 0;
}
