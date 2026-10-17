#include "lib/types/types_interface.h"
#include "lib/types/test.h"
#include "x86-nxvm/board-base/at_assembly_interface.h"
#include "x86-nxvm/board-base/pc_at_profile_interface.h"

static lib_bool reject_descriptor;
static lib_i32 descriptor_validate(const x86_nxvm_board_pc_at_descriptor *descriptor)
{ return descriptor != LIB_NULL && !reject_descriptor; }

static void descriptor_contracts(void)
{
    x86_nxvm_board_at_port_leaf leaves[X86_NXVM_BOARD_AT_PORT_LEAF_COUNT];
    x86_nxvm_board_at_route routes[4];
    x86_nxvm_board_pc_at_firmware_service services[1] = {{X86_NXVM_BOARD_PC_AT_FIRMWARE_TIMER_IRQ0, 8u}};
    lib_memory_copy(leaves, x86_nxvm_board_at_port_leaves, sizeof(leaves));
    lib_memory_copy(routes, x86_nxvm_board_at_routes_without_aux, sizeof(routes));
    x86_nxvm_board_pc_at_descriptor descriptor = {
        .identity = "unit-at", .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE, .ticks_per_instruction = 7u,
        .default_memory_bytes = 2u * 1024u * 1024u, .fdc_bounce_segment = 0x600u,
        .kbc_aux_absent = LIB_TRUE, .port_leaves = leaves, .port_leaf_count = X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
        .routes = routes, .route_count = 4u, .firmware_services = services,
        .firmware_service_count = 1u, .validate = descriptor_validate
    };
    const struct { x86_core_cpu_profile cpu; x86_fpu_profile fpu; } requests[] = {
        {X86_CORE_CPU_PROFILE_DEFAULT, X86_FPU_PROFILE_NONE},
        {X86_CORE_CPU_PROFILE_8086, X86_FPU_PROFILE_8087},
        {X86_CORE_CPU_PROFILE_80186, X86_FPU_PROFILE_NONE},
        {X86_CORE_CPU_PROFILE_80286, X86_FPU_PROFILE_80287},
        {X86_CORE_CPU_PROFILE_80386, X86_FPU_PROFILE_80387},
        {X86_CORE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE}
    };
    x86_nxvm_board_pc_at_cpu_contract contract = {0}, before;
    x86_nxvm_board_config core;
    x86_nxvm_board_controller_timing_rules timing;
    lib_test_assert(x86_nxvm_board_pc_at_descriptor_is_valid(&descriptor));
    for (lib_size i = 0u; i < sizeof(requests) / sizeof(requests[0]); ++i) {
        lib_test_assert(x86_nxvm_board_pc_at_cpu_contract_select(&descriptor,
            requests[i].cpu, requests[i].fpu, &contract));
        lib_test_assert(contract.cpu_profile == (requests[i].cpu == X86_CORE_CPU_PROFILE_DEFAULT ?
            descriptor.cpu_profile : requests[i].cpu) && contract.fpu_profile == requests[i].fpu &&
            contract.ticks_per_instruction == 7u);
        lib_test_assert(x86_nxvm_board_pc_at_core_config_materialize(&descriptor, &contract, &core, &timing));
        lib_test_assert(core.cpu_profile == contract.cpu_profile && core.fpu_profile == contract.fpu_profile &&
            core.memory_bytes == descriptor.default_memory_bytes && core.ticks_per_instruction == 7u &&
            core.pic_topology == X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED);
    }
    lib_memory_copy(&before, &contract, sizeof(before));
    lib_test_assert(!x86_nxvm_board_pc_at_cpu_contract_select(&descriptor,
        X86_CORE_CPU_PROFILE_8088, X86_FPU_PROFILE_NONE, &contract));
    lib_test_assert(lib_memory_compare(&contract, &before, sizeof(before)) == 0);
    lib_test_assert(!x86_nxvm_board_pc_at_cpu_contract_select(&descriptor,
        X86_CORE_CPU_PROFILE_80386, (x86_fpu_profile)99u, &contract));
    lib_test_assert(lib_memory_compare(&contract, &before, sizeof(before)) == 0);
    reject_descriptor = LIB_TRUE;
    lib_test_assert(!x86_nxvm_board_pc_at_cpu_contract_select(&descriptor,
        X86_CORE_CPU_PROFILE_DEFAULT, X86_FPU_PROFILE_NONE, &contract));
    lib_test_assert(lib_memory_compare(&contract, &before, sizeof(before)) == 0);
    reject_descriptor = LIB_FALSE;
    x86_nxvm_board_pc_at_plan_snapshot snapshot = {0};
    lib_test_assert(x86_nxvm_board_pc_at_values_create(&descriptor,
        X86_CORE_CPU_PROFILE_DEFAULT, X86_FPU_PROFILE_NONE, &snapshot.values) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_board_pc_at_snapshot_copy(&snapshot, &descriptor, "copied-at", 4u) == LIB_STATUS_OK);
    lib_test_assert(snapshot.descriptor.port_leaves == snapshot.port_leaves &&
        snapshot.descriptor.routes == snapshot.routes && snapshot.descriptor.firmware_services == snapshot.firmware_services);
    leaves[0].port ^= 1u;
    routes[0].irq ^= 1u;
    services[0].vector = 99u;
    lib_test_assert(snapshot.port_leaves[0].port == x86_nxvm_board_at_port_leaves[0].port &&
        snapshot.routes[0].irq == x86_nxvm_board_at_routes_without_aux[0].irq && snapshot.firmware_services[0].vector == 8u);
    lib_test_assert(lib_text_compare(snapshot.descriptor.identity, "copied-at") == 0 &&
        snapshot.descriptor.cmos.floppy_type == 4u);
}

static void topology_values(void)
{
    for (lib_u32 ega = 0u; ega < 2u; ++ega) {
        x86_nxvm_board_plan_topology topology = {0};
        topology.display.ega_present = (lib_bool)ega;
        topology.display.ega_personality = X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
        topology.display.cecg.lightpen_switch_open = LIB_TRUE;
        topology.rtc_cmos.ticks_per_second = 32768u;
        topology.rtc_cmos.nmi_mask_bit = 0x80u;
        topology.rtc_cmos.default_count = 1u;
        topology.rtc_cmos.defaults[0].index = 0x30u;
        topology.rtc_cmos.defaults[0].value = 0x04u;
        topology.memory_alias_count = 1u;
        topology.memory_alias[0] = (x86_core_machine_memory_alias_config) {0xfa0000u, 0xa0000u, 0x60000u};
        lib_test_assert(x86_nxvm_board_at_topology_materialize(x86_nxvm_board_at_port_leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
            x86_nxvm_board_at_routes_without_aux, 4u, &topology) == LIB_STATUS_OK);
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
        x86_nxvm_board_plan_topology before = topology;
        lib_test_assert(x86_nxvm_board_at_topology_materialize(x86_nxvm_board_at_port_leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
            x86_nxvm_board_at_routes_without_aux, 3u, &topology) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(lib_memory_compare(&before, &topology, sizeof(before)) == 0);
    }
}

static void endpoints(void)
{
    x86_nxvm_board_at_port_leaf leaves[X86_NXVM_BOARD_AT_PORT_LEAF_COUNT];
    x86_nxvm_board_at_route routes[4u];
    x86_nxvm_board_fdc_config fdc = {.ready_mask = 5u, .clock_ticks_per_second = 8000000u,
        .diagnostic_port = 0x123u, .diagnostic_read_value = 0xa5u};
    lib_memory_copy(leaves, x86_nxvm_board_at_port_leaves, sizeof(leaves));
    lib_memory_copy(routes, x86_nxvm_board_at_routes_without_aux, sizeof(routes));
    const x86_nxvm_board_at_port_leaf *data = x86_nxvm_board_at_port_leaf_at(leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
        X86_NXVM_BOARD_AT_DEVICE_MASK_ALL, X86_NXVM_BOARD_AT_DEVICE_FDC, 2u);
    lib_test_assert(data != LIB_NULL);
    leaves[data - leaves].port = 0x234u;
    routes[3u].irq = 5u;
    routes[3u].dma_channel = 1u;
    lib_test_assert(x86_nxvm_board_at_fdc_materialize(leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT, routes, 4u, &fdc) == LIB_STATUS_OK);
    lib_test_assert(fdc.data_port == 0x234u && fdc.irq == 5u && fdc.dma_channel == 1u);
    lib_test_assert(fdc.ready_mask == 5u && fdc.clock_ticks_per_second == 8000000u &&
        fdc.diagnostic_port == 0x123u && fdc.diagnostic_read_value == 0xa5u);
    lib_test_assert(x86_nxvm_board_at_port_leaf_find(leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
        X86_NXVM_BOARD_AT_DEVICE_MASK_ALL, X86_NXVM_BOARD_AT_DEVICE_FDC, 0x234u) == data);
    lib_test_assert(x86_nxvm_board_at_port_leaf_at(leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
        X86_NXVM_BOARD_AT_DEVICE_MASK_ALL & ~(1u << X86_NXVM_BOARD_AT_DEVICE_FDC), X86_NXVM_BOARD_AT_DEVICE_FDC, 2u) == LIB_NULL);
    lib_test_assert(x86_nxvm_board_at_port_leaf_at(leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
        X86_NXVM_BOARD_AT_DEVICE_MASK_ALL, (x86_nxvm_board_at_device_role)99u, 0u) == LIB_NULL);
    x86_nxvm_board_fdc_config before = fdc;
    lib_test_assert(x86_nxvm_board_at_fdc_materialize(leaves, 0u, routes, 4u, &fdc) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_memory_compare(&before, &fdc, sizeof(before)) == 0);
}

static void missing_endpoints(void)
{
    const x86_nxvm_board_at_device_role required[] = { X86_NXVM_BOARD_AT_DEVICE_CMOS, X86_NXVM_BOARD_AT_DEVICE_VADP,
        X86_NXVM_BOARD_AT_DEVICE_VADP_ATTRIBUTE, X86_NXVM_BOARD_AT_DEVICE_VADP_SEQUENCER, X86_NXVM_BOARD_AT_DEVICE_VADP_GRAPHICS };
    for (lib_size missing = 0u; missing < sizeof(required) / sizeof(required[0]); ++missing) {
        x86_nxvm_board_at_port_leaf leaves[X86_NXVM_BOARD_AT_PORT_LEAF_COUNT];
        lib_size count = 0u;
        x86_nxvm_board_plan_topology topology = {0};
        topology.display.ega_present = LIB_TRUE;
        topology.rtc_cmos.ticks_per_second = 32768u;
        x86_nxvm_board_plan_topology before = topology;
        for (lib_size index = 0u; index < X86_NXVM_BOARD_AT_PORT_LEAF_COUNT; ++index) {
            if (x86_nxvm_board_at_port_leaves[index].device != required[missing])
                leaves[count++] = x86_nxvm_board_at_port_leaves[index];
        }
        lib_test_assert(x86_nxvm_board_at_topology_materialize(leaves, count,
            x86_nxvm_board_at_routes_without_aux, 4u, &topology) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(lib_memory_compare(&before, &topology, sizeof(before)) == 0);
    }
}

lib_i32 main(void)
{
    topology_values();
    endpoints();
    missing_endpoints();
    descriptor_contracts();
    return 0;
}
