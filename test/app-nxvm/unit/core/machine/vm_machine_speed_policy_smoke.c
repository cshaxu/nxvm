#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/waiting.h"
#include "support/rom/session_assets.h"

static lib_i32 verify_ratio_compare(void)
{
    return vm_machine_pacing_ratio_compare(1u, 2u, 2u, 3u) >= 0 ||
        vm_machine_pacing_ratio_compare(2u, 3u, 1u, 2u) <= 0 ||
        vm_machine_pacing_ratio_compare(7u, 11u, 14u, 22u) != 0 ||
        vm_machine_pacing_ratio_compare(UINT64_MAX - 1u, UINT64_MAX,
            UINT64_MAX - 2u, UINT64_MAX - 1u) <= 0 ||
        vm_machine_pacing_ratio_compare(UINT64_MAX - 2u, UINT64_MAX - 1u,
            UINT64_MAX - 1u, UINT64_MAX) >= 0;
}

static lib_i32 verify_wait_speed(vm_machine *session, vm_machine_speed speed)
{
    const core_machine_run_result waiting = {
        .reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT
    };
    lib_u64 before;
    lib_u64 after;
    core_machine_time_observation observation;
    lib_i32 advanced = 0;

    if (vm_machine_set_speed(session, speed) != LIB_STATUS_OK ||
        core_machine_get_elapsed_ticks(session->core_machine, &before) != LIB_STATUS_OK) {
        return 1;
    }
    if (core_machine_reset(session->core_machine) != LIB_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0043u, 0x34u) !=
            LIB_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0040u, 4u) !=
            LIB_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0040u, 0u) !=
            LIB_STATUS_OK ||
        core_machine_capture_time_observation(session->core_machine, &observation) !=
            LIB_STATUS_OK || !observation.next_deadline_valid) return 1;
    if (core_machine_get_elapsed_ticks(session->core_machine, &before) != LIB_STATUS_OK) {
        return 1;
    }
    vm_machine_executor_state_start(session->control.state);
    if (vm_machine_waiting_advance(session, &waiting, &advanced) != LIB_STATUS_OK ||
        !advanced || core_machine_get_elapsed_ticks(session->core_machine, &after) !=
            LIB_STATUS_OK || after <= before) {
        vm_machine_executor_state_stop(session->control.state);
        return 1;
    }
    if (vm_machine_set_speed(session, VM_MACHINE_SPEED_STANDARD) != LIB_STATUS_INVALID_STATE) {
        vm_machine_executor_state_stop(session->control.state);
        return 1;
    }
    vm_machine_executor_state_stop(session->control.state);
    return 0;
}

int main(void)
{
    vm_machine *default_session = LIB_NULL;
    vm_machine_speed speed;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &default_session) != LIB_STATUS_OK) {
        vm_machine_destroy(default_session);
        return 1;
    }
    failed |= vm_machine_get_speed(default_session, &speed) != LIB_STATUS_OK ||
        speed != VM_MACHINE_SPEED_STANDARD;
    failed |= verify_ratio_compare();
    failed |= verify_wait_speed(default_session, VM_MACHINE_SPEED_STANDARD);
    failed |= verify_wait_speed(default_session, VM_MACHINE_SPEED_TURBO);
    vm_machine_destroy(default_session);
    if (failed) return 1;
    puts("M5:T459:S2:SESSION-SPEED-POLICY:OK");
    return 0;
}
