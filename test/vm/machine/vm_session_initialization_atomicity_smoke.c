#include "type.h"

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/runner.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/profile/default_profile/pc_at_profile_private.h"
#include "../support/rom/session_assets.h"

static C_INT verify_recovery(C_VOID);

static C_INT verify_reset_outcome(C_VOID)
{
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_DEFAULT_PC_AT,
        .bios_count = 1u
    };
    const vm_session_assets missing_assets = {0};
    vm_session *session = STD_NULL;

    return vm_session_create_from_assets(&config, &missing_assets, &session) ==
        TYPE_STATUS_INVALID_ARGUMENT && session == STD_NULL ? 0 : 1;
}

static C_INT verify_running_reset_outcome(C_VOID)
{
    return verify_reset_outcome();
}

static C_VOID initialize_config(vm_session *session,
    const vm_profile_default_pc_at_descriptor *profile)
{
    vm_profile_default_pc_at_cpu_contract contract;

    session->profile = profile;
    if (vm_profile_default_pc_at_cpu_contract_select(profile,
            CORE_MACHINE_CPU_PROFILE_DEFAULT, profile->fpu_profile, &contract)) {
        (C_VOID)vm_profile_default_pc_at_core_config_materialize(profile, &contract,
            &session->core_machine_config, &session->controller_timing_rules);
    }
}

static C_INT profile_timing_is_materialized(const core_machine_config *config,
    const vm_profile_default_pc_at_descriptor *profile)
{
    return config != STD_NULL && profile != STD_NULL &&
        config->ticks_per_instruction == profile->ticks_per_instruction &&
        STD_MEMCMP(&config->instruction_timing, &profile->instruction_timing,
            sizeof(config->instruction_timing)) == 0 &&
        STD_MEMCMP(&config->transaction_contract, &profile->transaction_contract,
            sizeof(config->transaction_contract)) == 0 &&
        STD_MEMCMP(&config->clock_plan, &profile->clock_plan,
            sizeof(config->clock_plan)) == 0 &&
        STD_MEMCMP(&config->time_axis, &profile->time_axis,
            sizeof(config->time_axis)) == 0 &&
        config->kbc_typematic_initial_ticks ==
            profile->kbc_typematic_initial_ticks &&
        config->kbc_typematic_repeat_ticks ==
            profile->kbc_typematic_repeat_ticks &&
        config->kbc_command_response_ticks ==
            profile->kbc_command_response_ticks &&
        config->kbc_command_response_status_polls ==
            profile->kbc_command_response_status_polls;
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
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_80387
    };
    vm_session *default_session = STD_NULL;
    vm_session *configured_session = STD_NULL;
    C_INT failed = 0;

    failed |= vm_test_default_pc_at_session_create(STD_NULL, &default_session) != TYPE_STATUS_OK ||
        default_session == STD_NULL ||
        default_session->core_machine_config.memory_bytes !=
            profile->default_memory_bytes ||
        default_session->core_machine_config.cpu_profile != profile->cpu_profile ||
        default_session->core_machine_config.fpu_profile != profile->fpu_profile ||
        STD_MEMCMP(&default_session->controller_timing_rules,
            &profile->controller_timing_rules,
            sizeof(default_session->controller_timing_rules)) != 0 ||
        !profile_timing_is_materialized(&default_session->core_machine_config,
            profile) || !session_core_config_is_applied(default_session,
            profile->default_memory_bytes, profile->cpu_profile,
            profile->fpu_profile);
    failed |= !failed && (vm_test_default_pc_at_session_create(&overrides, &configured_session) !=
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
        STD_MEMCMP(&configured_session->controller_timing_rules,
            &profile->controller_timing_rules,
            sizeof(configured_session->controller_timing_rules)) != 0 ||
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
        vm_platform_run_handle_is_active(session.platform_run_handle)) {
        vm_session_finalize(&session);
        return 1;
    }
    vm_session_finalize(&session);
    return 0;
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

static C_INT verify_fdd_initialization_failure(
    const vm_profile_default_pc_at_descriptor *profile)
{
    vm_session session = {0};

    initialize_config(&session, profile);
    session.floppy_kind = (vm_profile_floppy_kind)0xffu;
    if (vm_session_initialize(&session) != TYPE_STATUS_FAULT || session.active ||
        session.core_machine != STD_NULL ||
        vm_platform_run_handle_is_active(session.platform_run_handle)) {
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

    profile.hdc.bus.task_file.data_port = 0u;
    return verify_failure(&profile);
}

static C_INT verify_fdc_bounce_failure(
    const vm_profile_default_pc_at_descriptor *source)
{
    vm_profile_default_pc_at_descriptor profile = *source;

    profile.fdc_bounce_segment = (type_unsigned_16)(profile.default_memory_bytes >> 4u);
    return verify_failure(&profile);
}

static C_INT verify_invalid_media_slot(
    const vm_profile_default_pc_at_descriptor *profile)
{
    const vm_session_config config = {
        .memory_bytes = profile->default_memory_bytes,
        .cpu_profile = profile->cpu_profile,
        .fpu_profile = profile->fpu_profile,
        .floppy_image = { STD_NULL, "invalid-second-slot" }
    };
    vm_session *session = STD_NULL;

    if (vm_test_default_pc_at_session_create(&config, &session) != TYPE_STATUS_INVALID_ARGUMENT ||
        session != STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    return verify_recovery();
}

static C_INT verify_recovery(C_VOID)
{
    vm_session *session = STD_NULL;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
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
        verify_fdd_initialization_failure(profile) != 0 ||
        verify_controller_failure(profile) != 0 || verify_hdc_failure(profile) != 0 ||
        verify_fdc_bounce_failure(profile) != 0 || verify_invalid_media_slot(profile) != 0 ||
        verify_recovery() != 0 || verify_reset_outcome() != 0 ||
        verify_running_reset_outcome() != 0) {
        return 1;
    }
    STD_PRINTF("M5:T300:S3:SESSION-INITIALIZATION-ATOMICITY:OK\n");
    STD_PRINTF("M5:T332:S1:SESSION-CONFIG-MATERIALIZATION:OK\n");
    STD_PRINTF("M5:T332:S2:SESSION-CONSTRUCTION-TRANSACTION:OK\n");
    STD_PRINTF("M5:T439:S1:SESSION-RESET-OUTCOME:OK\n");
    return 0;
}
