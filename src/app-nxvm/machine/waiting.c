#include "lib/types/types_interface.h"

#include "lib/base/sync_interface.h"
#include "lib/base/clock_interface.h"
#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/waiting.h"

/* Compare positive rational values without cross multiplication overflow. */
lib_i32 vm_machine_pacing_ratio_compare(lib_u64 left_numerator,
    lib_u64 left_denominator, lib_u64 right_numerator,
    lib_u64 right_denominator)
{
    lib_i32 inverted = 0;

    if (left_denominator == 0u || right_denominator == 0u) return 0;
    for (;;) {
        lib_u64 left_integer = left_numerator / left_denominator;
        lib_u64 right_integer = right_numerator / right_denominator;
        lib_u64 left_remainder;
        lib_u64 right_remainder;
        lib_i32 result;

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

void vm_machine_pacing_reset(vm_machine *session)
{
    if (session == LIB_NULL) return;
    session->pacing_origin_valid = LIB_FALSE;
    session->pacing_host_origin_units = 0u;
    session->pacing_host_units_per_second = 0u;
    session->pacing_core_origin_ticks = 0u;
}

static lib_i32 vm_machine_pacing_waits_at_least_millisecond(
    const vm_machine *session, const core_machine_time_observation *observation,
    lib_u64 target_tick, lib_u64 host_units)
{
    lib_u64 host_lead;

    if (session == LIB_NULL || observation == LIB_NULL ||
        session->pacing_host_units_per_second < 1000u) return 0;
    host_lead = session->pacing_host_units_per_second / 1000u;
    if (host_units > UINT64_MAX - host_lead) return 0;
    return vm_machine_pacing_ratio_compare(target_tick -
            session->pacing_core_origin_ticks,
            observation->pacing_ticks_per_second, host_units -
            session->pacing_host_origin_units + host_lead,
            session->pacing_host_units_per_second) > 0;
}

static lib_i32 vm_machine_pacing_target_due(vm_machine *session,
    const core_machine_time_observation *observation,
    lib_u64 target_tick)
{
    lib_u64 host_units;
    lib_u64 host_units_per_second;

    if (session == LIB_NULL || observation == LIB_NULL ||
        !observation->pacing_time_available ||
        observation->pacing_ticks_per_second == 0u ||
        session->speed != VM_MACHINE_SPEED_STANDARD) return LIB_TRUE;
    if (base_clock_monotonic_counter(&host_units,
            &host_units_per_second) != LIB_STATUS_OK || host_units_per_second == 0u) {
        vm_machine_pacing_reset(session);
        return LIB_TRUE;
    }
    if (!session->pacing_origin_valid ||
        session->pacing_host_units_per_second != host_units_per_second ||
        host_units < session->pacing_host_origin_units ||
        target_tick < session->pacing_core_origin_ticks) {
        session->pacing_host_origin_units = host_units;
        session->pacing_host_units_per_second = host_units_per_second;
        session->pacing_core_origin_ticks = observation->elapsed_ticks;
        session->pacing_origin_valid = LIB_TRUE;
    }
    while (vm_machine_pacing_ratio_compare(target_tick -
            session->pacing_core_origin_ticks,
            observation->pacing_ticks_per_second, host_units -
            session->pacing_host_origin_units,
            session->pacing_host_units_per_second) > 0) {
        if (!vm_machine_control_is_running(&session->control)) return LIB_FALSE;
        /* Keep the final sub-millisecond interval responsive without a
         * fixed 1 ms oversleep. Neither branch advances guest time. */
        if (vm_machine_pacing_waits_at_least_millisecond(session, observation,
                target_tick, host_units)) {
            base_sync_sleep_milliseconds(1u);
        } else {
            base_sync_yield();
        }
        if (base_clock_monotonic_counter(&host_units,
                &host_units_per_second) != LIB_STATUS_OK ||
            host_units_per_second != session->pacing_host_units_per_second ||
            host_units < session->pacing_host_origin_units) {
            vm_machine_pacing_reset(session);
            return LIB_TRUE;
        }
    }
    return LIB_TRUE;
}

lib_status vm_machine_pacing_wait(vm_machine *session)
{
    core_machine_time_observation observation;

    if (session == LIB_NULL || session->core_machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_capture_time_observation(session->core_machine,
            &observation) != LIB_STATUS_OK || !observation.pacing_time_available ||
        session->speed != VM_MACHINE_SPEED_STANDARD) return LIB_STATUS_OK;
    (void)vm_machine_pacing_target_due(session, &observation,
        observation.elapsed_ticks);
    return LIB_STATUS_OK;
}

lib_status vm_machine_waiting_advance(vm_machine *session,
    const core_machine_run_result *result, lib_i32 *out_advanced)
{
    core_machine_time_observation observation;
    lib_u8 advanced;
    lib_status status;

    if (session == LIB_NULL || result == LIB_NULL || out_advanced == LIB_NULL ||
        session->core_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_advanced = 0;
    if (result->reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        !vm_machine_control_is_running(&session->control)) {
        return LIB_STATUS_INVALID_STATE;
    }
    status = core_machine_capture_time_observation(session->core_machine,
        &observation);
    if (status != LIB_STATUS_OK) return status;
    if (!observation.next_deadline_valid) {
        if (observation.progress_disposition !=
                CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY) {
            return LIB_STATUS_OK;
        }
        /* Standard may pace only completed Core progress.  This does not
         * request a guest duration; the following bounded quantum is wholly
         * selected and advanced by Core. */
        if (session->speed == VM_MACHINE_SPEED_STANDARD &&
            !vm_machine_pacing_target_due(session, &observation,
                observation.elapsed_ticks)) return LIB_STATUS_OK;
        status = core_machine_advance_l1_compatibility(session->core_machine, &advanced);
        if (status == LIB_STATUS_OK && advanced) *out_advanced = 1;
        return status;
    }
    if (session->speed == VM_MACHINE_SPEED_STANDARD &&
        !vm_machine_pacing_target_due(session, &observation,
            observation.next_deadline_tick)) {
        return LIB_STATUS_OK;
    }
    status = core_machine_advance_to_next_deadline(session->core_machine, &advanced);
    if (status == LIB_STATUS_OK && advanced) *out_advanced = 1;
    return status;
}
