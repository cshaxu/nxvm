#include "type.h"
#include <windows.h>
#include "vm/machine/runtime/machine_private.h"

#include "core/debug/debug_access.h"

#include "vm/machine/runtime/debug_target.h"

#include "vm/machine/runtime/lifecycle.h"

#include "test/integration/support/session_yaml.h"

static C_INT wait_for_running(const core_debug_target *target)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (core_debug_is_running(target)) return 1;
        Sleep(1u);
    }
    return core_debug_is_running(target);
}

static C_INT wait_for_pause_reason(const core_debug_target *target,
    core_debug_pause_reason reason)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (core_debug_is_paused(target) &&
            core_debug_get_pause_reason(target) == reason)
            return 1;
        Sleep(1u);
    }
    return core_debug_is_paused(target) &&
        core_debug_get_pause_reason(target) == reason;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_yaml_session yaml_session;
    vm_machine *session;
    const core_debug_target *target;

    if (argc != 3 || integration_yaml_session_open(argv[1], argv[2],
            &yaml_session) != TYPE_STATUS_OK) return 77;
    session = yaml_session.session;
    target = vm_machine_debug_target(session);
    if (target == STD_NULL) goto fail;
    if (vm_machine_start(session) != TYPE_STATUS_OK || !wait_for_running(target) ||
        !core_debug_request_pause(target, CORE_DEBUG_PAUSE_EXPLICIT) ||
        !vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        !core_debug_is_paused(target) ||
        core_debug_get_pause_reason(target) != CORE_DEBUG_PAUSE_EXPLICIT ||
        !core_debug_step(target) ||
        !wait_for_pause_reason(target, CORE_DEBUG_PAUSE_STEP)) goto fail;
    core_debug_continue(target);
    if (!wait_for_running(target) ||
        !core_debug_request_pause(target, CORE_DEBUG_PAUSE_EXPLICIT) ||
        !vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        !core_debug_is_paused(target) ||
        core_debug_get_pause_reason(target) != CORE_DEBUG_PAUSE_EXPLICIT) {
        goto fail;
    }
    vm_machine_stop(session);
    integration_yaml_session_close(&yaml_session);
    puts("M5:T46:S1:UNIFIED-DEBUG-BACKEND:OK");
    return 0;

fail:
    if (session != STD_NULL) vm_machine_stop(session);
    integration_yaml_session_close(&yaml_session);
    return 1;
}
