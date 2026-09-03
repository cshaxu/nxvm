#include "type.h"
#include "vm/composition/session/session_private.h"

#include <windows.h>





#include "core/product/debug/debug_access.h"

#include "vm/composition/session/control.h"

#include "vm/composition/session/debug_target.h"

#include "vm/composition/session/lifecycle.h"

#include "test/integration/support/session_yaml.h"

static DWORD WINAPI run_full_pc(C_VOID *opaque)
{
    vm_session_control_start((vm_session_control_state *)opaque);
    return 0u;
}

static C_INT wait_for_running(const core_product_debug_target *target)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (core_product_debug_is_running(target)) return 1;
        Sleep(1u);
    }
    return core_product_debug_is_running(target);
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    integration_yaml_session yaml_session;
    vm_session *session;
    const core_product_debug_target *target;

    if (argc != 3 || integration_yaml_session_open(argv[1], argv[2],
            &yaml_session) != TYPE_STATUS_OK) return 77;
    session = yaml_session.session;
    target = vm_session_debug_target(session);
    if (target == STD_NULL) goto fail;
    vm_session_control_reset(&session->control);
    thread = CreateThread(STD_NULL, 0u, run_full_pc, &session->control, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    if (!wait_for_running(target) ||
        !core_product_debug_request_pause(target, CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT) ||
        !vm_session_control_wait_for_pause(&session->control, 2000u) ||
        !core_product_debug_is_paused(target) ||
        core_product_debug_get_pause_reason(target) != CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT ||
        !core_product_debug_step(target) ||
        !vm_session_control_wait_for_pause(&session->control, 2000u) ||
        core_product_debug_get_pause_reason(target) != CORE_PRODUCT_DEBUG_PAUSE_STEP) goto fail_thread;
    core_product_debug_continue(target);
    if (!wait_for_running(target) ||
        !core_product_debug_request_pause(target, CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT) ||
        !vm_session_control_wait_for_pause(&session->control, 2000u) ||
        !core_product_debug_is_paused(target) ||
        core_product_debug_get_pause_reason(target) != CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT) {
        goto fail_thread;
    }
    vm_session_control_stop(&session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    integration_yaml_session_close(&yaml_session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T46:S1:UNIFIED-DEBUG-BACKEND:OK");
    return 0;

fail_thread:
    vm_session_control_stop(&session->control);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
fail:
    integration_yaml_session_close(&yaml_session);
    return 1;
}
