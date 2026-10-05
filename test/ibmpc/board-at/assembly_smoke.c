#include "lib/types/types_interface.h"
#include "lib/types/test.h"
#include "ibmpc/board-common/at_assembly_interface.h"

static void topology_values(void)
{
    for (lib_u32 ega = 0u; ega < 2u; ++ega) {
        core_machine_plan_topology topology = {0};
        topology.display.ega_present = (lib_bool)ega;
        topology.display.ega_personality = X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
        topology.display.cecg.lightpen_switch_open = LIB_TRUE;
        topology.rtc_cmos.ticks_per_second = 32768u;
        topology.rtc_cmos.nmi_mask_bit = 0x80u;
        topology.rtc_cmos.default_count = 1u;
        topology.rtc_cmos.defaults[0].index = 0x30u;
        topology.rtc_cmos.defaults[0].value = 0x04u;
        topology.memory_alias_count = 1u;
        topology.memory_alias[0] = (core_machine_memory_alias_config) {0xfa0000u, 0xa0000u, 0x60000u};
        lib_test_assert(vm_at_topology_materialize(vm_at_port_leaves, VM_AT_PORT_LEAF_COUNT,
            vm_at_routes_without_aux, 4u, &topology) == LIB_STATUS_OK);
        lib_test_assert(topology.display_present && topology.rtc_cmos_present && topology.dma_present);
        lib_test_assert(topology.display.ports.crtc_first == 0x03d4u &&
            topology.display.ports.crtc_last == 0x03dau);
        lib_test_assert(topology.display.ports.attribute_first == (ega ? 0x03c0u : 0u));
        lib_test_assert(topology.display.cecg.lightpen_switch_open && topology.display.ega_personality ==
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR);
        lib_test_assert(topology.rtc_cmos.index_port == 0x70u &&
            topology.rtc_cmos.data_port == 0x71u && topology.rtc_cmos.irq == 8u);
        lib_test_assert(topology.rtc_cmos.ticks_per_second == 32768u &&
            topology.rtc_cmos.defaults[0].value == 0x04u);
        lib_test_assert(topology.dma.fdc_channel == 2u && topology.memory_alias[0].physical_start == 0xfa0000u);
        core_machine_plan_topology before = topology;
        lib_test_assert(vm_at_topology_materialize(vm_at_port_leaves, VM_AT_PORT_LEAF_COUNT,
            vm_at_routes_without_aux, 3u, &topology) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(lib_memory_compare(&before, &topology, sizeof(before)) == 0);
    }
}

static void endpoints(void)
{
    vm_at_port_leaf leaves[VM_AT_PORT_LEAF_COUNT];
    vm_at_route routes[4u];
    core_machine_fdc_config fdc = {.ready_mask = 5u, .clock_ticks_per_second = 8000000u,
        .diagnostic_port = 0x123u, .diagnostic_read_value = 0xa5u};
    lib_memory_copy(leaves, vm_at_port_leaves, sizeof(leaves));
    lib_memory_copy(routes, vm_at_routes_without_aux, sizeof(routes));
    const vm_at_port_leaf *data = vm_at_port_leaf_at(leaves, VM_AT_PORT_LEAF_COUNT,
        VM_AT_DEVICE_MASK_ALL, VM_AT_DEVICE_FDC, 2u);
    lib_test_assert(data != LIB_NULL);
    leaves[data - leaves].port = 0x234u;
    routes[3u].irq = 5u;
    routes[3u].dma_channel = 1u;
    lib_test_assert(vm_at_fdc_materialize(leaves, VM_AT_PORT_LEAF_COUNT, routes, 4u, &fdc) == LIB_STATUS_OK);
    lib_test_assert(fdc.data_port == 0x234u && fdc.irq == 5u && fdc.dma_channel == 1u);
    lib_test_assert(fdc.ready_mask == 5u && fdc.clock_ticks_per_second == 8000000u &&
        fdc.diagnostic_port == 0x123u && fdc.diagnostic_read_value == 0xa5u);
    lib_test_assert(vm_at_port_leaf_find(leaves, VM_AT_PORT_LEAF_COUNT,
        VM_AT_DEVICE_MASK_ALL, VM_AT_DEVICE_FDC, 0x234u) == data);
    lib_test_assert(vm_at_port_leaf_at(leaves, VM_AT_PORT_LEAF_COUNT,
        VM_AT_DEVICE_MASK_ALL & ~(1u << VM_AT_DEVICE_FDC), VM_AT_DEVICE_FDC, 2u) == LIB_NULL);
    lib_test_assert(vm_at_port_leaf_at(leaves, VM_AT_PORT_LEAF_COUNT,
        VM_AT_DEVICE_MASK_ALL, (vm_at_device_role)99u, 0u) == LIB_NULL);
    core_machine_fdc_config before = fdc;
    lib_test_assert(vm_at_fdc_materialize(leaves, 0u, routes, 4u, &fdc) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_memory_compare(&before, &fdc, sizeof(before)) == 0);
}

static void missing_endpoints(void)
{
    const vm_at_device_role required[] = { VM_AT_DEVICE_CMOS, VM_AT_DEVICE_VADP,
        VM_AT_DEVICE_VADP_ATTRIBUTE, VM_AT_DEVICE_VADP_SEQUENCER, VM_AT_DEVICE_VADP_GRAPHICS };
    for (lib_size missing = 0u; missing < sizeof(required) / sizeof(required[0]); ++missing) {
        vm_at_port_leaf leaves[VM_AT_PORT_LEAF_COUNT];
        lib_size count = 0u;
        core_machine_plan_topology topology = {0};
        topology.display.ega_present = LIB_TRUE;
        topology.rtc_cmos.ticks_per_second = 32768u;
        core_machine_plan_topology before = topology;
        for (lib_size index = 0u; index < VM_AT_PORT_LEAF_COUNT; ++index) {
            if (vm_at_port_leaves[index].device != required[missing])
                leaves[count++] = vm_at_port_leaves[index];
        }
        lib_test_assert(vm_at_topology_materialize(leaves, count,
            vm_at_routes_without_aux, 4u, &topology) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(lib_memory_compare(&before, &topology, sizeof(before)) == 0);
    }
}

lib_i32 main(void)
{
    topology_values();
    endpoints();
    missing_endpoints();
    return 0;
}
