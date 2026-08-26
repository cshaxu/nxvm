#include "vm/profile/default_profile/pc_at_profile_private.h"

static const vm_profile_default_pc_at_port_leaf default_pc_at_port_leaves[] = {
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x0020u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x0021u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x00a0u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x00a1u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0040u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0041u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0042u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0043u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0000u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0001u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0002u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0003u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0004u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0005u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0006u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0007u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0008u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0009u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000au, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000bu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000cu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000du, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000eu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000fu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0081u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0082u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0083u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0087u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0089u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x008au, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x008bu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x008fu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c0u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c2u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c4u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c6u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c8u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00cau, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00ccu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00ceu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d0u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d2u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d4u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d6u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d8u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00dau, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00dcu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00deu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC, 0x0060u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC, 0x0064u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_MEMORY_CONTROL, 0x0092u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 0x03c0u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 0x03c1u, TYPE_TRUE, TYPE_FALSE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 0x03c4u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 0x03c5u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 0x03ceu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 0x03cfu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d4u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d5u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d8u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d9u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03dau, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03c2u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0x0070u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0x0071u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f2u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f4u, TYPE_TRUE, TYPE_FALSE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f5u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f7u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f0u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f1u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f2u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f3u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f4u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f5u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f6u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f7u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x03f6u, TYPE_TRUE, TYPE_TRUE }
};

static const vm_profile_default_pc_at_route default_pc_at_routes[] = {
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_PIT_IRQ0, 0u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_KEYBOARD_IRQ1, 1u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_AUX_IRQ12, 12u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_CMOS_IRQ8, 8u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2, 6u, 2u }
};

static const vm_profile_default_pc_at_firmware_service
default_pc_at_firmware_services[] = {
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10, 0x10u },
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

static const vm_profile_default_pc_at_firmware_service
ibm_5170_model_339_firmware_services[] = {
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10, 0x10u },
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
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIT_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST, 0u }
};

static const vm_profile_default_pc_at_descriptor default_pc_at_descriptor = {
    "default-pc-at",
    1u,
    CORE_MACHINE_CPU_PROFILE_80386,
    CORE_MACHINE_FPU_PROFILE_NONE,
    1u,
    { 1u, 0u, 0u, 0u, 0u, 0u },
    { { 0u, 0u, 0u, CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u },
        {{0}}, 0u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE },
    { { 1u, 1u, 0u }, { 1u, 4u, 0u }, { 1u, 1u, 0u }, { 1u, 1u, 0u },
        { 1u, 1u, 0u }, { 1u, 1u, 0u } },
    { CORE_MACHINE_TIME_AXIS_UNQUALIFIED, 0u },
    { CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK },
    0u,
    0u,
    0u,
    50000u,
    { 48u, 8u, 8u },
    { CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_TRUE },
    { { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u } },
    16u * 1024u * 1024u,
    TYPE_FALSE,
    0x9fc0u,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_TRUE,
    TYPE_FALSE,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC,
    TYPE_FALSE,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x027fu, 0x40u, 0xf0u, 0x2fu, 0u, 0x80u },
    default_pc_at_port_leaves,
    sizeof(default_pc_at_port_leaves) / sizeof(default_pc_at_port_leaves[0]),
    default_pc_at_routes,
    sizeof(default_pc_at_routes) / sizeof(default_pc_at_routes[0]),
    { 0x01f0u, 0x01f1u, 0x01f2u, 0x01f3u, 0x01f4u, 0x01f5u, 0x01f6u,
        0x01f7u, 0x03f6u, 14u, VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL,
        16u, 8u, TYPE_TRUE, TYPE_FALSE, TYPE_FALSE },
    default_pc_at_firmware_services,
    sizeof(default_pc_at_firmware_services) /
        sizeof(default_pc_at_firmware_services[0])
};

static const vm_profile_default_pc_at_descriptor ibm_5170_model_339_descriptor = {
    "ibm-5170-model-339",
    1u,
    CORE_MACHINE_CPU_PROFILE_80286,
    CORE_MACHINE_FPU_PROFILE_NONE,
    1u,
    { 1u, 0u, 0u, 0u, 0u, 0u },
    { { 0u, 0u, 0u, CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u },
        {{0}}, 0u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE },
    /* IBM 6280099, System Board 1-22 and 1-57: 8254 at 1.193182 MHz,
     * RTC at 32.768 kHz. The CGA character rate is the constrained v6.0
     * 86Box IBM-CGA reference rate 157500000/88 Hz. Ratios are to this
     * profile's nominal 8 MHz CPU source; they do not model availability,
     * waits, monitor output, or host elapsed time. */
    { { 3u, 8u, 0u }, { 596591u, 4000000u, 0u }, { 64u, 15625u, 0u },
        { 315u, 1408u, 0u }, { 1u, 1u, 0u }, { 1u, 1u, 0u } },
    { CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 8000000u },
    { CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK },
    /* IBM 6280099 Keyboard: default 500 ms delay and 10 cps typematic,
     * each with +/-20 percent tolerance. These are nominal Model-339 values. */
    4000000u,
    800000u,
    0u,
    32768u,
    { 48u, 8u, 8u },
    { CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_TRUE },
    { { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u } },
    512u * 1024u,
    TYPE_TRUE,
    0x7000u,
    TYPE_FALSE,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_TRUE,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT,
    TYPE_TRUE,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x0200u, 0x40u, 0x00u, 0x00u, 0u, 0x80u },
    default_pc_at_port_leaves,
    sizeof(default_pc_at_port_leaves) / sizeof(default_pc_at_port_leaves[0]),
    default_pc_at_routes,
    sizeof(default_pc_at_routes) / sizeof(default_pc_at_routes[0]),
    { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL,
        0u, 0u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE },
    ibm_5170_model_339_firmware_services,
    sizeof(ibm_5170_model_339_firmware_services) /
        sizeof(ibm_5170_model_339_firmware_services[0])
};

static const type_unsigned_32 ibm_5170_root_contract_ids[] = {1u};

static type_unsigned_32 vm_profile_ibm_5170_device_bit(
    vm_profile_default_pc_at_device_role role)
{
    return 1u << (type_unsigned_32)role;
}

static vm_profile_default_pc_at_device_role vm_profile_ibm_5170_route_device(
    vm_profile_default_pc_at_route_source source)
{
    if (source == VM_PROFILE_DEFAULT_PC_AT_ROUTE_PIT_IRQ0) {
        return VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT;
    }
    if (source == VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_KEYBOARD_IRQ1 ||
        source == VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_AUX_IRQ12) {
        return VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC;
    }
    if (source == VM_PROFILE_DEFAULT_PC_AT_ROUTE_CMOS_IRQ8) {
        return VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS;
    }
    return VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC;
}

const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(C_VOID)
{
    return &default_pc_at_descriptor;
}

const vm_profile_default_pc_at_descriptor *
vm_profile_ibm_5170_model_339_descriptor_get(C_VOID)
{
    return &ibm_5170_model_339_descriptor;
}

static C_INT vm_profile_default_pc_at_cpu_profile_is_valid(
    core_machine_cpu_profile profile)
{
    return profile == CORE_MACHINE_CPU_PROFILE_8086 ||
        profile == CORE_MACHINE_CPU_PROFILE_80186 ||
        profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        profile == CORE_MACHINE_CPU_PROFILE_80386;
}

static C_INT vm_profile_default_pc_at_fpu_profile_is_valid(
    core_machine_fpu_profile profile)
{
    return profile == CORE_MACHINE_FPU_PROFILE_NONE ||
        profile == CORE_MACHINE_FPU_PROFILE_8087 ||
        profile == CORE_MACHINE_FPU_PROFILE_80287 ||
        profile == CORE_MACHINE_FPU_PROFILE_80387;
}

static C_INT vm_profile_default_pc_at_fdc_bounce_is_valid(
    const vm_profile_default_pc_at_descriptor *descriptor)
{
    const STD_SIZE_T physical = (STD_SIZE_T)descriptor->fdc_bounce_segment << 4u;

    return descriptor->fdc_bounce_segment != 0u && physical <=
        descriptor->default_memory_bytes && 512u <=
        descriptor->default_memory_bytes - physical;
}

C_INT vm_profile_default_pc_at_cpu_contract_select(
    const vm_profile_default_pc_at_descriptor *descriptor,
    core_machine_cpu_profile requested_cpu,
    core_machine_fpu_profile requested_fpu,
    vm_profile_default_pc_at_cpu_contract *out_contract)
{
    if (descriptor == STD_NULL || out_contract == STD_NULL ||
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
        descriptor->kbc_typematic_initial_ticks,
        descriptor->kbc_typematic_repeat_ticks,
        descriptor->kbc_command_response_ticks
    };
    return 1;
}

C_INT vm_profile_default_pc_at_core_config_materialize(
    const vm_profile_default_pc_at_descriptor *descriptor,
    const vm_profile_default_pc_at_cpu_contract *contract,
    core_machine_config *out_config,
    core_machine_controller_timing_rules *out_timing_rules)
{
    if (descriptor == STD_NULL || contract == STD_NULL || out_config == STD_NULL ||
        out_timing_rules == STD_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(descriptor)) return 0;
    *out_config = (core_machine_config) {
        .memory_bytes = descriptor->default_memory_bytes,
        .cpu_profile = contract->cpu_profile,
        .fpu_profile = contract->fpu_profile,
        .ticks_per_instruction = contract->ticks_per_instruction,
        .instruction_timing = contract->instruction_timing,
        .transaction_contract = contract->transaction_contract,
        .clock_plan = contract->clock_plan,
        .time_axis = contract->time_axis,
        .kbc_typematic_initial_ticks = contract->kbc_typematic_initial_ticks,
        .kbc_typematic_repeat_ticks = contract->kbc_typematic_repeat_ticks,
        .kbc_command_response_ticks = contract->kbc_command_response_ticks
    };
    *out_timing_rules = contract->controller_timing_rules;
    return 1;
}

type_status vm_profile_ibm_5170_root_declaration_create(
    vm_profile_resolver_declaration *out_declaration)
{
    const vm_profile_default_pc_at_descriptor *descriptor =
        vm_profile_ibm_5170_model_339_descriptor_get();
    vm_profile_default_pc_at_cpu_contract contract;
    vm_profile_resolver_declaration declaration = {0};
    STD_SIZE_T role;
    STD_SIZE_T route_index;

    if (out_declaration == STD_NULL ||
        !vm_profile_default_pc_at_cpu_contract_select(descriptor,
            descriptor->cpu_profile, descriptor->fpu_profile, &contract) ||
        !vm_profile_default_pc_at_core_config_materialize(descriptor, &contract,
            &declaration.values.core.configuration,
            &declaration.values.core.controller_timing_rules)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    declaration.identity = "pc-at-5170";
    declaration.provided_fields = VM_PROFILE_RESOLVER_FIELD_ALL;
    declaration.owned_fields = VM_PROFILE_RESOLVER_FIELD_ALL;
    declaration.values.core.contract_id = ibm_5170_root_contract_ids[0];
    for (role = 0u; role <= VM_PROFILE_DEFAULT_PC_AT_DEVICE_MEMORY_CONTROL; ++role) {
        STD_SIZE_T ordinal;

        for (ordinal = 0u;; ++ordinal) {
            const vm_profile_default_pc_at_port_leaf *leaf =
                vm_profile_default_pc_at_port_leaf_at(descriptor,
                    (vm_profile_default_pc_at_device_role)role, ordinal);
            if (leaf == STD_NULL) break;
            if (declaration.values.port_leaf_count ==
                VM_PROFILE_RESOLVER_PORT_LEAF_CAPACITY) {
                return TYPE_STATUS_NO_MEMORY;
            }
            declaration.values.enabled_devices |= vm_profile_ibm_5170_device_bit(
                (vm_profile_default_pc_at_device_role)role);
            declaration.values.port_leaves[declaration.values.port_leaf_count++] =
                (vm_profile_resolver_port_leaf) {
                    vm_profile_ibm_5170_device_bit(
                        (vm_profile_default_pc_at_device_role)role),
                    leaf->port, leaf->read, leaf->write};
        }
    }
    if (descriptor->cga_vram_present) {
        declaration.values.memory_windows[0] = (vm_profile_resolver_window) {
            0x000b8000u, 0x000bffffu,
            vm_profile_ibm_5170_device_bit(VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP)};
        declaration.values.memory_window_count = 1u;
    }
    for (route_index = 0u; route_index < descriptor->route_count; ++route_index) {
        const vm_profile_default_pc_at_route *route = &descriptor->routes[route_index];
        const type_unsigned_32 device = vm_profile_ibm_5170_device_bit(
            vm_profile_ibm_5170_route_device(route->source));

        if (declaration.values.irq_route_count == VM_PROFILE_RESOLVER_ROUTE_CAPACITY) {
            return TYPE_STATUS_NO_MEMORY;
        }
        declaration.values.irq_routes[declaration.values.irq_route_count++] =
            (vm_profile_resolver_route) {device, route->irq};
        if (route->dma_channel != VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL) {
            if (declaration.values.drq_route_count ==
                VM_PROFILE_RESOLVER_ROUTE_CAPACITY) {
                return TYPE_STATUS_NO_MEMORY;
            }
            declaration.values.drq_routes[declaration.values.drq_route_count++] =
                (vm_profile_resolver_route) {device, route->dma_channel};
        }
    }
    declaration.values.firmware_policy = VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BUILTIN;
    declaration.values.media_policy = VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION;
    declaration.values.allowed_session_options = 0u;
    *out_declaration = declaration;
    return TYPE_STATUS_OK;
}

type_status vm_profile_ibm_5170_root_resolve(
    vm_profile_default_pc_at_resolved_root *out_root)
{
    const vm_profile_default_pc_at_descriptor *descriptor =
        vm_profile_ibm_5170_model_339_descriptor_get();
    vm_profile_resolver_declaration declaration;
    const vm_profile_resolver_contract_catalog catalog = {
        ibm_5170_root_contract_ids,
        sizeof(ibm_5170_root_contract_ids) / sizeof(ibm_5170_root_contract_ids[0])};

    if (out_root == STD_NULL || descriptor->port_leaf_count >
        VM_PROFILE_DEFAULT_PC_AT_ROOT_PORT_LEAF_CAPACITY || descriptor->route_count >
        VM_PROFILE_DEFAULT_PC_AT_ROOT_ROUTE_CAPACITY || descriptor->firmware_service_count >
        VM_PROFILE_DEFAULT_PC_AT_ROOT_FIRMWARE_SERVICE_CAPACITY) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET(out_root, 0, sizeof(*out_root));
    if (vm_profile_ibm_5170_root_declaration_create(&declaration) != TYPE_STATUS_OK ||
        vm_profile_resolver_resolve(&declaration, &catalog,
            &(vm_profile_resolver_session_request) {0u}, &out_root->resolved) !=
            TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    out_root->descriptor = *descriptor;
    STD_MEMCPY(out_root->port_leaves, descriptor->port_leaves,
        descriptor->port_leaf_count * sizeof(out_root->port_leaves[0]));
    STD_MEMCPY(out_root->routes, descriptor->routes,
        descriptor->route_count * sizeof(out_root->routes[0]));
    STD_MEMCPY(out_root->firmware_services, descriptor->firmware_services,
        descriptor->firmware_service_count * sizeof(out_root->firmware_services[0]));
    out_root->descriptor.identity = "pc-at-5170";
    out_root->descriptor.port_leaves = out_root->port_leaves;
    out_root->descriptor.routes = out_root->routes;
    out_root->descriptor.firmware_services = out_root->firmware_services;
    return TYPE_STATUS_OK;
}

const vm_profile_default_pc_at_port_leaf *
vm_profile_default_pc_at_port_leaf_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device, type_unsigned_16 port)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL) return STD_NULL;
    for (index = 0u; index < descriptor->port_leaf_count; ++index) {
        if (descriptor->port_leaves[index].device == device &&
            (device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC || descriptor->hdc_present) &&
            ((device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE &&
            device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER &&
            device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS) ||
            descriptor->ega_present) &&
            descriptor->port_leaves[index].port == port) {
            return &descriptor->port_leaves[index];
        }
    }
    return STD_NULL;
}

const vm_profile_default_pc_at_port_leaf *
vm_profile_default_pc_at_port_leaf_at(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device, STD_SIZE_T ordinal)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL) return STD_NULL;
    for (index = 0u; index < descriptor->port_leaf_count; ++index) {
        if (descriptor->port_leaves[index].device == device &&
            (device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC || descriptor->hdc_present) &&
            ((device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE &&
            device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER &&
            device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS) ||
            descriptor->ega_present)) {
            if (ordinal == 0u) return &descriptor->port_leaves[index];
            --ordinal;
        }
    }
    return STD_NULL;
}

const vm_profile_default_pc_at_route *vm_profile_default_pc_at_route_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_route_source source)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL) return STD_NULL;
    for (index = 0u; index < descriptor->route_count; ++index) {
        if (descriptor->routes[index].source == source) {
            return &descriptor->routes[index];
        }
    }
    return STD_NULL;
}

C_INT vm_profile_default_pc_at_descriptor_is_valid(
    const vm_profile_default_pc_at_descriptor *descriptor)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL || !vm_profile_default_pc_at_fdc_bounce_is_valid(descriptor) ||
        descriptor->port_leaves == STD_NULL ||
        descriptor->routes == STD_NULL || descriptor->port_leaf_count !=
        sizeof(default_pc_at_port_leaves) / sizeof(default_pc_at_port_leaves[0]) ||
        descriptor->route_count != sizeof(default_pc_at_routes) /
        sizeof(default_pc_at_routes[0])) return 0;
    for (index = 0u; index < descriptor->port_leaf_count; ++index) {
        if (STD_MEMCMP(&descriptor->port_leaves[index],
                &default_pc_at_port_leaves[index],
                sizeof(default_pc_at_port_leaves[index])) != 0) return 0;
    }
    for (index = 0u; index < descriptor->route_count; ++index) {
        if (STD_MEMCMP(&descriptor->routes[index], &default_pc_at_routes[index],
                sizeof(default_pc_at_routes[index])) != 0) return 0;
    }
    if (descriptor->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
        return descriptor->time_axis.kind == CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL &&
            descriptor->time_axis.ticks_per_second == 8000000u &&
            descriptor->default_memory_bytes == 512u * 1024u &&
            descriptor->unpopulated_extended_memory &&
            descriptor->fdc_bounce_segment == 0x7000u &&
            !descriptor->hdc_present && descriptor->planar_parity_present &&
            !descriptor->ega_present && descriptor->cga_vram_present &&
            descriptor->firmware_slot ==
                VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT &&
            descriptor->diskette_drive_a_field_upgrade &&
            descriptor->cmos.base_memory_kib == 0x0200u &&
            descriptor->cmos.floppy_type == 0x40u &&
            descriptor->cmos.fixed_disk_type == 0u &&
            descriptor->cmos.fixed_disk_type_extended_0 == 0u &&
            descriptor->firmware_services != STD_NULL &&
            descriptor->firmware_service_count ==
                sizeof(ibm_5170_model_339_firmware_services) /
                    sizeof(ibm_5170_model_339_firmware_services[0]) &&
            STD_MEMCMP(descriptor->firmware_services,
                ibm_5170_model_339_firmware_services,
                sizeof(ibm_5170_model_339_firmware_services)) == 0;
    }
    return descriptor->hdc_present && !descriptor->planar_parity_present &&
        descriptor->time_axis.kind == CORE_MACHINE_TIME_AXIS_UNQUALIFIED &&
        descriptor->time_axis.ticks_per_second == 0u &&
        descriptor->ega_present && !descriptor->cga_vram_present &&
        descriptor->firmware_slot == VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC &&
        !descriptor->diskette_drive_a_field_upgrade &&
        descriptor->hdc_pio.data_port == 0x01f0u &&
        descriptor->hdc_pio.error_features_port == 0x01f1u &&
        descriptor->hdc_pio.sector_count_port == 0x01f2u &&
        descriptor->hdc_pio.sector_number_port == 0x01f3u &&
        descriptor->hdc_pio.cylinder_low_port == 0x01f4u &&
        descriptor->hdc_pio.cylinder_high_port == 0x01f5u &&
        descriptor->hdc_pio.drive_head_port == 0x01f6u &&
        descriptor->hdc_pio.status_command_port == 0x01f7u &&
        descriptor->hdc_pio.alternate_status_device_control_port == 0x03f6u &&
        descriptor->hdc_pio.irq == 14u &&
        descriptor->hdc_pio.dma_channel ==
            VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL &&
        descriptor->hdc_pio.data_width_bits == 16u &&
        descriptor->hdc_pio.register_width_bits == 8u &&
        descriptor->hdc_pio.lba28_supported && !descriptor->hdc_pio.slave_present &&
        !descriptor->hdc_pio.secondary_channel_present;
}
