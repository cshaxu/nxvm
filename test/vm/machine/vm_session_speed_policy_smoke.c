#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"
#include "../support/rom/session_assets.h"

static C_INT verify_ratio_compare(void)
{
    return vm_session_pacing_ratio_compare(1u, 2u, 2u, 3u) >= 0 ||
        vm_session_pacing_ratio_compare(2u, 3u, 1u, 2u) <= 0 ||
        vm_session_pacing_ratio_compare(7u, 11u, 14u, 22u) != 0 ||
        vm_session_pacing_ratio_compare(UINT64_MAX - 1u, UINT64_MAX,
            UINT64_MAX - 2u, UINT64_MAX - 1u) <= 0 ||
        vm_session_pacing_ratio_compare(UINT64_MAX - 2u, UINT64_MAX - 1u,
            UINT64_MAX - 1u, UINT64_MAX) >= 0;
}

static C_INT verify_wait_speed(vm_session *session, vm_session_speed speed)
{
    const core_machine_run_result waiting = {
        .reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT
    };
    type_unsigned_64 before;
    type_unsigned_64 after;
    core_machine_time_observation observation;
    C_INT advanced = 0;

    if (vm_session_set_speed(session, speed) != TYPE_STATUS_OK ||
        core_machine_get_elapsed_ticks(session->core_machine, &before) != TYPE_STATUS_OK) {
        return 1;
    }
    if (core_machine_reset(session->core_machine) != TYPE_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0043u, 0x34u) !=
            TYPE_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0040u, 4u) !=
            TYPE_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0040u, 0u) !=
            TYPE_STATUS_OK ||
        core_machine_capture_time_observation(session->core_machine, &observation) !=
            TYPE_STATUS_OK || !observation.next_deadline_valid) return 1;
    if (core_machine_get_elapsed_ticks(session->core_machine, &before) != TYPE_STATUS_OK) {
        return 1;
    }
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_TRUE);
    if (vm_session_waiting_advance(session, &waiting, &advanced) != TYPE_STATUS_OK ||
        !advanced || core_machine_get_elapsed_ticks(session->core_machine, &after) !=
            TYPE_STATUS_OK || after <= before) {
        STD_ATOMIC_STORE(&session->control.flagRun, TYPE_FALSE);
        return 1;
    }
    if (vm_session_set_speed(session, VM_SESSION_SPEED_STANDARD) != TYPE_STATUS_INVALID_STATE) {
        STD_ATOMIC_STORE(&session->control.flagRun, TYPE_FALSE);
        return 1;
    }
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_FALSE);
    return 0;
}

int main(void)
{
    vm_session *default_session = STD_NULL;
    vm_session_speed speed;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &default_session) != TYPE_STATUS_OK) {
        vm_session_destroy(default_session);
        return 1;
    }
    failed |= vm_session_get_speed(default_session, &speed) != TYPE_STATUS_OK ||
        speed != VM_SESSION_SPEED_STANDARD;
    failed |= verify_ratio_compare();
    failed |= verify_wait_speed(default_session, VM_SESSION_SPEED_STANDARD);
    failed |= verify_wait_speed(default_session, VM_SESSION_SPEED_TURBO);
    vm_session_destroy(default_session);
    if (failed) return 1;
    puts("M5:T459:S2:SESSION-SPEED-POLICY:OK");
    return 0;
}
