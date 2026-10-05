#include "lib/types/types_interface.h"
#include "lib/types/test.h"
#include "ibmpc/board-common/at_contract_interface.h"

static void effective_values(void)
{
    const lib_u32 all = (1u << (VM_AT_DEVICE_BOARD + 1u)) - 1u;
    const lib_u32 video_planes = (1u << VM_AT_DEVICE_VADP_ATTRIBUTE) |
        (1u << VM_AT_DEVICE_VADP_SEQUENCER) | (1u << VM_AT_DEVICE_VADP_GRAPHICS);
    const struct {
        core_machine_cpu_profile cpu;
        lib_size memory;
        lib_u32 enabled;
        const vm_at_route *routes;
        lib_size route_count;
        lib_size port_count;
        lib_bool cga;
    } cases[] = {
        {CORE_MACHINE_CPU_PROFILE_80386, 1024u * 1024u, all, vm_at_routes_with_aux, 5u, 79u, LIB_FALSE},
        {CORE_MACHINE_CPU_PROFILE_80286, 512u * 1024u, all & ~video_planes,
            vm_at_routes_without_aux, 4u, 73u, LIB_TRUE},
        {CORE_MACHINE_CPU_PROFILE_80386, 2u * 1024u * 1024u, all & ~video_planes,
            vm_at_routes_without_aux, 4u, 73u, LIB_TRUE}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        vm_profile_contract_core_input core = {.id = 1u, .configuration = {
            .cpu_profile = cases[index].cpu, .memory_bytes = cases[index].memory,
            .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_CASCADED,
            .dma_controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT}};
        vm_profile_contract_values values = {0};
        lib_test_assert(vm_at_contract_materialize(&core, vm_at_port_leaves, VM_AT_PORT_LEAF_COUNT,
            cases[index].routes, cases[index].route_count, cases[index].enabled,
            cases[index].cga, &values) == LIB_STATUS_OK);
        lib_test_assert(lib_memory_compare(&core, &values.core, sizeof(core)) == 0);
        lib_test_assert(values.enabled_devices == cases[index].enabled);
        lib_test_assert(values.port_leaf_count == cases[index].port_count);
        for (lib_size port = 1u; port < values.port_leaf_count; ++port)
            lib_test_assert(values.port_leaves[port - 1u].device <= values.port_leaves[port].device);
        lib_test_assert(values.irq_route_count == cases[index].route_count && values.drq_route_count == 1u);
        lib_test_assert(values.irq_routes[0].line == 0u && values.irq_routes[1].line == 1u);
        lib_test_assert(values.irq_routes[cases[index].route_count - 2u].line == 8u &&
            values.irq_routes[cases[index].route_count - 1u].line == 6u);
        lib_test_assert(values.drq_routes[0].device == (1u << VM_AT_DEVICE_FDC) && values.drq_routes[0].line == 2u);
        lib_test_assert(values.memory_window_count == (cases[index].cga ? 1u : 0u));
        if (cases[index].cga) lib_test_assert(values.memory_windows[0].first == 0x000b8000u &&
            values.memory_windows[0].last == 0x000bffffu);
        lib_test_assert(values.firmware_policy == 0u && values.media_policy == 0u &&
            values.allowed_session_options == 0u);
    }
}

static void invalid_inputs(void)
{
    const vm_profile_contract_core_input core = {.id = 1u, .configuration = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386, .memory_bytes = 1024u * 1024u}};
    const lib_u32 enabled = (1u << (VM_AT_DEVICE_BOARD + 1u)) - 1u;
    vm_at_port_leaf leaves[VM_AT_PORT_LEAF_COUNT];
    vm_at_route routes[5u];
    vm_profile_contract_values values = {0};

    for (lib_size variant = 0u; variant < 9u; ++variant) {
        lib_memory_copy(leaves, vm_at_port_leaves, sizeof(leaves));
        lib_memory_copy(routes, vm_at_routes_with_aux, sizeof(routes));
        vm_profile_contract_core_input input = core;
        if (variant == 0u) leaves[0].device = (vm_at_device_role)99;
        if (variant == 1u) leaves[1].port = leaves[0].port;
        if (variant == 2u) leaves[0].read = leaves[0].write = LIB_FALSE;
        if (variant == 3u) routes[0].source = (vm_at_route_source)99;
        if (variant == 4u) routes[1].irq = routes[0].irq;
        if (variant == 5u) input.configuration.memory_bytes = 0u;
        if (variant == 6u) input.configuration.cpu_profile = CORE_MACHINE_CPU_PROFILE_DEFAULT;
        values.enabled_devices = 99u;
        lib_test_assert(vm_at_contract_materialize(&input, leaves,
            variant == 7u ? VM_PROFILE_CONTRACT_PORT_LEAF_CAPACITY + 1u : VM_AT_PORT_LEAF_COUNT,
            routes, variant == 8u ? VM_PROFILE_CONTRACT_ROUTE_CAPACITY + 1u : 5u,
            enabled, LIB_FALSE, &values) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(values.enabled_devices == 0u && values.port_leaf_count == 0u);
    }
    lib_test_assert(vm_at_contract_materialize(LIB_NULL, leaves, VM_AT_PORT_LEAF_COUNT,
        routes, 5u, enabled, LIB_FALSE, &values) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_at_contract_materialize(&core, LIB_NULL, VM_AT_PORT_LEAF_COUNT,
        routes, 5u, enabled, LIB_FALSE, &values) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_at_contract_materialize(&core, leaves, VM_AT_PORT_LEAF_COUNT,
        LIB_NULL, 5u, enabled, LIB_FALSE, &values) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_at_contract_materialize(&core, vm_at_port_leaves, VM_AT_PORT_LEAF_COUNT,
        vm_at_routes_with_aux, 5u, 1u << 31u, LIB_FALSE, &values) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_at_contract_materialize(&core, vm_at_port_leaves, VM_AT_PORT_LEAF_COUNT,
        vm_at_routes_with_aux, 5u, enabled & ~(1u << VM_AT_DEVICE_KBC), LIB_FALSE, &values) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_at_contract_materialize(&core, vm_at_port_leaves, VM_AT_PORT_LEAF_COUNT,
        vm_at_routes_with_aux, 5u, enabled, LIB_FALSE, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
}

lib_i32 main(void)
{
    effective_values();
    invalid_inputs();
    return 0;
}
