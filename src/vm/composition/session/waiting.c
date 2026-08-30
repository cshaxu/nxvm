#include "type.h"

#include "core/platform/sleep.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"
#include "vm/platform/virtual_time.h"

/* Compare positive rational values without cross multiplication overflow. */
C_INT vm_session_pacing_ratio_compare(type_unsigned_64 left_numerator,
    type_unsigned_64 left_denominator, type_unsigned_64 right_numerator,
    type_unsigned_64 right_denominator)
{
    C_INT inverted = 0;

    if (left_denominator == 0u || right_denominator == 0u) return 0;
    for (;;) {
        type_unsigned_64 left_integer = left_numerator / left_denominator;
        type_unsigned_64 right_integer = right_numerator / right_denominator;
        type_unsigned_64 left_remainder;
        type_unsigned_64 right_remainder;
        C_INT result;

        if (left_integer != right_integer) {
            result = left_integer < right_integer ? -1 : 1;
            return inverted ? -result : result;
        }
        left_remainder = left_numerator % left_denominator;
        right_remainder = right_numerator % right_denominator;
        if (left_remainder == 0u || right_remainder == 0u) {
            if (left_remainder == right_remainder) return 0;
            result = left_remainder == 0u ? -1 : 1;
            return inverted ? -result : result;
        }
        left_numerator = left_denominator;
        left_denominator = left_remainder;
        right_numerator = right_denominator;
        right_denominator = right_remainder;
        inverted = !inverted;
    }
}

C_VOID vm_session_pacing_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    session->pacing_origin_valid = TYPE_FALSE;
    session->pacing_host_origin_units = 0u;
    session->pacing_host_units_per_second = 0u;
    session->pacing_core_origin_ticks = 0u;
}

static C_INT vm_session_pacing_waits_at_least_millisecond(
    const vm_session *session, const core_machine_time_observation *observation,
    type_unsigned_64 target_tick, type_unsigned_64 host_units)
{
    type_unsigned_64 host_lead;

    if (session == STD_NULL || observation == STD_NULL ||
        session->pacing_host_units_per_second < 1000u) return 0;
    host_lead = session->pacing_host_units_per_second / 1000u;
    if (host_units > UINT64_MAX - host_lead) return 0;
    return vm_session_pacing_ratio_compare(target_tick -
            session->pacing_core_origin_ticks,
            observation->pacing_ticks_per_second, host_units -
            session->pacing_host_origin_units + host_lead,
            session->pacing_host_units_per_second) > 0;
}

static C_INT vm_session_pacing_target_due(vm_session *session,
    const core_machine_time_observation *observation,
    type_unsigned_64 target_tick)
{
    type_unsigned_64 host_units;
    type_unsigned_64 host_units_per_second;

    if (session == STD_NULL || observation == STD_NULL ||
        !observation->pacing_time_available ||
        observation->pacing_ticks_per_second == 0u ||
        session->speed != VM_SESSION_SPEED_STANDARD) return TYPE_TRUE;
    if (vm_platform_host_monotonic_counter(&host_units,
            &host_units_per_second) != TYPE_STATUS_OK || host_units_per_second == 0u) {
        vm_session_pacing_reset(session);
        return TYPE_TRUE;
    }
    if (!session->pacing_origin_valid ||
        session->pacing_host_units_per_second != host_units_per_second ||
        host_units < session->pacing_host_origin_units ||
        target_tick < session->pacing_core_origin_ticks) {
        session->pacing_host_origin_units = host_units;
        session->pacing_host_units_per_second = host_units_per_second;
        session->pacing_core_origin_ticks = observation->elapsed_ticks;
        session->pacing_origin_valid = TYPE_TRUE;
    }
    while (vm_session_pacing_ratio_compare(target_tick -
            session->pacing_core_origin_ticks,
            observation->pacing_ticks_per_second, host_units -
            session->pacing_host_origin_units,
            session->pacing_host_units_per_second) > 0) {
        if (!vm_session_control_is_running(&session->control)) return TYPE_FALSE;
        /* Keep the final sub-millisecond interval responsive without a
         * fixed 1 ms oversleep. Neither branch advances guest time. */
        if (vm_session_pacing_waits_at_least_millisecond(session, observation,
                target_tick, host_units)) {
            core_platform_sleep_milliseconds(1u);
        } else {
            core_platform_yield();
        }
        if (vm_platform_host_monotonic_counter(&host_units,
                &host_units_per_second) != TYPE_STATUS_OK ||
            host_units_per_second != session->pacing_host_units_per_second ||
            host_units < session->pacing_host_origin_units) {
            vm_session_pacing_reset(session);
            return TYPE_TRUE;
        }
    }
    return TYPE_TRUE;
}

type_status vm_session_pacing_wait(vm_session *session)
{
    core_machine_time_observation observation;

    if (session == STD_NULL || session->core_machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_capture_time_observation(session->core_machine,
            &observation) != TYPE_STATUS_OK || !observation.pacing_time_available ||
        session->speed != VM_SESSION_SPEED_STANDARD) return TYPE_STATUS_OK;
    (C_VOID)vm_session_pacing_target_due(session, &observation,
        observation.elapsed_ticks);
    return TYPE_STATUS_OK;
}

type_status vm_session_waiting_advance(vm_session *session,
    const core_machine_run_result *result, C_INT *out_advanced)
{
    core_machine_time_observation observation;
    type_bool advanced;
    type_status status;

    if (session == STD_NULL || result == STD_NULL || out_advanced == STD_NULL ||
        session->core_machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_advanced = 0;
    if (result->reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        !vm_session_control_is_running(&session->control)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    status = core_machine_capture_time_observation(session->core_machine,
        &observation);
    if (status != TYPE_STATUS_OK) return status;
    if (!observation.next_deadline_valid) {
        if (observation.progress_disposition !=
                CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY) {
            return TYPE_STATUS_OK;
        }
        /* Standard may pace only completed Core progress.  This does not
         * request a guest duration; the following bounded quantum is wholly
         * selected and advanced by Core. */
        if (session->speed == VM_SESSION_SPEED_STANDARD &&
            !vm_session_pacing_target_due(session, &observation,
                observation.elapsed_ticks)) return TYPE_STATUS_OK;
        status = core_machine_advance_l1_compatibility(session->core_machine, &advanced);
        if (status == TYPE_STATUS_OK && advanced) *out_advanced = 1;
        return status;
    }
    if (session->speed == VM_SESSION_SPEED_STANDARD &&
        !vm_session_pacing_target_due(session, &observation,
            observation.next_deadline_tick)) {
        return TYPE_STATUS_OK;
    }
    status = core_machine_advance_to_next_deadline(session->core_machine, &advanced);
    if (status == TYPE_STATUS_OK && advanced) *out_advanced = 1;
    return status;
}
