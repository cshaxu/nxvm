#include "type.h"
#include "core/machine/machine_private.h"

#include <windows.h>





#include "core/machine/control.h"

#include "test/integration/support/session_ini.h"

static C_INT wait_for_running(const vm_machine *machine)
{
    ULONGLONG deadline = GetTickCount64() + 2000u;

    do {
        if (vm_machine_control_is_running(&machine->control)) return 1;
        Sleep(10u);
    } while (GetTickCount64() < deadline);
    return machine != STD_NULL && vm_machine_control_is_running(&machine->control);
}

static DWORD WINAPI run_machine(C_VOID *opaque)
{
    vm_machine_control_start(&((vm_machine *)opaque)->control);
    return 0u;
}

static C_INT wait_for_pause_reason(const vm_machine *machine,
    vm_machine_pause_reason reason)
{
    ULONGLONG deadline = GetTickCount64() + 2000u;

    do {
        if (machine != STD_NULL && vm_machine_control_is_paused(&machine->control) &&
            vm_machine_control_get_pause_reason(&machine->control) == reason)
            return 1;
        Sleep(10u);
    } while (GetTickCount64() < deadline);
    return machine != STD_NULL && vm_machine_control_is_paused(&machine->control) &&
        vm_machine_control_get_pause_reason(&machine->control) == reason;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_ini_session ini_session;
    vm_machine *session;
    HANDLE thread = STD_NULL;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != TYPE_STATUS_OK) return 77;
    session = ini_session.session;
    thread = CreateThread(STD_NULL, 0u, run_machine, session, 0u, STD_NULL);
    if (thread == STD_NULL || !wait_for_running(session)) goto fail;
    vm_machine_control_request_pause(&session->control, VM_MACHINE_PAUSE_EXPLICIT);
    if (!vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        vm_machine_control_get_pause_reason(&session->control) !=
            VM_MACHINE_PAUSE_EXPLICIT || !vm_machine_control_is_paused(
            &session->control)) goto fail;

    if (!vm_machine_control_step(&session->control) ||
        !wait_for_pause_reason(session, VM_MACHINE_PAUSE_STEP))
        goto fail;

    vm_machine_control_continue(&session->control);
    if (!wait_for_running(session)) goto fail;
    vm_machine_control_request_pause(&session->control, VM_MACHINE_PAUSE_EXPLICIT);
    if (!vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        vm_machine_control_get_pause_reason(&session->control) !=
        VM_MACHINE_PAUSE_EXPLICIT) goto fail;
    vm_machine_stop(session);
    if (WaitForSingleObject(thread, 2000u) != WAIT_OBJECT_0) goto fail;
    CloseHandle(thread);
    integration_ini_session_close(&ini_session);
    puts("M5:T45:S1:PAUSE-BOUNDARY:OK");
    return 0;

fail:
    if (session != STD_NULL) vm_machine_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    integration_ini_session_close(&ini_session);
    return 1;
}
