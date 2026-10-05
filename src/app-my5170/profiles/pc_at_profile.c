#include "app-my5170/profiles/profile_interface.h"

static lib_i32 vm_profile_ibm_5170_memory_is_valid(lib_size memory_bytes);
static lib_i32 vm_profile_ibm_5170_descriptor_validate(
    const vm_profile_default_pc_at_descriptor *descriptor);

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
    vm_pc_at_firmware_services,
    sizeof(vm_pc_at_firmware_services) /
        sizeof(vm_pc_at_firmware_services[0]),
    /* IBM 5170 Technical Reference: A: is a 96-TPI, 80-cylinder 1.2 MB
     * physical unit. A 360 KB disk changes only the mounted medium. */
    /* PCjs corroborates that the Rev-3 ROM reads this D/S/P-board endpoint
     * before enabling its 360 KB-in-1.2 MB compatibility path. It is an
     * explicit Other-L2 board capability, never a firmware exception. */
    0x01u, 0x01u, {80u, 0u, 0u, 0u}, 0u, 0x03f1u, 0x50u, LIB_TRUE, vm_profile_ibm_5170_descriptor_validate
};

static const lib_u32 ibm_5170_contract_ids[] = {1u};

const vm_profile_default_pc_at_descriptor *
vm_profile_ibm_5170_model_339_descriptor_get(void)
{
    return &ibm_5170_model_339_descriptor;
}

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
        values.allowed_session_options = VM_PROFILE_5170_SESSION_OPTION_MEMORY;
    }
    values.firmware_policy = VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BUILTIN;
    values.media_policy = VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION;
    if (vm_profile_contract_validate(&values, &catalog, memory_bytes != 0u ?
            VM_PROFILE_5170_SESSION_OPTION_MEMORY : 0u) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_values = values;
    return LIB_STATUS_OK;
}

static lib_i32 vm_profile_ibm_5170_memory_is_valid(lib_size memory_bytes)
{
    if (memory_bytes == 0u || memory_bytes == 512u * 1024u ||
        memory_bytes == 640u * 1024u) return 1;
    return memory_bytes >= 1536u * 1024u && memory_bytes <= 3u * 1024u * 1024u &&
        (memory_bytes - 1024u * 1024u) % (512u * 1024u) == 0u;
}

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

static lib_i32 vm_profile_ibm_5170_descriptor_validate(
    const vm_profile_default_pc_at_descriptor *descriptor)
{
    if (descriptor->cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 ||
        !descriptor->kbc_aux_absent) return 0;
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
            sizeof(vm_pc_at_firmware_services) /
                sizeof(vm_pc_at_firmware_services[0]) &&
        lib_memory_compare(descriptor->firmware_services,
            vm_pc_at_firmware_services,
            sizeof(vm_pc_at_firmware_services)) == 0;
}
