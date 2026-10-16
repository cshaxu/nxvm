#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/machine/control.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/waiting.h"
#include "../support/rom/session_assets.h"

static lib_i32 verify_ratio_compare(void)
{
    return x86_nxvm_machine_pacing_ratio_compare(1u, 2u, 2u, 3u) >= 0 ||
        x86_nxvm_machine_pacing_ratio_compare(2u, 3u, 1u, 2u) <= 0 ||
        x86_nxvm_machine_pacing_ratio_compare(7u, 11u, 14u, 22u) != 0 ||
        x86_nxvm_machine_pacing_ratio_compare(UINT64_MAX - 1u, UINT64_MAX,
            UINT64_MAX - 2u, UINT64_MAX - 1u) <= 0 ||
        x86_nxvm_machine_pacing_ratio_compare(UINT64_MAX - 2u, UINT64_MAX - 1u,
            UINT64_MAX - 1u, UINT64_MAX) >= 0;
}

static lib_i32 verify_wait_speed(x86_nxvm_machine *session, x86_nxvm_machine_speed speed)
{
    const x86_core_machine_run_result waiting = {
        .reason = X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT
    };
    lib_u64 before;
    lib_u64 after;
    x86_core_machine_time_observation observation;
            lib_bool advanced = LIB_FALSE;

    if (x86_nxvm_machine_set_speed(session, speed) != LIB_STATUS_OK ||
        x86_core_machine_get_elapsed_ticks(session->x86_core_machine, &before) != LIB_STATUS_OK) {
        return 1;
    }
    if (x86_core_machine_reset(session->x86_core_machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x0043u, 0x34u) !=
            LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x0040u, 4u) !=
            LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x0040u, 0u) !=
            LIB_STATUS_OK ||
        x86_core_machine_capture_time_observation(session->x86_core_machine, &observation) !=
            LIB_STATUS_OK || !observation.next_deadline_valid) return 1;
    if (x86_core_machine_get_elapsed_ticks(session->x86_core_machine, &before) != LIB_STATUS_OK) {
        return 1;
    }
    x86_nxvm_machine_executor_state_start(&session->control.state);
    if (x86_nxvm_machine_waiting_advance(session, &waiting, &advanced) != LIB_STATUS_OK ||
        !advanced || x86_core_machine_get_elapsed_ticks(session->x86_core_machine, &after) !=
            LIB_STATUS_OK || after <= before) {
        x86_nxvm_machine_executor_state_stop(&session->control.state);
        return 1;
    }
    if (x86_nxvm_machine_set_speed(session, X86_NXVM_MACHINE_SPEED_STANDARD) != LIB_STATUS_INVALID_STATE) {
        x86_nxvm_machine_executor_state_stop(&session->control.state);
        return 1;
    }
    x86_nxvm_machine_executor_state_stop(&session->control.state);
    /* Stop may arrive after the runner's active check but before HLT service.
     * Cancellation must neither advance guest time nor become a fault. */
    before = after;
    advanced = 1;
    if (x86_nxvm_machine_waiting_advance(session, &waiting, &advanced) != LIB_STATUS_OK ||
        advanced || x86_core_machine_get_elapsed_ticks(session->x86_core_machine, &after) !=
            LIB_STATUS_OK || after != before) {
        puts("Stopped HLT service must succeed without advancing time");
        return 1;
    }
    return 0;
}

int main(void)
{
    x86_nxvm_machine *default_session = LIB_NULL;
    x86_nxvm_machine_speed speed;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &default_session) != LIB_STATUS_OK) {
        x86_nxvm_machine_destroy(default_session);
        return 1;
    }
    failed |= x86_nxvm_machine_get_speed(default_session, &speed) != LIB_STATUS_OK ||
        speed != X86_NXVM_MACHINE_SPEED_STANDARD;
    failed |= verify_ratio_compare();
    failed |= verify_wait_speed(default_session, X86_NXVM_MACHINE_SPEED_STANDARD);
    failed |= verify_wait_speed(default_session, X86_NXVM_MACHINE_SPEED_TURBO);
    x86_nxvm_machine_destroy(default_session);
    if (failed) return 1;
    puts("SESSION-SPEED-POLICY:OK");
    return 0;
}
