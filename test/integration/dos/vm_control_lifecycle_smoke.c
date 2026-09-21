#include "type.h"

#include <windows.h>




#include "core/machine/control.h"
#include "core/machine/machine_private.h"
#include "test/integration/support/session_ini.h"

static DWORD WINAPI run_device(LPVOID parameter)
{
    vm_machine_control_start((vm_machine_control_state *)parameter);
    return 0u;
}
C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    integration_ini_session ini_session;
    vm_machine *session;

    if (argc != 3) {
        return 1;
    }
    if (integration_ini_session_open(argv[1], argv[2], &ini_session) != TYPE_STATUS_OK) {
        return 77;
    }
    session = ini_session.session;
    vm_machine_control_reset(&session->control);
    thread = CreateThread(STD_NULL, 0u, run_device, &session->control, 0u, STD_NULL);
    if (thread == STD_NULL) {
        STD_FPUTS("M5:T10:S4:CONTEXT-LIFECYCLE:THREAD-CREATE-FAILED\n", STD_STDERR);
        integration_ini_session_close(&ini_session);
        return 1;
    }

    Sleep(10u);
    if (!vm_machine_control_is_running(&session->control)) {
        STD_FPUTS("M5:T10:S4:CONTEXT-LIFECYCLE:DEVICE-DID-NOT-START\n", STD_STDERR);
        vm_machine_control_stop(&session->control);
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        integration_ini_session_close(&ini_session);
        return 1;
    }
    vm_machine_control_reset(&session->control);
    Sleep(10u);
    vm_machine_control_request_pause(&session->control,
        VM_MACHINE_PAUSE_EXPLICIT);
    if (!vm_machine_control_wait_for_pause(&session->control, 2000u)) {
        STD_FPUTS("M5:T10:S4:CONTEXT-LIFECYCLE:PAUSE-FAILED\n", STD_STDERR);
        vm_machine_control_stop(&session->control);
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        integration_ini_session_close(&ini_session);
        return 1;
    }
    vm_machine_control_continue(&session->control);
    result = GetTickCount();
    while (!vm_machine_control_is_running(&session->control) &&
        GetTickCount() - result < 2000u) Sleep(10u);
    if (!vm_machine_control_is_running(&session->control)) {
        STD_FPUTS("M5:T10:S4:CONTEXT-LIFECYCLE:RESUME-FAILED\n", STD_STDERR);
        vm_machine_control_stop(&session->control);
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        integration_ini_session_close(&ini_session);
        return 1;
    }
    vm_machine_control_stop(&session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    integration_ini_session_close(&ini_session);

    if (result != WAIT_OBJECT_0) {
        STD_FPRINTF(STD_STDERR,
            "M5:T10:S4:CONTEXT-LIFECYCLE:STOP-FAILED:%lu:%d\n",
            (C_ULONG)result,
            0);
        return 1;
    }
    puts("M5:T10:S4:CONTEXT-LIFECYCLE:OK");
    return 0;
}
