#include "type.h"
#include "vm/composition/session/session_private.h"

#include <windows.h>





#include "core/product/debug/debug_access.h"

#include "vm/composition/session/control.h"

#include "vm/composition/session/debug_target.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/machine/fdd.h"

static DWORD WINAPI run_full_pc(C_VOID *opaque)
{
    vm_session_control_start((vm_session_control_state *)opaque);
    return 0u;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    vm_session *session;
    const core_product_debug_target *target;

    if (argc != 2) return 1;
    session = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    if (vm_machine_fdd_insert_for(&session->fdd, argv[1]) != 0) goto fail;
    target = vm_session_debug_target(session);
    if (target == STD_NULL) goto fail;
    vm_session_control_reset(&session->control);
    thread = CreateThread(STD_NULL, 0u, run_full_pc, &session->control, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    Sleep(10u);
    if (!core_product_debug_is_running(target) ||
        !core_product_debug_request_pause(target, CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT) ||
        !vm_session_control_wait_for_pause(&session->control, 2000u) ||
        !core_product_debug_is_paused(target) ||
        core_product_debug_get_pause_reason(target) != CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT ||
        !core_product_debug_step(target) ||
        !vm_session_control_wait_for_pause(&session->control, 2000u) ||
        core_product_debug_get_pause_reason(target) != CORE_PRODUCT_DEBUG_PAUSE_STEP) goto fail_thread;
    core_product_debug_continue(target);
    Sleep(10u);
    if (!core_product_debug_is_running(target)) goto fail_thread;
    vm_session_control_stop(&session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_session_finalize(session);
    STD_FREE(session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T46:S1:UNIFIED-DEBUG-BACKEND:OK");
    return 0;

fail_thread:
    vm_session_control_stop(&session->control);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
fail:
    vm_session_finalize(session);
    STD_FREE(session);
    return 1;
}
