#include "type.h"

#include <windows.h>




#include "vm/composition/session/control.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#include "vm/machine/fdd.h"

#include "vm/profile/default_profile/firmware/bios.h"

#include "vm/composition/session/execution.h"

static DWORD WINAPI run_device(LPVOID parameter)
{
    vm_session_control_start((vm_session_control_state *)parameter);
    return 0u;
}
C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    vm_session *session = STD_NULL;

    if (argc != 2) {
        return 1;
    }
    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    if (vm_machine_fdd_insert_for(&session->fdd, argv[1]) != 0) {
        vm_session_destroy(session);
        return 1;
    }
    vm_profile_default_bios_set_boot_hdd(&session->default_bios, 0);
    vm_session_control_reset(&session->control);
    thread = CreateThread(STD_NULL, 0u, run_device, &session->control, 0u, STD_NULL);
    if (thread == STD_NULL) {
        STD_FPUTS("M5:T10:S4:CONTEXT-LIFECYCLE:THREAD-CREATE-FAILED\n", STD_STDERR);
        vm_session_destroy(session);
        return 1;
    }

    Sleep(10u);
    if (!vm_session_control_is_running(&session->control)) {
        STD_FPUTS("M5:T10:S4:CONTEXT-LIFECYCLE:DEVICE-DID-NOT-START\n", STD_STDERR);
        vm_session_control_stop(&session->control);
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        vm_session_destroy(session);
        return 1;
    }
    vm_session_control_reset(&session->control);
    Sleep(10u);
    vm_session_control_stop(&session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_session_destroy(session);

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
