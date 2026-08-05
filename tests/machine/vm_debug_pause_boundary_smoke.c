#include "type.h"
#include "tests/support/vm_session_fixture.h"

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
    session = vm_session_fixture_allocate();
    if (session == STD_NULL) return 1;
    vm_session_fixture_storage_initialize(session);
    vm_session_control_initialize(vm_session_fixture_control(session), session);
    if (vm_machine_fdd_insert_for(vm_session_fixture_fdd(session), argv[1]) != 0) {
        vm_session_control_finalize(vm_session_fixture_control(session), session);
        vm_session_fixture_storage_finalize(session);
        vm_session_fixture_free(session);
        return 1;
    }
    vm_session_control_reset(vm_session_fixture_control(session));
    thread = CreateThread(STD_NULL, 0u, run_full_pc, vm_session_fixture_control(session), 0u, STD_NULL);
    if (thread == STD_NULL) {
        vm_session_control_finalize(vm_session_fixture_control(session), session);
        vm_session_fixture_storage_finalize(session);
        vm_session_fixture_free(session);
        return 1;
    }
    Sleep(10u);
    if (!vm_session_control_is_running(vm_session_fixture_control(session))) goto fail;

    vm_session_control_request_pause(vm_session_fixture_control(session), VM_SESSION_PAUSE_EXPLICIT);
    if (!vm_session_control_wait_for_pause(vm_session_fixture_control(session), 2000u) ||
        vm_session_control_get_pause_reason(vm_session_fixture_control(session)) !=
            VM_SESSION_PAUSE_EXPLICIT ||
        vm_session_control_is_running(vm_session_fixture_control(session))) goto fail;

    if (!vm_session_control_step(vm_session_fixture_control(session)) ||
        !vm_session_control_wait_for_pause(vm_session_fixture_control(session), 2000u) ||
        vm_session_control_get_pause_reason(vm_session_fixture_control(session)) != VM_SESSION_PAUSE_STEP)
        goto fail;

    vm_session_control_continue(vm_session_fixture_control(session));
    Sleep(10u);
    if (!vm_session_control_is_running(vm_session_fixture_control(session))) goto fail;
    vm_session_control_stop(vm_session_fixture_control(session));
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_session_control_finalize(vm_session_fixture_control(session), session);
    vm_session_fixture_storage_finalize(session);
    vm_session_fixture_free(session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T45:S1:PAUSE-BOUNDARY:OK");
    return 0;

fail:
    vm_session_control_stop(vm_session_fixture_control(session));
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_session_control_finalize(vm_session_fixture_control(session), session);
    vm_session_fixture_storage_finalize(session);
    vm_session_fixture_free(session);
    return 1;
}
