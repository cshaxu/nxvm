#include "type.h"

#include <windows.h>




#include "vm/composition/session/control.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/execution.h"
#include "test/integration/support/session_yaml.h"

static DWORD WINAPI run_device(LPVOID parameter)
{
    vm_session_control_start((vm_session_control_state *)parameter);
    return 0u;
}
C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    integration_yaml_session yaml_session;
    vm_session *session;

    if (argc != 3) {
        return 1;
    }
    if (integration_yaml_session_open(argv[1], argv[2], &yaml_session) != TYPE_STATUS_OK) {
        return 77;
    }
    session = yaml_session.session;
    vm_session_control_reset(&session->control);
    thread = CreateThread(STD_NULL, 0u, run_device, &session->control, 0u, STD_NULL);
    if (thread == STD_NULL) {
        STD_FPUTS("M5:T10:S4:CONTEXT-LIFECYCLE:THREAD-CREATE-FAILED\n", STD_STDERR);
        integration_yaml_session_close(&yaml_session);
        return 1;
    }

    Sleep(10u);
    if (!vm_session_control_is_running(&session->control)) {
        STD_FPUTS("M5:T10:S4:CONTEXT-LIFECYCLE:DEVICE-DID-NOT-START\n", STD_STDERR);
        vm_session_control_stop(&session->control);
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        integration_yaml_session_close(&yaml_session);
        return 1;
    }
    vm_session_control_reset(&session->control);
    Sleep(10u);
    vm_session_control_stop(&session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    integration_yaml_session_close(&yaml_session);

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
