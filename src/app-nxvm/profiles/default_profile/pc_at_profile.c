#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/at_assembly_interface.h"
#include "app-nxvm/profiles/default_profile/pc_at_profile_private.h"

static lib_i32 vm_profile_ibm_5170_memory_is_valid(lib_size memory_bytes);

static const vm_profile_default_pc_at_firmware_service
default_pc_at_firmware_services[] = {
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_IRQ0, 0x08u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_INT1A, 0x1au },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_IRQ1, 0x09u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_INT16, 0x16u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_DMA_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_IRQ6, 0x0eu },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT13, 0x13u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT40, 0x40u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13, 0x13u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIT_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST, 0u }
};

/* Production compiles one fixed AT composition; the test aggregate compiles both. */
#if !defined(VM_PROFILE_BUILD_5170)
static const vm_profile_default_pc_at_descriptor default_pc_at_descriptor = {
    "default-pc-at",
    1u,
    CORE_MACHINE_CPU_PROFILE_80386,
    X86_FPU_PROFILE_NONE,
    1u,
    { 1u, 0u, 0u, 0u, 0u, 0u },
    { { 0u, 0u, 0u, CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u },
        {{0}}, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE },
    { { 1u, 1u, 0u }, { 596591u, 4000000u, 0u }, { 1u, 1u, 0u },
        { 1u, 1u, 0u }, { 1u, 1u, 0u }, { 1u, 1u, 0u },
        { 1u, 1u, 0u } },
    { CORE_MACHINE_TIME_AXIS_UNQUALIFIED, 0u },
    { CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK },
    {{0u}},
    0u,
    0u,
    0u,
    0u,
    LIB_FALSE,
    0u,
    LIB_FALSE,
    0u,
    50000u,
    { 48u, 8u, 8u },
    { CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE },
    { { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u } },
    16u * 1024u * 1024u,
    LIB_TRUE,
    0x9fc0u,
    0x0fu,
    LIB_TRUE,
    LIB_FALSE,
    CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1,
    0u,
    LIB_TRUE,
    LIB_FALSE,
    LIB_FALSE,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC,
    LIB_FALSE,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x027fu, 0x40u, 0xf0u, 0x2fu, 0u, 0x80u },
    vm_at_port_leaves,
    sizeof(vm_at_port_leaves) / sizeof(vm_at_port_leaves[0]),
    vm_at_routes_with_aux,
    sizeof(vm_at_routes_with_aux) / sizeof(vm_at_routes_with_aux[0]),
    { .protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO, .irq = 14u,
        /* 86Box's generic ATA fallback completes through a controller timer,
         * rather than at command issue.  200 is this profile's frozen
         * Other-L2 service quantum in the existing Core elapsed axis; it is
         * not a universal ATA mechanical-time or wall-clock assertion. */
        .service = {200u, 200u},
        .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = LIB_TRUE }},
    default_pc_at_firmware_services,
    sizeof(default_pc_at_firmware_services) /
        sizeof(default_pc_at_firmware_services[0]),
    0x01u, 0x01u, {80u, 0u, 0u, 0u}, 0u, 0u, 0u
};

#endif
#if !defined(VM_PROFILE_BUILD_DEFAULT)
static const vm_profile_default_pc_at_descriptor ibm_5170_model_339_descriptor = {
    "ibm-5170-model-339",
    1u,
    CORE_MACHINE_CPU_PROFILE_80286,
    X86_FPU_PROFILE_NONE,
    1u,
    { 1u, 0u, 0u, 0u, 0u, 0u },
    { { 0u, 0u, 0u, CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u },
        {{0}}, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE },
    /* IBM 6280099, System Board 1-22 and 1-57: 8254 at 1.193182 MHz,
     * RTC at 32.768 kHz. The CGA character rate is the constrained v6.0
     * 86Box IBM-CGA reference rate 157500000/88 Hz. Ratios are to this
     * profile's nominal 8 MHz CPU source; they do not model availability,
     * waits, monitor output, or host elapsed time. */
    { { 3u, 8u, 0u }, { 596591u, 4000000u, 0u }, { 1u, 1u, 0u },
        { 64u, 15625u, 0u }, { 315u, 1408u, 0u }, { 1u, 1u, 0u },
        { 1u, 1u, 0u } },
    { CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 8000000u },
    { CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK },
    /* PCjs's Rev-3 model retains a 120-instruction keyboard IRQ phase for
     * this ROM.  The Board PIC owns the delayed IMR-release eligibility; this
     * immutable board value is Other-L2, never a BIOS-side exception. */
    {{0u, 120u}},
    /* IBM 6280099 Keyboard: default 500 ms delay and 10 cps typematic,
     * each with +/-20 percent tolerance. These are nominal Model-339 values. */
    4000000u,
    800000u,
    0u,
    1u,
    /* The 5170 8042 comes out of reset with reset deasserted and A20 enabled.
     * The Rev-3 ROM's early 60h/5Dh pair writes its command byte, not D1h's
     * output port; leaving the output port at the generic 01h aliases its
     * protected-mode 1 MiB probe over the GDT.  This is immutable board input
     * (PCjs's IBM AT 8042 model corroborates 03h), not a firmware shortcut. */
    LIB_TRUE,
    0x03u,
    /* 5170: 512 KiB planar RAM, color primary, no manufacturing loop,
     * keyboard unlocked.  The BIOS obtains these board straps with 8042 C0h. */
    LIB_TRUE,
    0xb0u,
    32768u,
    { 48u, 8u, 8u },
    { CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE },
    { { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u } },
    512u * 1024u,
    LIB_TRUE,
    0x7000u,
    0x0fu,
    LIB_TRUE,
    LIB_TRUE,
    /* IBM specifies counter 1 as the refresh-request source, but not the
     * readable port-61h waveform.  PCjs's Rev-3 model ties that observation
     * to the 8 MHz cycle axis and documents a 64-cycle half period required
     * by this ROM's two refresh POST checks.  Keep the physical PIT1-to-DMA
     * route in Board; this frozen Other-L2 board observation owns only bit 4. */
    CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE,
    64u,
    LIB_FALSE,
    LIB_TRUE,
    LIB_TRUE,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT,
    LIB_FALSE,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x0200u, 0x20u, 0x30u, 0x00u, 0u, 0x80u },
    vm_at_port_leaves,
    sizeof(vm_at_port_leaves) / sizeof(vm_at_port_leaves[0]),
    vm_at_routes_without_aux,
    sizeof(vm_at_routes_without_aux) / sizeof(vm_at_routes_without_aux[0]),
    { .protocol = CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506, .irq = 14u,
        .service = {16000u, 7840u},
        .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = LIB_FALSE, .clock_ticks_per_second = 8000000u }},
    default_pc_at_firmware_services,
    sizeof(default_pc_at_firmware_services) /
        sizeof(default_pc_at_firmware_services[0]),
    /* IBM 5170 Technical Reference: A: is a 96-TPI, 80-cylinder 1.2 MB
     * physical unit. A 360 KB disk changes only the mounted medium. */
    /* PCjs corroborates that the Rev-3 ROM reads this D/S/P-board endpoint
     * before enabling its 360 KB-in-1.2 MB compatibility path. It is an
     * explicit Other-L2 board capability, never a firmware exception. */
    0x01u, 0x01u, {80u, 0u, 0u, 0u}, 0u, 0x03f1u, 0x50u
};

#endif
static const lib_u32 ibm_5170_contract_ids[] = {1u};

#if !defined(VM_PROFILE_BUILD_5170)
const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(void)
{
    return &default_pc_at_descriptor;
}

#endif
#if !defined(VM_PROFILE_BUILD_DEFAULT)
const vm_profile_default_pc_at_descriptor *
vm_profile_ibm_5170_model_339_descriptor_get(void)
{
    return &ibm_5170_model_339_descriptor;
}

#endif
static lib_i32 vm_profile_default_pc_at_cpu_profile_is_valid(
    core_machine_cpu_profile profile)
{
    return profile == CORE_MACHINE_CPU_PROFILE_8086 ||
        profile == CORE_MACHINE_CPU_PROFILE_80186 ||
        profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        profile == CORE_MACHINE_CPU_PROFILE_80386;
}

static lib_i32 vm_profile_default_pc_at_fpu_profile_is_valid(
    x86_fpu_profile profile)
{
    return profile == X86_FPU_PROFILE_NONE ||
        profile == X86_FPU_PROFILE_8087 ||
        profile == X86_FPU_PROFILE_80287 ||
        profile == X86_FPU_PROFILE_80387;
}

static lib_i32 vm_profile_default_pc_at_fdc_bounce_is_valid(
    const vm_profile_default_pc_at_descriptor *descriptor)
{
    const lib_size physical = (lib_size)descriptor->fdc_bounce_segment << 4u;

    return descriptor->fdc_bounce_segment != 0u && physical <=
        descriptor->default_memory_bytes && 512u <=
        descriptor->default_memory_bytes - physical;
}

lib_i32 vm_profile_default_pc_at_cpu_contract_select(
    const vm_profile_default_pc_at_descriptor *descriptor,
    core_machine_cpu_profile requested_cpu,
    x86_fpu_profile requested_fpu,
    vm_profile_default_pc_at_cpu_contract *out_contract)
{
    if (descriptor == LIB_NULL || out_contract == LIB_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(descriptor)) return 0;
    if (requested_cpu == CORE_MACHINE_CPU_PROFILE_DEFAULT) {
        requested_cpu = descriptor->cpu_profile;
    }
    if (!vm_profile_default_pc_at_cpu_profile_is_valid(requested_cpu) ||
        !vm_profile_default_pc_at_fpu_profile_is_valid(requested_fpu)) return 0;
    *out_contract = (vm_profile_default_pc_at_cpu_contract) {
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

lib_i32 vm_profile_default_pc_at_core_config_materialize(
    const vm_profile_default_pc_at_descriptor *descriptor,
    const vm_profile_default_pc_at_cpu_contract *contract,
    core_machine_config *out_config,
    core_machine_controller_timing_rules *out_timing_rules)
{
    if (descriptor == LIB_NULL || contract == LIB_NULL || out_config == LIB_NULL ||
        out_timing_rules == LIB_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(descriptor)) return 0;
    *out_config = (core_machine_config) {
        .memory_bytes = descriptor->default_memory_bytes,
        .cpu_profile = contract->cpu_profile,
        .fpu_profile = contract->fpu_profile,
        .ticks_per_instruction = contract->ticks_per_instruction,
        .instruction_timing = contract->instruction_timing,
        .transaction_contract = contract->transaction_contract,
        .clock_plan = contract->clock_plan,
        .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_CASCADED,
        .dma_controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .time_axis = contract->time_axis,
        .l1_compatibility_policy = CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS,
        .pic_irq_timing = contract->pic_irq_timing,
        .kbc_aux_absent = descriptor->firmware_slot ==
            VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT,
        .kbc_typematic_initial_ticks = contract->kbc_typematic_initial_ticks,
        .kbc_typematic_repeat_ticks = contract->kbc_typematic_repeat_ticks,
        .kbc_command_response_ticks = contract->kbc_command_response_ticks,
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

lib_status vm_profile_default_pc_at_topology_materialize(
    const vm_profile_default_pc_at_descriptor *descriptor,
    const core_machine_controller_timing_rules *timing_rules,
    core_machine_plan_topology *out_topology)
{
    core_machine_plan_topology topology = {0};
    lib_u32 first_expansion_decode;

    if (descriptor == LIB_NULL || timing_rules == LIB_NULL || out_topology == LIB_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(descriptor)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (descriptor->unpopulated_extended_memory) {
        topology.absent_memory_count = 1u;
        topology.absent_memory[0] =
            (core_machine_absent_memory_config) { 0x00100000u, 0x00f00000u, 0xffu };
    }
    /* Below C0000h, an uninstalled video aperture is decoded as open bus, not
     * as missing Core memory.  A present EGA provider wins over this fallback;
     * CGA owns B8000h when selected.  IBM's 5170 BIOS deliberately probes
     * A0000h/B0000h/B8000h while it discovers the installed adapter. */
    first_expansion_decode = descriptor->ega_present ? 0x000a0000u :
        (descriptor->cga_vram_present ? 0x000b8000u : 0x000c0000u);
    if (descriptor->default_memory_bytes < first_expansion_decode) {
        topology.absent_memory[topology.absent_memory_count++] =
            (core_machine_absent_memory_config) { descriptor->default_memory_bytes,
                first_expansion_decode - descriptor->default_memory_bytes, 0xffu };
    }
    /* Every PC/AT descriptor owns system-board Port B. Parity is an optional
     * producer on that one port; generic Default PC/AT retains the port's
     * PIT1/PIT2 visibility without inventing parity memory. */
    topology.planar_parity_present = LIB_TRUE;
    topology.planar_parity = (core_machine_planar_parity_config) {
        CORE_MACHINE_PC_AT_PORT_B,
        descriptor->planar_parity_present ? descriptor->default_memory_bytes : 0u,
        descriptor->refresh_status_source,
        descriptor->refresh_status_toggle_ticks };
    topology.display = (core_machine_display_config) {
        .text_timing = descriptor->cga_text_timing,
        .cga_vram_present = descriptor->cga_vram_present,
        .ega_present = descriptor->ega_present,
        .ega_sequencer = descriptor->ega_sequencer,
        .ega_controllers = descriptor->ega_controllers
    };
    if (descriptor->monochrome_aperture_absent &&
        first_expansion_decode <= 0x000b0000u) {
        topology.absent_memory[topology.absent_memory_count++] =
            (core_machine_absent_memory_config) { 0x000b0000u, 0x00008000u, 0xffu };
    }
    /* PC/AT adapter ROM space is socket-decoded, not installed RAM.  An
     * external option/video ROM mapping wins over this fallback; without one,
     * firmware must observe the open bus and decide that no adapter ROM exists. */
    topology.absent_memory[topology.absent_memory_count++] =
        (core_machine_absent_memory_config) { 0x000c0000u, 0x00030000u, 0xffu };
    topology.rtc_cmos = (core_machine_rtc_cmos_config) {
        .nmi_mask_bit = 0x80u,
        .ticks_per_second = descriptor->rtc_ticks_per_second,
        .timing = timing_rules->rtc_clock ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ?
            (core_machine_rtc_timing_plan) {8u, 65u, CORE_MACHINE_RTC_TIMING_L3_SOURCE} :
            (core_machine_rtc_timing_plan) {0u, 0u, CORE_MACHINE_RTC_TIMING_L2_RATIO},
        .defaults = {
            { CORE_MACHINE_RTC_TYPE_DISK_FLOPPY, descriptor->cmos.floppy_type },
            { CORE_MACHINE_RTC_TYPE_DISK_FIXED, descriptor->cmos.fixed_disk_type },
            { CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0,
                descriptor->cmos.fixed_disk_type_extended_0 },
            { CORE_MACHINE_RTC_EQUIPMENT, descriptor->cmos.equipment },
            { CORE_MACHINE_RTC_BASEMEM_LSB,
                (lib_u8)descriptor->cmos.base_memory_kib },
            { CORE_MACHINE_RTC_BASEMEM_MSB,
                (lib_u8)(descriptor->cmos.base_memory_kib >> 8) }
        },
        .default_count = CORE_MACHINE_RTC_DEFAULT_COUNT,
        .derive_configuration_checksum = LIB_TRUE
    };
    if (vm_at_topology_materialize(descriptor->port_leaves, descriptor->port_leaf_count,
            descriptor->routes, descriptor->route_count, &topology) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_topology = topology;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_default_pc_at_values_create(
    const vm_profile_default_pc_at_descriptor *descriptor,
    core_machine_cpu_profile cpu_profile, x86_fpu_profile fpu_profile,
    vm_profile_contract_values *out_values)
{
    vm_profile_default_pc_at_cpu_contract contract;
    vm_profile_contract_core_input core = {.id = ibm_5170_contract_ids[0]};
    lib_u32 enabled = vm_profile_default_pc_at_enabled_devices(descriptor);

    if (out_values == LIB_NULL ||
        !vm_profile_default_pc_at_cpu_contract_select(descriptor,
            cpu_profile, fpu_profile, &contract) ||
        !vm_profile_default_pc_at_core_config_materialize(descriptor, &contract,
            &core.configuration, &core.controller_timing_rules))
        return LIB_STATUS_INVALID_ARGUMENT;
    return vm_at_contract_materialize(&core, descriptor->port_leaves, descriptor->port_leaf_count,
        descriptor->routes, descriptor->route_count, enabled, descriptor->cga_vram_present, out_values);
}

static lib_status vm_profile_default_pc_at_snapshot_copy(
    vm_profile_default_pc_at_plan_snapshot *out_profile,
    const vm_profile_default_pc_at_descriptor *descriptor, const char *identity,
    lib_u8 floppy_cmos_type)
{
    if (out_profile == LIB_NULL || descriptor == LIB_NULL || identity == LIB_NULL ||
        descriptor->port_leaf_count > VM_PROFILE_DEFAULT_PC_AT_PLAN_PORT_LEAF_CAPACITY ||
        descriptor->route_count > VM_PROFILE_DEFAULT_PC_AT_PLAN_ROUTE_CAPACITY ||
        descriptor->firmware_service_count >
            VM_PROFILE_DEFAULT_PC_AT_PLAN_FIRMWARE_SERVICE_CAPACITY) {
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
    return vm_profile_default_pc_at_topology_materialize(&out_profile->descriptor,
        &out_profile->values.core.controller_timing_rules,
        &out_profile->topology);
}

#if !defined(VM_PROFILE_BUILD_DEFAULT)
lib_status vm_profile_ibm_5170_values_create(lib_size memory_bytes,
    vm_profile_contract_values *out_values)
{
    const vm_profile_default_pc_at_descriptor *descriptor =
        vm_profile_ibm_5170_model_339_descriptor_get();
    vm_profile_contract_values values = {0};
    const vm_profile_contract_catalog catalog = { ibm_5170_contract_ids,
        sizeof(ibm_5170_contract_ids) / sizeof(ibm_5170_contract_ids[0]) };

    if (out_values == LIB_NULL || !vm_profile_ibm_5170_memory_is_valid(memory_bytes) ||
        vm_profile_default_pc_at_values_create(descriptor, descriptor->cpu_profile,
            descriptor->fpu_profile, &values) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (memory_bytes != 0u) {
        values.core.configuration.memory_bytes = memory_bytes;
        values.allowed_session_options = VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY;
    }
    values.firmware_policy = VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BUILTIN;
    values.media_policy = VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION;
    if (vm_profile_contract_validate(&values, &catalog, memory_bytes != 0u ?
            VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY : 0u) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_values = values;
    return LIB_STATUS_OK;
}

#endif
static lib_i32 vm_profile_ibm_5170_memory_is_valid(lib_size memory_bytes)
{
    if (memory_bytes == 0u || memory_bytes == 512u * 1024u ||
        memory_bytes == 640u * 1024u) return 1;
    return memory_bytes >= 1536u * 1024u && memory_bytes <= 3u * 1024u * 1024u &&
        (memory_bytes - 1024u * 1024u) % (512u * 1024u) == 0u;
}

#if !defined(VM_PROFILE_BUILD_DEFAULT)
lib_status vm_profile_ibm_5170_plan_create_memory(lib_size memory_bytes,
    vm_profile_default_pc_at_plan_snapshot *out_profile)
{
    const vm_profile_default_pc_at_descriptor *source =
        vm_profile_ibm_5170_model_339_descriptor_get();
    vm_profile_default_pc_at_descriptor descriptor;

    if (out_profile == LIB_NULL || !vm_profile_ibm_5170_memory_is_valid(memory_bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    descriptor = *source;
    if (memory_bytes != 0u) descriptor.default_memory_bytes = memory_bytes;
    if (memory_bytes == 640u * 1024u || memory_bytes >= 1536u * 1024u) {
        descriptor.cmos.base_memory_kib = 0x0280u;
    }
    if (memory_bytes >= 1536u * 1024u) {
        descriptor.unpopulated_extended_memory = LIB_FALSE;
    }
    lib_memory_set(out_profile, 0, sizeof(*out_profile));
    if (vm_profile_ibm_5170_values_create(memory_bytes, &out_profile->values) !=
        LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return vm_profile_default_pc_at_snapshot_copy(out_profile, &descriptor, "pc-at-5170",
        descriptor.cmos.floppy_type);
}

lib_status vm_profile_ibm_5170_plan_create(
    vm_profile_default_pc_at_plan_snapshot *out_profile)
{
    return vm_profile_ibm_5170_plan_create_memory(0u, out_profile);
}

#endif
#if !defined(VM_PROFILE_BUILD_5170)
static lib_status vm_profile_default_at_request_select(
    const vm_profile_default_at_request *request,
    core_machine_cpu_profile *out_cpu, x86_fpu_profile *out_fpu,
    lib_size *out_memory)
{
    const vm_profile_default_pc_at_descriptor *descriptor =
        vm_profile_default_pc_at_descriptor_get();

    if (request == LIB_NULL || out_cpu == LIB_NULL || out_fpu == LIB_NULL ||
        out_memory == LIB_NULL ||
        (request->requested_options & ~(VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
            VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
            VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY)) != 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_cpu = descriptor->cpu_profile;
    *out_fpu = descriptor->fpu_profile;
    *out_memory = descriptor->default_memory_bytes;
    if ((request->requested_options & VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU) != 0u) {
        *out_cpu = request->cpu_profile;
        *out_fpu = request->fpu_profile;
    }
    if ((request->requested_options & VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY) != 0u) {
        if (request->memory_bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
        *out_memory = request->memory_bytes;
    }
    if ((request->requested_options & VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY) != 0u &&
        request->floppy_cmos_type != 0x10u && request->floppy_cmos_type != 0x20u &&
        request->floppy_cmos_type != 0x30u && request->floppy_cmos_type != 0x40u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

static lib_status vm_profile_default_at_values_create(
    const vm_profile_default_at_request *request,
    vm_profile_contract_values *out_values)
{
    core_machine_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    lib_size memory_bytes;
    vm_profile_contract_values values = {0};
    const vm_profile_contract_catalog catalog = { ibm_5170_contract_ids,
        sizeof(ibm_5170_contract_ids) / sizeof(ibm_5170_contract_ids[0]) };

    if (out_values == LIB_NULL ||
        vm_profile_default_at_request_select(request, &cpu_profile, &fpu_profile,
            &memory_bytes) != LIB_STATUS_OK ||
        vm_profile_default_pc_at_values_create(vm_profile_default_pc_at_descriptor_get(),
            cpu_profile, fpu_profile, &values) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    values.core.configuration.memory_bytes = memory_bytes;
    values.firmware_policy = VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BUILTIN;
    values.media_policy = VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION;
    values.allowed_session_options =
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY;
    if (vm_profile_contract_validate(&values, &catalog, request->requested_options) !=
        LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    *out_values = values;
    return LIB_STATUS_OK;
}

lib_status vm_profile_default_at_plan_create(
    const vm_profile_default_at_request *request,
    vm_profile_default_pc_at_plan_snapshot *out_profile)
{
    if (out_profile == LIB_NULL || request == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_set(out_profile, 0, sizeof(*out_profile));
    if (vm_profile_default_at_values_create(request, &out_profile->values) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return vm_profile_default_pc_at_snapshot_copy(out_profile,
        vm_profile_default_pc_at_descriptor_get(), "default-at",
        (request->requested_options & VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY) != 0u ?
            request->floppy_cmos_type :
            vm_profile_default_pc_at_descriptor_get()->cmos.floppy_type);
}

#endif
lib_u32 vm_profile_default_pc_at_enabled_devices(
    const vm_profile_default_pc_at_descriptor *descriptor)
{
    lib_u32 enabled = VM_AT_DEVICE_MASK_ALL;

    if (descriptor == LIB_NULL) return 0u;
    if (!descriptor->hdc_present) enabled &= ~(1u << VM_AT_DEVICE_HDC);
    if (!descriptor->ega_present) enabled &= ~((1u << VM_AT_DEVICE_VADP_ATTRIBUTE) |
        (1u << VM_AT_DEVICE_VADP_SEQUENCER) | (1u << VM_AT_DEVICE_VADP_GRAPHICS));
    return enabled;
}

lib_i32 vm_profile_default_pc_at_descriptor_is_valid(
    const vm_profile_default_pc_at_descriptor *descriptor)
{
    const vm_at_route *expected_routes;
    lib_size expected_route_count;
    lib_size index;

    if (descriptor == LIB_NULL || !vm_profile_default_pc_at_fdc_bounce_is_valid(descriptor) ||
        descriptor->port_leaves == LIB_NULL ||
        descriptor->routes == LIB_NULL || descriptor->port_leaf_count !=
        sizeof(vm_at_port_leaves) / sizeof(vm_at_port_leaves[0]) ||
        descriptor->route_count == 0u) return 0;
    expected_routes = descriptor->firmware_slot ==
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT ?
        vm_at_routes_without_aux : vm_at_routes_with_aux;
    expected_route_count = descriptor->firmware_slot ==
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT ?
        sizeof(vm_at_routes_without_aux) / sizeof(vm_at_routes_without_aux[0]) :
        sizeof(vm_at_routes_with_aux) / sizeof(vm_at_routes_with_aux[0]);
    if (descriptor->route_count != expected_route_count) return 0;
    for (index = 0u; index < descriptor->port_leaf_count; ++index) {
        if (lib_memory_compare(&descriptor->port_leaves[index],
                &vm_at_port_leaves[index],
                sizeof(vm_at_port_leaves[index])) != 0) return 0;
    }
    for (index = 0u; index < descriptor->route_count; ++index) {
        if (lib_memory_compare(&descriptor->routes[index], &expected_routes[index],
                sizeof(expected_routes[index])) != 0) return 0;
    }
    if (descriptor->firmware_slot ==
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT) {
        if (descriptor->cpu_profile != CORE_MACHINE_CPU_PROFILE_80286) return 0;
        return descriptor->time_axis.kind == CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL &&
            descriptor->time_axis.ticks_per_second == 8000000u &&
            vm_profile_ibm_5170_memory_is_valid(descriptor->default_memory_bytes) &&
            ((descriptor->default_memory_bytes == 512u * 1024u &&
                descriptor->unpopulated_extended_memory &&
                descriptor->cmos.base_memory_kib == 0x0200u) ||
                (descriptor->default_memory_bytes != 512u * 1024u &&
                    !descriptor->unpopulated_extended_memory &&
                    descriptor->cmos.base_memory_kib == 0x0280u)) &&
            descriptor->fdc_bounce_segment == 0x7000u &&
            descriptor->fdc_installed_mask == 0x01u &&
            descriptor->fdc_double_sided_mask == 0x01u &&
            descriptor->fdc_cylinder_count[0u] == 80u &&
            descriptor->fdc_track_zero_active_low_mask == 0u &&
            descriptor->fdc_diagnostic_port == 0x03f1u &&
            descriptor->fdc_diagnostic_read_value == 0x50u &&
            descriptor->hdc_present && descriptor->planar_parity_present &&
            descriptor->kbc_input_port_configured && descriptor->kbc_input_port == 0xb0u &&
            descriptor->refresh_status_source ==
                CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE &&
            descriptor->refresh_status_toggle_ticks == 64u &&
            !descriptor->ega_present && descriptor->cga_vram_present &&
            descriptor->monochrome_aperture_absent &&
            descriptor->firmware_slot ==
                VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT &&
            !descriptor->diskette_drive_a_field_upgrade &&
            descriptor->cmos.floppy_type == 0x20u &&
            descriptor->cmos.fixed_disk_type == 0x30u &&
            descriptor->cmos.fixed_disk_type_extended_0 == 0u &&
            descriptor->hdc.protocol == CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506 &&
            descriptor->hdc.bus.task_file.data_port == 0x01f0u &&
            descriptor->hdc.bus.task_file.error_features_port == 0x01f1u &&
            descriptor->hdc.bus.task_file.sector_count_port == 0x01f2u &&
            descriptor->hdc.bus.task_file.sector_number_port == 0x01f3u &&
            descriptor->hdc.bus.task_file.cylinder_low_port == 0x01f4u &&
            descriptor->hdc.bus.task_file.cylinder_high_port == 0x01f5u &&
            descriptor->hdc.bus.task_file.drive_head_port == 0x01f6u &&
            descriptor->hdc.bus.task_file.status_command_port == 0x01f7u &&
            descriptor->hdc.bus.task_file.alternate_status_device_control_port == 0x03f6u &&
            descriptor->hdc.irq == 14u && !descriptor->hdc.bus.task_file.lba28_supported &&
            descriptor->hdc.bus.task_file.clock_ticks_per_second == 8000000u &&
            descriptor->firmware_services != LIB_NULL &&
            descriptor->firmware_service_count ==
                sizeof(default_pc_at_firmware_services) /
                    sizeof(default_pc_at_firmware_services[0]) &&
            lib_memory_compare(descriptor->firmware_services,
                default_pc_at_firmware_services,
                sizeof(default_pc_at_firmware_services)) == 0;
    }
    return descriptor->firmware_slot == VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC &&
        vm_profile_default_pc_at_cpu_profile_is_valid(descriptor->cpu_profile) &&
        vm_profile_default_pc_at_fpu_profile_is_valid(descriptor->fpu_profile) &&
        descriptor->hdc_present && !descriptor->planar_parity_present &&
        descriptor->fdc_installed_mask == 0x01u &&
        descriptor->fdc_double_sided_mask == 0x01u &&
        descriptor->fdc_cylinder_count[0u] == 80u &&
        descriptor->fdc_track_zero_active_low_mask == 0u &&
        descriptor->fdc_diagnostic_port == 0u &&
        descriptor->fdc_diagnostic_read_value == 0u &&
        !descriptor->kbc_input_port_configured && descriptor->kbc_input_port == 0u &&
        descriptor->refresh_status_source ==
            CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1 &&
        descriptor->refresh_status_toggle_ticks == 0u &&
        descriptor->time_axis.kind == CORE_MACHINE_TIME_AXIS_UNQUALIFIED &&
        descriptor->time_axis.ticks_per_second == 0u &&
        descriptor->ega_present && !descriptor->cga_vram_present &&
        descriptor->firmware_slot == VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC &&
        !descriptor->diskette_drive_a_field_upgrade &&
        descriptor->hdc.protocol == CORE_MACHINE_HDC_PROTOCOL_ATA_PIO &&
        descriptor->hdc.bus.task_file.data_port == 0x01f0u &&
        descriptor->hdc.bus.task_file.error_features_port == 0x01f1u &&
        descriptor->hdc.bus.task_file.sector_count_port == 0x01f2u &&
        descriptor->hdc.bus.task_file.sector_number_port == 0x01f3u &&
        descriptor->hdc.bus.task_file.cylinder_low_port == 0x01f4u &&
        descriptor->hdc.bus.task_file.cylinder_high_port == 0x01f5u &&
        descriptor->hdc.bus.task_file.drive_head_port == 0x01f6u &&
        descriptor->hdc.bus.task_file.status_command_port == 0x01f7u &&
        descriptor->hdc.bus.task_file.alternate_status_device_control_port == 0x03f6u &&
        descriptor->hdc.irq == 14u && descriptor->hdc.bus.task_file.lba28_supported;
}
