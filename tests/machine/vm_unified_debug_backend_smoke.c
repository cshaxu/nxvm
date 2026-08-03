#include "type.h"

#include <windows.h>





#include "core/product/debug/debug_access.h"

#include "vm/composition/composition_control.h"

#include "vm/composition/composition_debug.h"

#include "vm/composition/composition_machine.h"

#include "vm/machine/fdd.h"

static DWORD WINAPI run_full_pc(C_VOID *opaque)
{
    vm_composition_control_start((vm_composition_control_state *)opaque);
    return 0u;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    vm_composition_live_machine *session;
    const core_product_debug_target *target;
    C_INT debug_scope_active = 0;

    if (argc != 2) return 1;
    session = (vm_composition_live_machine *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_composition_initialize(session);
    if (vm_machine_fdd_insert_for(session->fdd, argv[1]) != 0) goto fail;
    target = vm_composition_debug_target(session);
    if (target == STD_NULL) goto fail;
    core_product_debug_scope_enter(target);
    debug_scope_active = 1;
    vm_composition_control_reset(session->control);
    thread = CreateThread(STD_NULL, 0u, run_full_pc, session->control, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    Sleep(10u);
    if (!core_product_debug_is_running() ||
        !core_product_debug_request_pause(CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT) ||
        !vm_composition_control_wait_for_pause(session->control, 2000u) ||
        !core_product_debug_is_paused() ||
        core_product_debug_get_pause_reason() != CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT ||
        !core_product_debug_step() ||
        !vm_composition_control_wait_for_pause(session->control, 2000u) ||
        core_product_debug_get_pause_reason() != CORE_PRODUCT_DEBUG_PAUSE_STEP) goto fail_thread;
    core_product_debug_continue();
    Sleep(10u);
    if (!core_product_debug_is_running()) goto fail_thread;
    vm_composition_control_stop(session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    core_product_debug_scope_leave();
    debug_scope_active = 0;
    vm_composition_finalize(session);
    STD_FREE(session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T46:S1:UNIFIED-DEBUG-BACKEND:OK");
    return 0;

fail_thread:
    vm_composition_control_stop(session->control);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
fail:
    if (debug_scope_active) core_product_debug_scope_leave();
    vm_composition_finalize(session);
    STD_FREE(session);
    return 1;
}
