#include "vm/profile/default_profile/pc_at_profile_private.h"

static C_INT vm_profile_ibm_5170_memory_is_valid(STD_SIZE_T memory_bytes);

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
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_BOARD, 0x0061u, TYPE_TRUE, TYPE_TRUE },
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
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13, 0x13u },
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
    { { 1u, 1u, 0u }, { 1u, 4u, 0u }, { 1u, 1u, 0u },
        { 1u, 1u, 0u }, { 1u, 1u, 0u }, { 1u, 1u, 0u },
        { 1u, 1u, 0u } },
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
    TYPE_TRUE,
    0x9fc0u,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_FALSE,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC,
    TYPE_FALSE,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x027fu, 0x40u, 0xf0u, 0x2fu, 0u, 0x80u },
    default_pc_at_port_leaves,
    sizeof(default_pc_at_port_leaves) / sizeof(default_pc_at_port_leaves[0]),
    default_pc_at_routes,
    sizeof(default_pc_at_routes) / sizeof(default_pc_at_routes[0]),
    { .protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO, .irq = 14u,
        .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = TYPE_TRUE }},
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
    { { 3u, 8u, 0u }, { 596591u, 4000000u, 0u }, { 1u, 1u, 0u },
        { 64u, 15625u, 0u }, { 315u, 1408u, 0u }, { 1u, 1u, 0u },
        { 1u, 1u, 0u } },
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
    TYPE_TRUE,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_TRUE,
    TYPE_TRUE,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT,
    TYPE_FALSE,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x0200u, 0x20u, 0x30u, 0x00u, 0u, 0x80u },
    default_pc_at_port_leaves,
    sizeof(default_pc_at_port_leaves) / sizeof(default_pc_at_port_leaves[0]),
    default_pc_at_routes,
    sizeof(default_pc_at_routes) / sizeof(default_pc_at_routes[0]),
    { .protocol = CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506, .irq = 14u,
        .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = TYPE_FALSE, .clock_ticks_per_second = 8000000u }},
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
        .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_CASCADED,
        .dma_controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .time_axis = contract->time_axis,
        .l1_compatibility_policy = CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS,
        .kbc_typematic_initial_ticks = contract->kbc_typematic_initial_ticks,
        .kbc_typematic_repeat_ticks = contract->kbc_typematic_repeat_ticks,
        .kbc_command_response_ticks = contract->kbc_command_response_ticks
    };
    *out_timing_rules = contract->controller_timing_rules;
    return 1;
}

type_status vm_profile_default_pc_at_topology_materialize(
    const vm_profile_default_pc_at_descriptor *descriptor,
    const core_machine_controller_timing_rules *timing_rules,
    core_machine_plan_topology *out_topology)
{
    const vm_profile_default_pc_at_port_leaf *attribute_first;
    const vm_profile_default_pc_at_port_leaf *attribute_last;
    const vm_profile_default_pc_at_port_leaf *sequencer_first;
    const vm_profile_default_pc_at_port_leaf *sequencer_last;
    const vm_profile_default_pc_at_port_leaf *graphics_first;
    const vm_profile_default_pc_at_port_leaf *graphics_last;
    const vm_profile_default_pc_at_port_leaf *crtc_first;
    const vm_profile_default_pc_at_port_leaf *crtc_last;
    const vm_profile_default_pc_at_port_leaf *cmos_first;
    const vm_profile_default_pc_at_port_leaf *cmos_last;
    const vm_profile_default_pc_at_route *cmos_route;
    const vm_profile_default_pc_at_route *fdc_route;
    core_machine_plan_topology topology = {0};

    if (descriptor == STD_NULL || timing_rules == STD_NULL || out_topology == STD_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(descriptor)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    attribute_first = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 0u);
    attribute_last = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 1u);
    sequencer_first = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 0u);
    sequencer_last = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 1u);
    graphics_first = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 0u);
    graphics_last = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 1u);
    crtc_first = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0u);
    crtc_last = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 4u);
    cmos_first = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0u);
    cmos_last = vm_profile_default_pc_at_port_leaf_at(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 1u);
    cmos_route = vm_profile_default_pc_at_route_find(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_CMOS_IRQ8);
    fdc_route = vm_profile_default_pc_at_route_find(descriptor,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2);
    if ((descriptor->ega_present && (attribute_first == STD_NULL ||
        attribute_last == STD_NULL || sequencer_first == STD_NULL ||
        sequencer_last == STD_NULL || graphics_first == STD_NULL ||
        graphics_last == STD_NULL)) || crtc_first == STD_NULL ||
        crtc_last == STD_NULL || cmos_first == STD_NULL ||
        cmos_last == STD_NULL || cmos_route == STD_NULL || fdc_route == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (descriptor->unpopulated_extended_memory) {
        topology.absent_memory_count = 1u;
        topology.absent_memory[0] =
            (core_machine_absent_memory_config) { 0x00100000u, 0x00f00000u, 0xffu };
    }
    if (descriptor->default_memory_bytes < 0x000a0000u) {
        topology.absent_memory[topology.absent_memory_count++] =
            (core_machine_absent_memory_config) { descriptor->default_memory_bytes,
                0x000a0000u - descriptor->default_memory_bytes, 0xffu };
    }
    /* Every PC/AT descriptor owns system-board Port B. Parity is an optional
     * producer on that one port; generic Default PC/AT retains the port's
     * PIT1/PIT2 visibility without inventing parity memory. */
    topology.planar_parity_present = TYPE_TRUE;
    topology.planar_parity = (core_machine_planar_parity_config) {
        CORE_MACHINE_PC_AT_PORT_B,
        descriptor->planar_parity_present ? descriptor->default_memory_bytes : 0u };
    topology.display_present = TYPE_TRUE;
    topology.display = (core_machine_display_config) {
        .text_timing = descriptor->cga_text_timing,
        .cga_vram_present = descriptor->cga_vram_present,
        .ega_present = descriptor->ega_present,
        .ega_sequencer = descriptor->ega_sequencer,
        .ega_controllers = descriptor->ega_controllers,
        .ports = {
            .attribute_first = attribute_first == STD_NULL ? 0u : attribute_first->port,
            .attribute_last = attribute_last == STD_NULL ? 0u : attribute_last->port,
            .sequencer_first = sequencer_first == STD_NULL ? 0u : sequencer_first->port,
            .sequencer_last = sequencer_last == STD_NULL ? 0u : sequencer_last->port,
            .graphics_first = graphics_first == STD_NULL ? 0u : graphics_first->port,
            .graphics_last = graphics_last == STD_NULL ? 0u : graphics_last->port,
            .crtc_first = crtc_first->port,
            .crtc_last = crtc_last->port
        }
    };
    if (descriptor->monochrome_aperture_absent) {
        topology.absent_memory[topology.absent_memory_count++] =
            (core_machine_absent_memory_config) { 0x000b0000u, 0x00008000u, 0xffu };
    }
    topology.dma_present = TYPE_TRUE;
    topology.dma = (core_machine_dma_wiring) {
        fdc_route->dma_channel, CORE_MACHINE_DMA_CONTROLLER_COUNT,
        CORE_MACHINE_DMA_CASCADE_CHANNEL };
    topology.rtc_cmos_present = TYPE_TRUE;
    topology.rtc_cmos = (core_machine_rtc_cmos_config) {
        .index_port = cmos_first->port,
        .data_port = cmos_last->port,
        .irq = cmos_route->irq,
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
                TYPE_MASK_UNSIGNED_8(descriptor->cmos.base_memory_kib) },
            { CORE_MACHINE_RTC_BASEMEM_MSB,
                TYPE_MASK_UNSIGNED_8(descriptor->cmos.base_memory_kib >> 8) }
        },
        .default_count = CORE_MACHINE_RTC_DEFAULT_COUNT
    };
    *out_topology = topology;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_default_pc_at_values_create(
    const vm_profile_default_pc_at_descriptor *descriptor,
    core_machine_cpu_profile cpu_profile, core_machine_fpu_profile fpu_profile,
    vm_profile_resolver_values *out_values)
{
    vm_profile_default_pc_at_cpu_contract contract;
    STD_SIZE_T role;
    STD_SIZE_T route_index;

    if (out_values == STD_NULL ||
        !vm_profile_default_pc_at_cpu_contract_select(descriptor,
            cpu_profile, fpu_profile, &contract) ||
        !vm_profile_default_pc_at_core_config_materialize(descriptor, &contract,
            &out_values->core.configuration,
            &out_values->core.controller_timing_rules)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    out_values->core.contract_id = ibm_5170_root_contract_ids[0];
    for (role = 0u; role <= VM_PROFILE_DEFAULT_PC_AT_DEVICE_BOARD; ++role) {
        STD_SIZE_T ordinal;

        for (ordinal = 0u;; ++ordinal) {
            const vm_profile_default_pc_at_port_leaf *leaf =
                vm_profile_default_pc_at_port_leaf_at(descriptor,
                    (vm_profile_default_pc_at_device_role)role, ordinal);
            if (leaf == STD_NULL) break;
            if (out_values->port_leaf_count ==
                VM_PROFILE_RESOLVER_PORT_LEAF_CAPACITY) {
                return TYPE_STATUS_NO_MEMORY;
            }
            out_values->enabled_devices |= vm_profile_ibm_5170_device_bit(
                (vm_profile_default_pc_at_device_role)role);
            out_values->port_leaves[out_values->port_leaf_count++] =
                (vm_profile_resolver_port_leaf) {
                    vm_profile_ibm_5170_device_bit(
                        (vm_profile_default_pc_at_device_role)role),
                    leaf->port, leaf->read, leaf->write};
        }
    }
    if (descriptor->cga_vram_present) {
        out_values->memory_windows[0] = (vm_profile_resolver_window) {
            0x000b8000u, 0x000bffffu,
            vm_profile_ibm_5170_device_bit(VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP)};
        out_values->memory_window_count = 1u;
    }
    for (route_index = 0u; route_index < descriptor->route_count; ++route_index) {
        const vm_profile_default_pc_at_route *route = &descriptor->routes[route_index];
        const type_unsigned_32 device = vm_profile_ibm_5170_device_bit(
            vm_profile_ibm_5170_route_device(route->source));

        if (out_values->irq_route_count == VM_PROFILE_RESOLVER_ROUTE_CAPACITY) {
            return TYPE_STATUS_NO_MEMORY;
        }
        out_values->irq_routes[out_values->irq_route_count++] =
            (vm_profile_resolver_route) {device, route->irq};
        if (route->dma_channel != VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL) {
            if (out_values->drq_route_count ==
                VM_PROFILE_RESOLVER_ROUTE_CAPACITY) {
                return TYPE_STATUS_NO_MEMORY;
            }
            out_values->drq_routes[out_values->drq_route_count++] =
                (vm_profile_resolver_route) {device, route->dma_channel};
        }
    }
    return TYPE_STATUS_OK;
}

static type_status vm_profile_default_pc_at_snapshot_copy(
    vm_profile_default_pc_at_resolved_profile *out_profile,
    const vm_profile_default_pc_at_descriptor *descriptor, const C_CHAR *identity,
    type_unsigned_8 floppy_cmos_type)
{
    if (out_profile == STD_NULL || descriptor == STD_NULL || identity == STD_NULL ||
        descriptor->port_leaf_count > VM_PROFILE_DEFAULT_PC_AT_RESOLVED_PORT_LEAF_CAPACITY ||
        descriptor->route_count > VM_PROFILE_DEFAULT_PC_AT_RESOLVED_ROUTE_CAPACITY ||
        descriptor->firmware_service_count >
            VM_PROFILE_DEFAULT_PC_AT_RESOLVED_FIRMWARE_SERVICE_CAPACITY) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    out_profile->descriptor = *descriptor;
    STD_MEMCPY(out_profile->port_leaves, descriptor->port_leaves,
        descriptor->port_leaf_count * sizeof(out_profile->port_leaves[0]));
    STD_MEMCPY(out_profile->routes, descriptor->routes,
        descriptor->route_count * sizeof(out_profile->routes[0]));
    STD_MEMCPY(out_profile->firmware_services, descriptor->firmware_services,
        descriptor->firmware_service_count * sizeof(out_profile->firmware_services[0]));
    out_profile->descriptor.identity = identity;
    out_profile->descriptor.port_leaves = out_profile->port_leaves;
    out_profile->descriptor.routes = out_profile->routes;
    out_profile->descriptor.firmware_services = out_profile->firmware_services;
    out_profile->descriptor.cpu_profile =
        out_profile->resolved.values.core.configuration.cpu_profile;
    out_profile->descriptor.fpu_profile =
        out_profile->resolved.values.core.configuration.fpu_profile;
    out_profile->descriptor.default_memory_bytes =
        out_profile->resolved.values.core.configuration.memory_bytes;
    out_profile->descriptor.cmos.floppy_type = floppy_cmos_type;
    return vm_profile_default_pc_at_topology_materialize(&out_profile->descriptor,
        &out_profile->resolved.values.core.controller_timing_rules,
        &out_profile->topology);
}

type_status vm_profile_ibm_5170_root_declaration_create(
    vm_profile_resolver_declaration *out_declaration)
{
    const vm_profile_default_pc_at_descriptor *descriptor =
        vm_profile_ibm_5170_model_339_descriptor_get();
    vm_profile_resolver_declaration declaration = {0};

    if (out_declaration == STD_NULL ||
        vm_profile_default_pc_at_values_create(descriptor, descriptor->cpu_profile,
            descriptor->fpu_profile, &declaration.values) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    declaration.identity = "pc-at-5170";
    declaration.provided_fields = VM_PROFILE_RESOLVER_FIELD_ALL;
    declaration.owned_fields = VM_PROFILE_RESOLVER_FIELD_ALL;
    declaration.values.firmware_policy = VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BUILTIN;
    declaration.values.media_policy = VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION;
    declaration.values.allowed_session_options = 0u;
    *out_declaration = declaration;
    return TYPE_STATUS_OK;
}

static C_INT vm_profile_ibm_5170_memory_is_valid(STD_SIZE_T memory_bytes)
{
    if (memory_bytes == 0u || memory_bytes == 512u * 1024u ||
        memory_bytes == 640u * 1024u) return 1;
    return memory_bytes >= 1536u * 1024u && memory_bytes <= 3u * 1024u * 1024u &&
        (memory_bytes - 1024u * 1024u) % (512u * 1024u) == 0u;
}

type_status vm_profile_ibm_5170_root_resolve_memory(STD_SIZE_T memory_bytes,
    vm_profile_default_pc_at_resolved_profile *out_profile)
{
    const vm_profile_default_pc_at_descriptor *source =
        vm_profile_ibm_5170_model_339_descriptor_get();
    vm_profile_default_pc_at_descriptor descriptor;
    vm_profile_resolver_declaration declaration;
    const vm_profile_resolver_contract_catalog catalog = {
        ibm_5170_root_contract_ids,
        sizeof(ibm_5170_root_contract_ids) / sizeof(ibm_5170_root_contract_ids[0])};

    if (out_profile == STD_NULL || !vm_profile_ibm_5170_memory_is_valid(memory_bytes)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    descriptor = *source;
    if (memory_bytes != 0u) descriptor.default_memory_bytes = memory_bytes;
    if (memory_bytes == 640u * 1024u || memory_bytes >= 1536u * 1024u) {
        descriptor.cmos.base_memory_kib = 0x0280u;
    }
    if (memory_bytes >= 1536u * 1024u) {
        descriptor.unpopulated_extended_memory = TYPE_FALSE;
    }
    STD_MEMSET(out_profile, 0, sizeof(*out_profile));
    if (vm_profile_ibm_5170_root_declaration_create(&declaration) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (memory_bytes != 0u) {
        declaration.values.core.configuration.memory_bytes = memory_bytes;
        declaration.values.allowed_session_options = VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY;
    }
    if (
        vm_profile_resolver_resolve(&declaration, &catalog,
            &(vm_profile_resolver_session_request) {memory_bytes != 0u ?
                VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY : 0u}, &out_profile->resolved) !=
            TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return vm_profile_default_pc_at_snapshot_copy(out_profile, &descriptor, "pc-at-5170",
        descriptor.cmos.floppy_type);
}

type_status vm_profile_ibm_5170_root_resolve(
    vm_profile_default_pc_at_resolved_profile *out_profile)
{
    return vm_profile_ibm_5170_root_resolve_memory(0u, out_profile);
}

static type_status vm_profile_default_at_request_select(
    const vm_profile_default_at_request *request,
    core_machine_cpu_profile *out_cpu, core_machine_fpu_profile *out_fpu,
    STD_SIZE_T *out_memory)
{
    const vm_profile_default_pc_at_descriptor *descriptor =
        vm_profile_default_pc_at_descriptor_get();

    if (request == STD_NULL || out_cpu == STD_NULL || out_fpu == STD_NULL ||
        out_memory == STD_NULL ||
        (request->requested_options & ~(VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
            VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
            VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY)) != 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_cpu = descriptor->cpu_profile;
    *out_fpu = descriptor->fpu_profile;
    *out_memory = descriptor->default_memory_bytes;
    if ((request->requested_options & VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU) != 0u) {
        *out_cpu = request->cpu_profile;
        *out_fpu = request->fpu_profile;
    }
    if ((request->requested_options & VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY) != 0u) {
        if (request->memory_bytes == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
        *out_memory = request->memory_bytes;
    }
    if ((request->requested_options & VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY) != 0u &&
        request->floppy_cmos_type != 0x10u && request->floppy_cmos_type != 0x20u &&
        request->floppy_cmos_type != 0x30u && request->floppy_cmos_type != 0x40u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return TYPE_STATUS_OK;
}

type_status vm_profile_default_at_child_declaration_create(
    const vm_profile_resolver_declaration *parent,
    const vm_profile_default_at_request *request,
    vm_profile_resolver_declaration *out_declaration)
{
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    STD_SIZE_T memory_bytes;
    vm_profile_resolver_declaration declaration = {0};

    if (parent == STD_NULL || out_declaration == STD_NULL ||
        STD_STRCMP(parent->identity, "pc-at-5170") != 0 ||
        vm_profile_default_at_request_select(request, &cpu_profile, &fpu_profile,
            &memory_bytes) != TYPE_STATUS_OK ||
        vm_profile_default_pc_at_values_create(vm_profile_default_pc_at_descriptor_get(),
            cpu_profile, fpu_profile, &declaration.values) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    declaration.identity = "default-at";
    declaration.parent = parent;
    declaration.provided_fields = VM_PROFILE_RESOLVER_FIELD_CORE |
        VM_PROFILE_RESOLVER_FIELD_DEVICES | VM_PROFILE_RESOLVER_FIELD_PORTS |
        VM_PROFILE_RESOLVER_FIELD_MEMORY | VM_PROFILE_RESOLVER_FIELD_POLICY;
    declaration.owned_fields = declaration.provided_fields;
    declaration.values.core.configuration.memory_bytes = memory_bytes;
    declaration.values.firmware_policy = VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BUILTIN;
    declaration.values.media_policy = VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION;
    declaration.values.allowed_session_options =
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY;
    *out_declaration = declaration;
    return TYPE_STATUS_OK;
}

type_status vm_profile_default_at_child_resolve(
    const vm_profile_default_at_request *request,
    vm_profile_default_pc_at_resolved_profile *out_profile)
{
    vm_profile_resolver_declaration root;
    vm_profile_resolver_declaration child;
    const vm_profile_resolver_contract_catalog catalog = {
        ibm_5170_root_contract_ids,
        sizeof(ibm_5170_root_contract_ids) / sizeof(ibm_5170_root_contract_ids[0])};

    if (out_profile == STD_NULL ||
        vm_profile_ibm_5170_root_declaration_create(&root) != TYPE_STATUS_OK ||
        vm_profile_default_at_child_declaration_create(&root, request, &child) !=
            TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET(out_profile, 0, sizeof(*out_profile));
    if (vm_profile_resolver_resolve(&child, &catalog,
            &(vm_profile_resolver_session_request) {request->requested_options},
            &out_profile->resolved) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return vm_profile_default_pc_at_snapshot_copy(out_profile,
        vm_profile_default_pc_at_descriptor_get(), "default-at",
        (request->requested_options & VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY) != 0u ?
            request->floppy_cmos_type :
            vm_profile_default_pc_at_descriptor_get()->cmos.floppy_type);
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
            descriptor->hdc_present && descriptor->planar_parity_present &&
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
            descriptor->firmware_services != STD_NULL &&
            descriptor->firmware_service_count ==
                sizeof(ibm_5170_model_339_firmware_services) /
                    sizeof(ibm_5170_model_339_firmware_services[0]) &&
            STD_MEMCMP(descriptor->firmware_services,
                ibm_5170_model_339_firmware_services,
                sizeof(ibm_5170_model_339_firmware_services)) == 0;
    }
    return descriptor->firmware_slot == VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC &&
        vm_profile_default_pc_at_cpu_profile_is_valid(descriptor->cpu_profile) &&
        vm_profile_default_pc_at_fpu_profile_is_valid(descriptor->fpu_profile) &&
        descriptor->hdc_present && !descriptor->planar_parity_present &&
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
