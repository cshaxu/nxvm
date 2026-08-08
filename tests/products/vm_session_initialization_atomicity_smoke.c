#include "type.h"

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/composition/session/session_interface.h"
#include "vm/profile/default_profile/pc_at_profile.h"

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
    vm_profile_default_pc_at_port_range ranges[16];
    STD_SIZE_T index;

    if (source->port_range_count > sizeof(ranges) / sizeof(ranges[0])) return 1;
    STD_MEMCPY(ranges, source->port_ranges,
        source->port_range_count * sizeof(ranges[0]));
    for (index = 0u; index < source->port_range_count; ++index) {
        if (ranges[index].device == VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC) {
            ranges[index].last = ranges[index].first + 4u;
            profile.port_ranges = ranges;
            return verify_failure(&profile);
        }
    }
    return 1;
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

    if (profile == STD_NULL || verify_core_failure(profile) != 0 ||
        verify_firmware_failure(profile) != 0 ||
        verify_controller_failure(profile) != 0 || verify_recovery() != 0) {
        return 1;
    }
    STD_PRINTF("M5:T300:S3:SESSION-INITIALIZATION-ATOMICITY:OK\n");
    return 0;
}
