#include "type.h"
#include "vm/composition/session/session_private.h"

#include <windows.h>





#include "vm/composition/session/control.h"

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

    if (argc != 2) return 1;
    session = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    if (session == STD_NULL) return 1;
    vm_session_storage_initialize(session);
    vm_session_control_initialize(&session->control, session);
    if (vm_machine_fdd_insert_for(&session->fdd, argv[1]) != 0) {
        vm_session_control_finalize(&session->control, session);
        vm_session_storage_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_control_reset(&session->control);
    thread = CreateThread(STD_NULL, 0u, run_full_pc, &session->control, 0u, STD_NULL);
    if (thread == STD_NULL) {
        vm_session_control_finalize(&session->control, session);
        vm_session_storage_finalize(session);
        STD_FREE(session);
        return 1;
    }
    Sleep(10u);
    if (!vm_session_control_is_running(&session->control)) goto fail;

    vm_session_control_request_pause(&session->control, VM_SESSION_PAUSE_EXPLICIT);
    if (!vm_session_control_wait_for_pause(&session->control, 2000u) ||
        vm_session_control_get_pause_reason(&session->control) !=
            VM_SESSION_PAUSE_EXPLICIT ||
        vm_session_control_is_running(&session->control)) goto fail;

    if (!vm_session_control_step(&session->control) ||
        !vm_session_control_wait_for_pause(&session->control, 2000u) ||
        vm_session_control_get_pause_reason(&session->control) != VM_SESSION_PAUSE_STEP)
        goto fail;

    vm_session_control_continue(&session->control);
    Sleep(10u);
    if (!vm_session_control_is_running(&session->control)) goto fail;
    vm_session_control_stop(&session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_session_control_finalize(&session->control, session);
    vm_session_storage_finalize(session);
    STD_FREE(session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T45:S1:PAUSE-BOUNDARY:OK");
    return 0;

fail:
    vm_session_control_stop(&session->control);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_session_control_finalize(&session->control, session);
    vm_session_storage_finalize(session);
    STD_FREE(session);
    return 1;
}
