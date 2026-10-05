#include "app-nxvm/profiles/profile_interface.h"

static lib_i32 vm_profile_default_descriptor_validate(
    const vm_profile_default_pc_at_descriptor *descriptor);

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
    vm_pc_at_firmware_services,
    sizeof(vm_pc_at_firmware_services) /
        sizeof(vm_pc_at_firmware_services[0]),
    0x01u, 0x01u, {80u, 0u, 0u, 0u}, 0u, 0u, 0u, LIB_FALSE, vm_profile_default_descriptor_validate
};

static const lib_u32 default_pc_at_contract_ids[] = {1u};

const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(void)
{
    return &default_pc_at_descriptor;
}

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
    const vm_profile_contract_catalog catalog = { default_pc_at_contract_ids,
        sizeof(default_pc_at_contract_ids) / sizeof(default_pc_at_contract_ids[0]) };

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

static lib_i32 vm_profile_default_descriptor_validate(
    const vm_profile_default_pc_at_descriptor *descriptor)
{
    return !descriptor->kbc_aux_absent &&
        descriptor->firmware_slot == VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC &&
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
