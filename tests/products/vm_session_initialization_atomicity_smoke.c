#include "type.h"

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/composition/session/session_interface.h"
#include "vm/profile/default_profile/pc_at_profile.h"

static C_INT verify_recovery(C_VOID);

static C_VOID initialize_config(vm_session *session,
    const vm_profile_default_pc_at_descriptor *profile)
{
    session->profile = profile;
    session->core_machine_config.memory_bytes = profile->default_memory_bytes;
    session->core_machine_config.cpu_profile = profile->cpu_profile;
    session->core_machine_config.fpu_profile = profile->fpu_profile;
    session->core_machine_config.ticks_per_instruction =
        profile->ticks_per_instruction;
    session->core_machine_config.instruction_timing = profile->instruction_timing;
    session->core_machine_config.clock_plan = profile->clock_plan;
    session->core_machine_config.kbc_typematic_initial_ticks =
        profile->kbc_typematic_initial_ticks;
    session->core_machine_config.kbc_typematic_repeat_ticks =
        profile->kbc_typematic_repeat_ticks;
    session->core_machine_config.kbc_command_response_ticks =
        profile->kbc_command_response_ticks;
}

static C_INT profile_timing_is_materialized(const core_machine_config *config,
    const vm_profile_default_pc_at_descriptor *profile)
{
    return config != STD_NULL && profile != STD_NULL &&
        config->ticks_per_instruction == profile->ticks_per_instruction &&
        STD_MEMCMP(&config->instruction_timing, &profile->instruction_timing,
            sizeof(config->instruction_timing)) == 0 &&
        STD_MEMCMP(&config->clock_plan, &profile->clock_plan,
            sizeof(config->clock_plan)) == 0 &&
        config->kbc_typematic_initial_ticks ==
            profile->kbc_typematic_initial_ticks &&
        config->kbc_typematic_repeat_ticks ==
            profile->kbc_typematic_repeat_ticks &&
        config->kbc_command_response_ticks ==
            profile->kbc_command_response_ticks;
}

static C_INT session_core_config_is_applied(const vm_session *session,
    STD_SIZE_T memory_bytes, core_machine_cpu_profile cpu_profile,
    core_machine_fpu_profile fpu_profile)
{
    STD_SIZE_T observed_memory_bytes = 0u;
    core_machine_cpu_profile observed_cpu_profile;
    core_machine_fpu_profile observed_fpu_profile;

    return session != STD_NULL && session->core_machine != STD_NULL &&
        core_machine_get_memory_bytes(session->core_machine,
            &observed_memory_bytes) == TYPE_STATUS_OK &&
        core_machine_get_cpu_profile(session->core_machine,
            &observed_cpu_profile) == TYPE_STATUS_OK &&
        core_machine_get_fpu_profile(session->core_machine,
            &observed_fpu_profile) == TYPE_STATUS_OK &&
        observed_memory_bytes == memory_bytes &&
        observed_cpu_profile == cpu_profile && observed_fpu_profile == fpu_profile;
}

static C_INT verify_create_materialization(
    const vm_profile_default_pc_at_descriptor *profile)
{
    const vm_session_config overrides = {
        .memory_bytes = 32u * 1024u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_8087
    };
    vm_session *default_session = STD_NULL;
    vm_session *configured_session = STD_NULL;
    C_INT failed = 0;

    failed |= vm_session_create(STD_NULL, &default_session) != TYPE_STATUS_OK ||
        default_session == STD_NULL ||
        default_session->core_machine_config.memory_bytes !=
            profile->default_memory_bytes ||
        default_session->core_machine_config.cpu_profile != profile->cpu_profile ||
        default_session->core_machine_config.fpu_profile != profile->fpu_profile ||
        !profile_timing_is_materialized(&default_session->core_machine_config,
            profile) || !session_core_config_is_applied(default_session,
            profile->default_memory_bytes, profile->cpu_profile,
            profile->fpu_profile);
    failed |= !failed && (vm_session_create(&overrides, &configured_session) !=
        TYPE_STATUS_OK || configured_session == STD_NULL ||
        configured_session->core_machine_config.memory_bytes !=
            overrides.memory_bytes ||
        configured_session->core_machine_config.cpu_profile !=
            overrides.cpu_profile ||
        configured_session->core_machine_config.fpu_profile !=
            overrides.fpu_profile ||
        configured_session->retained_config.memory_bytes != overrides.memory_bytes ||
        configured_session->retained_config.cpu_profile != overrides.cpu_profile ||
        configured_session->retained_config.fpu_profile != overrides.fpu_profile ||
        !profile_timing_is_materialized(&configured_session->core_machine_config,
            profile) || !session_core_config_is_applied(configured_session,
            overrides.memory_bytes, overrides.cpu_profile, overrides.fpu_profile));
    vm_session_destroy(configured_session);
    vm_session_destroy(default_session);
    return failed;
}

static C_INT verify_failure(const vm_profile_default_pc_at_descriptor *profile)
{
    vm_session session = {0};

    initialize_config(&session, profile);
    if (vm_session_initialize(&session) != TYPE_STATUS_INVALID_ARGUMENT ||
        session.active || session.core_machine != STD_NULL ||
        vm_platform_run_handle_is_active(&session.platform_run_handle)) {
        vm_session_finalize(&session);
        return 1;
    }
    vm_session_finalize(&session);
    return 0;
}

static C_INT verify_firmware_failure(
    const vm_profile_default_pc_at_descriptor *source)
{
    vm_profile_default_pc_at_descriptor profile = *source;
    vm_profile_default_pc_at_firmware_service services[16];

    if (source->firmware_service_count == 0u ||
        source->firmware_service_count > sizeof(services) / sizeof(services[0])) {
        return 1;
    }
    STD_MEMCPY(services, source->firmware_services,
        source->firmware_service_count * sizeof(services[0]));
    services[0].hook = (vm_profile_default_pc_at_firmware_hook)0xffu;
    profile.firmware_services = services;
    return verify_failure(&profile);
}

static C_INT verify_core_failure(
    const vm_profile_default_pc_at_descriptor *profile)
{
    vm_session session = {0};

    initialize_config(&session, profile);
    session.core_machine_config.cpu_profile = (core_machine_cpu_profile)0xffu;
    if (vm_session_initialize(&session) != TYPE_STATUS_INVALID_ARGUMENT ||
        session.active || session.core_machine != STD_NULL) {
        vm_session_finalize(&session);
        return 1;
    }
    return 0;
}

static C_INT verify_controller_failure(
    const vm_profile_default_pc_at_descriptor *source)
{
    vm_profile_default_pc_at_descriptor profile = *source;
    vm_profile_default_pc_at_port_leaf leaves[96];
    STD_SIZE_T index;

    if (source->port_leaf_count > sizeof(leaves) / sizeof(leaves[0])) return 1;
    STD_MEMCPY(leaves, source->port_leaves,
        source->port_leaf_count * sizeof(leaves[0]));
    for (index = 0u; index < source->port_leaf_count; ++index) {
        if (leaves[index].device == VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC &&
            leaves[index].port == 0x03f5u) {
            leaves[index].write = TYPE_FALSE;
            profile.port_leaves = leaves;
            return verify_failure(&profile);
        }
    }
    return 1;
}

static C_INT verify_hdc_failure(
    const vm_profile_default_pc_at_descriptor *source)
{
    vm_profile_default_pc_at_descriptor profile = *source;

    profile.hdc_pio.data_port = 0u;
    return verify_failure(&profile);
}

static C_INT verify_image_failure(
    const vm_profile_default_pc_at_descriptor *profile, const C_CHAR *fdd_image,
    const C_CHAR *hdd_image)
{
    const vm_session_config config = {
        .memory_bytes = profile->default_memory_bytes,
        .cpu_profile = profile->cpu_profile,
        .fpu_profile = profile->fpu_profile,
        .fdd_image = fdd_image,
        .hdd_image = hdd_image
    };
    vm_session *session = STD_NULL;

    if (vm_session_create(&config, &session) != TYPE_STATUS_FAULT ||
        session != STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    return verify_recovery();
}

static C_INT verify_recovery(C_VOID)
{
    vm_session *session = STD_NULL;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->active || session->core_machine == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    return 0;
}

C_INT main(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();

    if (profile == STD_NULL || verify_create_materialization(profile) != 0 ||
        verify_core_failure(profile) != 0 ||
        verify_firmware_failure(profile) != 0 ||
        verify_controller_failure(profile) != 0 || verify_hdc_failure(profile) != 0 ||
        verify_image_failure(profile, "t332-missing-fdd.img", STD_NULL) != 0 ||
        verify_image_failure(profile, STD_NULL, "t332-missing-hdd.img") != 0 ||
        verify_recovery() != 0) {
        return 1;
    }
    STD_PRINTF("M5:T300:S3:SESSION-INITIALIZATION-ATOMICITY:OK\n");
    STD_PRINTF("M5:T332:S1:SESSION-CONFIG-MATERIALIZATION:OK\n");
    STD_PRINTF("M5:T332:S2:SESSION-CONSTRUCTION-TRANSACTION:OK\n");
    return 0;
}
