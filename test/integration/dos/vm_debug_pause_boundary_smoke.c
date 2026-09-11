#include "type.h"
#include "vm/machine/runtime/machine_private.h"

#include <windows.h>





#include "vm/machine/runtime/control.h"

#include "test/integration/support/session_yaml.h"

static DWORD WINAPI run_full_pc(C_VOID *opaque)
{
    vm_machine_control_start((vm_machine_control_state *)opaque);
    return 0u;
}

static C_INT wait_for_running(const vm_machine_control_state *control)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (vm_machine_control_is_running(control)) return 1;
        Sleep(1u);
    }
    return vm_machine_control_is_running(control);
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    integration_yaml_session yaml_session;
    vm_machine *session;

    if (argc != 3 || integration_yaml_session_open(argv[1], argv[2],
            &yaml_session) != TYPE_STATUS_OK) return 77;
    session = yaml_session.session;
    vm_machine_control_reset(&session->control);
    thread = CreateThread(STD_NULL, 0u, run_full_pc, &session->control, 0u, STD_NULL);
    if (thread == STD_NULL) {
        integration_yaml_session_close(&yaml_session);
        return 1;
    }
    if (!wait_for_running(&session->control)) goto fail;
    vm_machine_control_request_pause(&session->control, VM_MACHINE_PAUSE_EXPLICIT);
    if (!vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        vm_machine_control_get_pause_reason(&session->control) !=
            VM_MACHINE_PAUSE_EXPLICIT ||
        vm_machine_control_is_running(&session->control)) goto fail;

    if (!vm_machine_control_step(&session->control) ||
        !vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        vm_machine_control_get_pause_reason(&session->control) != VM_MACHINE_PAUSE_STEP)
        goto fail;

    vm_machine_control_continue(&session->control);
    if (!wait_for_running(&session->control)) goto fail;
    vm_machine_control_request_pause(&session->control, VM_MACHINE_PAUSE_EXPLICIT);
    if (!vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        vm_machine_control_get_pause_reason(&session->control) !=
            VM_MACHINE_PAUSE_EXPLICIT) goto fail;
    vm_machine_control_stop(&session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    integration_yaml_session_close(&yaml_session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T45:S1:PAUSE-BOUNDARY:OK");
    return 0;

fail:
    vm_machine_control_stop(&session->control);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    integration_yaml_session_close(&yaml_session);
    return 1;
}
