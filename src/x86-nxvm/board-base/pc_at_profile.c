#include "x86-nxvm/board-base/pc_at_profile_interface.h"
#include "x86-nxvm/board-base/at_assembly_interface.h"

const x86_nxvm_board_pc_at_firmware_service
x86_nxvm_board_pc_at_firmware_services[13u] = {
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_CMOS_POST, 0u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_TIMER_IRQ0, 0x08u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_TIMER_INT1A, 0x1au },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_KEYBOARD_IRQ1, 0x09u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_KEYBOARD_INT16, 0x16u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_DMA_POST, 0u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_FDC_POST, 0u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_FDC_IRQ6, 0x0eu },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_FDC_INT13, 0x13u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_FDC_INT40, 0x40u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_HDC_INT13, 0x13u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_PIT_POST, 0u },
    { X86_NXVM_BOARD_PC_AT_FIRMWARE_PIC_POST, 0u }
};

lib_i32 x86_nxvm_board_pc_at_cpu_profile_is_valid(
    x86_core_cpu_profile profile)
{
    return profile == X86_CORE_CPU_PROFILE_8086 ||
        profile == X86_CORE_CPU_PROFILE_80186 ||
        profile == X86_CORE_CPU_PROFILE_80286 ||
        profile == X86_CORE_CPU_PROFILE_80386;
}

lib_i32 x86_nxvm_board_pc_at_fpu_profile_is_valid(
    x86_fpu_profile profile)
{
    return profile == X86_FPU_PROFILE_NONE ||
        profile == X86_FPU_PROFILE_8087 ||
        profile == X86_FPU_PROFILE_80287 ||
        profile == X86_FPU_PROFILE_80387;
}

static lib_i32 x86_nxvm_board_pc_at_fdc_bounce_is_valid(
    const x86_nxvm_board_pc_at_descriptor *descriptor)
{
    const lib_size physical = (lib_size)descriptor->fdc_bounce_segment << 4u;

    return descriptor->fdc_bounce_segment != 0u && physical <=
        descriptor->default_memory_bytes && 512u <=
        descriptor->default_memory_bytes - physical;
}

lib_i32 x86_nxvm_board_pc_at_cpu_contract_select(
    const x86_nxvm_board_pc_at_descriptor *descriptor,
    x86_core_cpu_profile requested_cpu,
    x86_fpu_profile requested_fpu,
    x86_nxvm_board_pc_at_cpu_contract *out_contract)
{
    if (descriptor == LIB_NULL || out_contract == LIB_NULL ||
        !x86_nxvm_board_pc_at_descriptor_is_valid(descriptor)) return 0;
    if (requested_cpu == X86_CORE_CPU_PROFILE_DEFAULT) {
        requested_cpu = descriptor->cpu_profile;
    }
    if (!x86_nxvm_board_pc_at_cpu_profile_is_valid(requested_cpu) ||
        !x86_nxvm_board_pc_at_fpu_profile_is_valid(requested_fpu)) return 0;
    *out_contract = (x86_nxvm_board_pc_at_cpu_contract) {
        requested_cpu,
        requested_fpu,
        descriptor->ticks_per_instruction,
        descriptor->instruction_timing,
        descriptor->transaction_contract,
        descriptor->clock_plan,
        descriptor->time_axis,
        descriptor->controller_timing_rules,
        descriptor->pic_irq_timing,
        descriptor->kbc_typematic_initial_ticks,
        descriptor->kbc_typematic_repeat_ticks,
        descriptor->kbc_command_response_ticks,
        descriptor->kbc_command_response_status_polls
    };
    return 1;
}

lib_i32 x86_nxvm_board_pc_at_core_config_materialize(
    const x86_nxvm_board_pc_at_descriptor *descriptor,
    const x86_nxvm_board_pc_at_cpu_contract *contract,
    x86_nxvm_board_config *out_config,
    x86_nxvm_board_controller_timing_rules *out_timing_rules)
{
    if (descriptor == LIB_NULL || contract == LIB_NULL || out_config == LIB_NULL ||
        out_timing_rules == LIB_NULL ||
        !x86_nxvm_board_pc_at_descriptor_is_valid(descriptor)) return 0;
    *out_config = (x86_nxvm_board_config) {
        .memory_bytes = descriptor->default_memory_bytes,
        .cpu_profile = contract->cpu_profile,
        .fpu_profile = contract->fpu_profile,
        .ticks_per_instruction = contract->ticks_per_instruction,
        .instruction_timing = contract->instruction_timing,
        .transaction_contract = contract->transaction_contract,
        .clock_plan = contract->clock_plan,
        .pic_topology = X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED,
        .dma_controller_count = X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
        .time_axis = contract->time_axis,
        .l1_compatibility_policy = X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS,
        .pic_irq_timing = contract->pic_irq_timing,
        .kbc_aux_absent = descriptor->kbc_aux_absent,
        .kbc_typematic_initial_ticks = contract->kbc_typematic_initial_ticks,
        .kbc_typematic_repeat_ticks = contract->kbc_typematic_repeat_ticks,
        .kbc_command_response_ticks = contract->kbc_command_response_ticks,
        .kbc_output_pulse_ticks = descriptor->kbc_output_pulse_ticks,
        .kbc_command_response_status_polls =
            contract->kbc_command_response_status_polls,
        .kbc_reset_output_port_configured =
            descriptor->kbc_reset_output_port_configured,
        .kbc_reset_output_port = descriptor->kbc_reset_output_port,
        .kbc_input_port_configured = descriptor->kbc_input_port_configured,
        .kbc_input_port = descriptor->kbc_input_port
    };
    *out_timing_rules = contract->controller_timing_rules;
    return 1;
}

lib_status x86_nxvm_board_pc_at_topology_materialize(
    const x86_nxvm_board_pc_at_descriptor *descriptor,
    const x86_nxvm_board_controller_timing_rules *timing_rules,
    x86_nxvm_board_plan_topology *out_topology)
{
    x86_nxvm_board_plan_topology topology = {0};
    lib_u32 first_expansion_decode;

    if (descriptor == LIB_NULL || timing_rules == LIB_NULL || out_topology == LIB_NULL ||
        !x86_nxvm_board_pc_at_descriptor_is_valid(descriptor)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (descriptor->unpopulated_extended_memory) {
        topology.absent_memory_count = 1u;
        topology.absent_memory[0] =
            (x86_nxvm_board_absent_memory_config) { 0x00100000u, 0x00f00000u, 0xffu };
    }
    /* Below C0000h, an uninstalled video aperture is decoded as open bus, not
     * as missing Core memory.  A present EGA provider wins over this fallback;
     * CGA owns B8000h when selected.  IBM's 5170 BIOS deliberately probes
     * A0000h/B0000h/B8000h while it discovers the installed adapter. */
    first_expansion_decode = descriptor->ega_present ? 0x000a0000u :
        (descriptor->cga_vram_present ? 0x000b8000u : 0x000c0000u);
    if (descriptor->default_memory_bytes < first_expansion_decode) {
        topology.absent_memory[topology.absent_memory_count++] =
            (x86_nxvm_board_absent_memory_config) { descriptor->default_memory_bytes,
                first_expansion_decode - descriptor->default_memory_bytes, 0xffu };
    }
    /* Every PC/AT descriptor owns system-board Port B. Parity is an optional
     * producer on that one port; generic Default PC/AT retains the port's
     * PIT1/PIT2 visibility without inventing parity memory. */
    topology.planar_parity_present = LIB_TRUE;
    topology.planar_parity = (x86_nxvm_board_planar_parity_config) {
        X86_NXVM_BOARD_PC_AT_PORT_B,
        descriptor->planar_parity_present ? descriptor->default_memory_bytes : 0u,
        descriptor->refresh_status_source,
        descriptor->refresh_status_toggle_ticks };
    topology.display = (x86_nxvm_board_display_config) {
        .text_timing = descriptor->cga_text_timing,
        .cga_vram_present = descriptor->cga_vram_present,
        .ega_present = descriptor->ega_present,
        .vertical_irq = descriptor->ega_present ? 9u : 0u,
        .ega_sequencer = descriptor->ega_sequencer,
        .ega_controllers = descriptor->ega_controllers
    };
    if (descriptor->monochrome_aperture_absent &&
        first_expansion_decode <= 0x000b0000u) {
        topology.absent_memory[topology.absent_memory_count++] =
            (x86_nxvm_board_absent_memory_config) { 0x000b0000u, 0x00008000u, 0xffu };
    }
    /* PC/AT adapter ROM space is socket-decoded, not installed RAM.  An
     * external option/video ROM mapping wins over this fallback; without one,
     * firmware must observe the open bus and decide that no adapter ROM exists. */
    topology.absent_memory[topology.absent_memory_count++] =
        (x86_nxvm_board_absent_memory_config) { 0x000c0000u, 0x00030000u, 0xffu };
    topology.rtc_cmos = (x86_nxvm_board_rtc_cmos_config) {
        .nmi_mask_bit = 0x80u,
        .ticks_per_second = descriptor->rtc_ticks_per_second,
        .timing = timing_rules->rtc_clock ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ?
            (x86_nxvm_board_rtc_timing_plan) {8u, 65u, X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE} :
            (x86_nxvm_board_rtc_timing_plan) {0u, 0u, X86_NXVM_BOARD_RTC_TIMING_L2_RATIO},
        .defaults = {
            { X86_NXVM_BOARD_RTC_TYPE_DISK_FLOPPY, descriptor->cmos.floppy_type },
            { X86_NXVM_BOARD_RTC_TYPE_DISK_FIXED, descriptor->cmos.fixed_disk_type },
            { X86_NXVM_BOARD_RTC_TYPE_DISK_FIXED_EXTENDED_0,
                descriptor->cmos.fixed_disk_type_extended_0 },
            { X86_NXVM_BOARD_RTC_EQUIPMENT, descriptor->cmos.equipment },
            { X86_NXVM_BOARD_RTC_BASEMEM_LSB,
                (lib_u8)descriptor->cmos.base_memory_kib },
            { X86_NXVM_BOARD_RTC_BASEMEM_MSB,
                (lib_u8)(descriptor->cmos.base_memory_kib >> 8) }
        },
        .default_count = X86_NXVM_BOARD_RTC_DEFAULT_COUNT,
        .derive_configuration_checksum = LIB_TRUE
    };
    if (x86_nxvm_board_at_topology_materialize(descriptor->port_leaves, descriptor->port_leaf_count,
            descriptor->routes, descriptor->route_count, &topology) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_topology = topology;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_pc_at_values_create(
    const x86_nxvm_board_pc_at_descriptor *descriptor,
    x86_core_cpu_profile cpu_profile, x86_fpu_profile fpu_profile,
    x86_nxvm_board_contract_values *out_values)
{
    x86_nxvm_board_pc_at_cpu_contract contract;
    x86_nxvm_board_contract_core_input core = {.id = 1u};
    lib_u32 enabled = x86_nxvm_board_pc_at_enabled_devices(descriptor);

    if (out_values == LIB_NULL ||
        !x86_nxvm_board_pc_at_cpu_contract_select(descriptor,
            cpu_profile, fpu_profile, &contract) ||
        !x86_nxvm_board_pc_at_core_config_materialize(descriptor, &contract,
            &core.configuration, &core.controller_timing_rules))
        return LIB_STATUS_INVALID_ARGUMENT;
    return x86_nxvm_board_at_contract_materialize(&core, descriptor->port_leaves, descriptor->port_leaf_count,
        descriptor->routes, descriptor->route_count, enabled, descriptor->cga_vram_present, out_values);
}

lib_status x86_nxvm_board_pc_at_snapshot_copy(
    x86_nxvm_board_pc_at_plan_snapshot *out_profile,
    const x86_nxvm_board_pc_at_descriptor *descriptor, const char *identity,
    lib_u8 floppy_cmos_type)
{
    if (out_profile == LIB_NULL || descriptor == LIB_NULL || identity == LIB_NULL ||
        descriptor->port_leaf_count > X86_NXVM_BOARD_PC_AT_PLAN_PORT_LEAF_CAPACITY ||
        descriptor->route_count > X86_NXVM_BOARD_PC_AT_PLAN_ROUTE_CAPACITY ||
        descriptor->firmware_service_count >
            X86_NXVM_BOARD_PC_AT_PLAN_FIRMWARE_SERVICE_CAPACITY) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    out_profile->descriptor = *descriptor;
    lib_memory_copy(out_profile->port_leaves, descriptor->port_leaves,
        descriptor->port_leaf_count * sizeof(out_profile->port_leaves[0]));
    lib_memory_copy(out_profile->routes, descriptor->routes,
        descriptor->route_count * sizeof(out_profile->routes[0]));
    lib_memory_copy(out_profile->firmware_services, descriptor->firmware_services,
        descriptor->firmware_service_count * sizeof(out_profile->firmware_services[0]));
    out_profile->descriptor.identity = identity;
    out_profile->descriptor.port_leaves = out_profile->port_leaves;
    out_profile->descriptor.routes = out_profile->routes;
    out_profile->descriptor.firmware_services = out_profile->firmware_services;
    out_profile->descriptor.cpu_profile =
        out_profile->values.core.configuration.cpu_profile;
    out_profile->descriptor.fpu_profile =
        out_profile->values.core.configuration.fpu_profile;
    out_profile->descriptor.default_memory_bytes =
        out_profile->values.core.configuration.memory_bytes;
    out_profile->descriptor.cmos.floppy_type = floppy_cmos_type;
    return x86_nxvm_board_pc_at_topology_materialize(&out_profile->descriptor,
        &out_profile->values.core.controller_timing_rules,
        &out_profile->topology);
}

lib_u32 x86_nxvm_board_pc_at_enabled_devices(
    const x86_nxvm_board_pc_at_descriptor *descriptor)
{
    lib_u32 enabled = X86_NXVM_BOARD_AT_DEVICE_MASK_ALL;

    if (descriptor == LIB_NULL) return 0u;
    if (!descriptor->hdc_present) enabled &= ~(1u << X86_NXVM_BOARD_AT_DEVICE_HDC);
    if (!descriptor->ega_present) enabled &= ~((1u << X86_NXVM_BOARD_AT_DEVICE_VADP_ATTRIBUTE) |
        (1u << X86_NXVM_BOARD_AT_DEVICE_VADP_SEQUENCER) | (1u << X86_NXVM_BOARD_AT_DEVICE_VADP_GRAPHICS));
    return enabled;
}

lib_i32 x86_nxvm_board_pc_at_descriptor_is_valid(
    const x86_nxvm_board_pc_at_descriptor *descriptor)
{
    const x86_nxvm_board_at_route *expected_routes;
    lib_size expected_route_count;
    lib_size index;

    if (descriptor == LIB_NULL || !x86_nxvm_board_pc_at_fdc_bounce_is_valid(descriptor) ||
        descriptor->port_leaves == LIB_NULL ||
        descriptor->routes == LIB_NULL || descriptor->port_leaf_count !=
        sizeof(x86_nxvm_board_at_port_leaves) / sizeof(x86_nxvm_board_at_port_leaves[0]) ||
        descriptor->route_count == 0u) return 0;
    expected_routes = descriptor->kbc_aux_absent ?
        x86_nxvm_board_at_routes_without_aux : x86_nxvm_board_at_routes_with_aux;
    expected_route_count = descriptor->kbc_aux_absent ?
        sizeof(x86_nxvm_board_at_routes_without_aux) / sizeof(x86_nxvm_board_at_routes_without_aux[0]) :
        sizeof(x86_nxvm_board_at_routes_with_aux) / sizeof(x86_nxvm_board_at_routes_with_aux[0]);
    if (descriptor->route_count != expected_route_count) return 0;
    for (index = 0u; index < descriptor->port_leaf_count; ++index) {
        if (lib_memory_compare(&descriptor->port_leaves[index],
                &x86_nxvm_board_at_port_leaves[index],
                sizeof(x86_nxvm_board_at_port_leaves[index])) != 0) return 0;
    }
    for (index = 0u; index < descriptor->route_count; ++index) {
        if (lib_memory_compare(&descriptor->routes[index], &expected_routes[index],
                sizeof(expected_routes[index])) != 0) return 0;
    }
    return descriptor->validate != LIB_NULL && descriptor->validate(descriptor);
}
