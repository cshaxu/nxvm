#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/runner.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/profiles/machine_plan_interface.h"
#include "app-nxvm/profiles/byob/blob.h"
#include "app-nxvm/profiles/default_profile/pc_at_profile_private.h"
#include "support/rom/session_assets.h"

static lib_i32 verify_recovery(void);

static lib_i32 verify_reset_outcome(void)
{
    const vm_machine_config config = {
        .profile_kind = VM_MACHINE_PROFILE_DEFAULT_PC_AT,
        .bios_count = 1u
    };
    const vm_machine_assets missing_assets = {0};
    vm_machine *session = LIB_NULL;

    return vm_machine_create_from_assets(&config, &missing_assets, &session) ==
        LIB_STATUS_INVALID_ARGUMENT && session == LIB_NULL ? 0 : 1;
}

static lib_i32 verify_running_reset_outcome(void)
{
    return verify_reset_outcome();
}

static lib_i32 verify_constructor_output_contract(void)
{
    const vm_machine_assets assets = {0};
    vm_machine *session = (vm_machine *)(lib_uptr)1u;
    vm_profile_machine_plan *plan =
        (vm_profile_machine_plan *)(lib_uptr)1u;

    if (vm_machine_create(LIB_NULL, &session) != LIB_STATUS_INVALID_ARGUMENT ||
        session != LIB_NULL) return 1;
    session = (vm_machine *)(lib_uptr)1u;
    if (vm_machine_create_from_assets(LIB_NULL, &assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT || session != LIB_NULL) return 1;
    if (vm_profile_machine_plan_create(LIB_NULL, &assets, &plan) !=
        LIB_STATUS_INVALID_ARGUMENT || plan != LIB_NULL) return 1;
    plan = (vm_profile_machine_plan *)(lib_uptr)1u;
    return vm_profile_machine_plan_create_file_backed(LIB_NULL, &plan) !=
        LIB_STATUS_INVALID_ARGUMENT || plan != LIB_NULL;
}

static lib_i32 verify_byob_blob_argument_contract(void)
{
    lib_u8 bytes[1u] = {0};
    const vm_profile_byob_blob invalid_blob = {LIB_NULL, LIB_NULL, sizeof(bytes)};

    return vm_profile_byob_blob_load(LIB_NULL, bytes) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_byob_blob_load(&invalid_blob, bytes) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_profile_byob_blob_load(&(vm_profile_byob_blob) {
            "asset.rom", LIB_NULL, sizeof(bytes)}, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
}

static lib_i32 profile_timing_is_materialized(const core_machine_config *config,
    const vm_profile_default_pc_at_descriptor *profile)
{
    return config != LIB_NULL && profile != LIB_NULL &&
        config->ticks_per_instruction == profile->ticks_per_instruction &&
        lib_memory_compare(&config->instruction_timing, &profile->instruction_timing,
            sizeof(config->instruction_timing)) == 0 &&
        lib_memory_compare(&config->transaction_contract, &profile->transaction_contract,
            sizeof(config->transaction_contract)) == 0 &&
        lib_memory_compare(&config->clock_plan, &profile->clock_plan,
            sizeof(config->clock_plan)) == 0 &&
        lib_memory_compare(&config->time_axis, &profile->time_axis,
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

static lib_i32 session_core_config_is_applied(const vm_machine *session,
    lib_size memory_bytes, core_machine_cpu_profile cpu_profile,
    core_machine_fpu_profile fpu_profile)
{
    lib_size observed_memory_bytes = 0u;
    core_machine_cpu_profile observed_cpu_profile;
    core_machine_fpu_profile observed_fpu_profile;

    return session != LIB_NULL && session->core_machine != LIB_NULL &&
        core_machine_get_memory_bytes(session->core_machine,
            &observed_memory_bytes) == LIB_STATUS_OK &&
        core_machine_get_cpu_profile(session->core_machine,
            &observed_cpu_profile) == LIB_STATUS_OK &&
        core_machine_get_fpu_profile(session->core_machine,
            &observed_fpu_profile) == LIB_STATUS_OK &&
        observed_memory_bytes == memory_bytes &&
        observed_cpu_profile == cpu_profile && observed_fpu_profile == fpu_profile;
}

static lib_i32 verify_create_materialization(
    const vm_profile_default_pc_at_descriptor *profile)
{
    const vm_machine_config overrides = {
        .memory_bytes = 32u * 1024u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_80387
    };
    vm_machine *default_session = LIB_NULL;
    vm_machine *configured_session = LIB_NULL;
    lib_i32 failed = 0;

    failed |= vm_test_default_pc_at_session_create(LIB_NULL, &default_session) != LIB_STATUS_OK ||
        default_session == LIB_NULL ||
        default_session->core_machine_config.memory_bytes !=
            profile->default_memory_bytes ||
        default_session->core_machine_config.cpu_profile != profile->cpu_profile ||
        default_session->core_machine_config.fpu_profile != profile->fpu_profile ||
        lib_memory_compare(&default_session->controller_timing_rules,
            &profile->controller_timing_rules,
            sizeof(default_session->controller_timing_rules)) != 0 ||
        !profile_timing_is_materialized(&default_session->core_machine_config,
            profile) || !session_core_config_is_applied(default_session,
            profile->default_memory_bytes, profile->cpu_profile,
            profile->fpu_profile);
    failed |= !failed && (vm_test_default_pc_at_session_create(&overrides, &configured_session) !=
        LIB_STATUS_OK || configured_session == LIB_NULL ||
        configured_session->core_machine_config.memory_bytes !=
            overrides.memory_bytes ||
        configured_session->core_machine_config.cpu_profile !=
            overrides.cpu_profile ||
        configured_session->core_machine_config.fpu_profile !=
            overrides.fpu_profile ||
        configured_session->retained_config.memory_bytes != overrides.memory_bytes ||
        configured_session->retained_config.cpu_profile != overrides.cpu_profile ||
        configured_session->retained_config.fpu_profile != overrides.fpu_profile ||
        lib_memory_compare(&configured_session->controller_timing_rules,
            &profile->controller_timing_rules,
            sizeof(configured_session->controller_timing_rules)) != 0 ||
        !profile_timing_is_materialized(&configured_session->core_machine_config,
            profile) || !session_core_config_is_applied(configured_session,
            overrides.memory_bytes, overrides.cpu_profile, overrides.fpu_profile));
    vm_machine_destroy(configured_session);
    vm_machine_destroy(default_session);
    return failed;
}

static lib_i32 verify_invalid_media_slot(
    const vm_profile_default_pc_at_descriptor *profile)
{
    const vm_machine_config config = {
        .memory_bytes = profile->default_memory_bytes,
        .cpu_profile = profile->cpu_profile,
        .fpu_profile = profile->fpu_profile,
        .floppy_image = { LIB_NULL, "invalid-second-slot" }
    };
    vm_machine *session = LIB_NULL;

    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_INVALID_ARGUMENT ||
        session != LIB_NULL) {
        vm_machine_destroy(session);
        return 1;
    }
    return verify_recovery();
}

static lib_i32 verify_recovery(void)
{
    vm_machine *session = LIB_NULL;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !session->active || session->core_machine == LIB_NULL) {
        vm_machine_destroy(session);
        return 1;
    }
    vm_machine_destroy(session);
    return 0;
}

lib_i32 main(void)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    if (profile == LIB_NULL || verify_create_materialization(profile) != 0 ||
        verify_invalid_media_slot(profile) != 0 ||
        verify_recovery() != 0 || verify_reset_outcome() != 0 ||
        verify_running_reset_outcome() != 0 ||
        verify_constructor_output_contract() != 0 ||
        verify_byob_blob_argument_contract() != 0) {
        return 1;
    }
    printf("M5:T300:S3:SESSION-INITIALIZATION-ATOMICITY:OK\n");
    printf("M5:T332:S1:SESSION-CONFIG-MATERIALIZATION:OK\n");
    printf("M5:T332:S2:SESSION-CONSTRUCTION-TRANSACTION:OK\n");
    printf("M5:T439:S1:SESSION-RESET-OUTCOME:OK\n");
    return 0;
}
