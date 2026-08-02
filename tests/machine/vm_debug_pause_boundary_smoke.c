#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "vm/composition_control.h"
#include "vm/machine/fdd.h"

static DWORD WINAPI run_full_pc(void *opaque)
{
    vm_composition_control_start((vm_composition_control_state *)opaque);
    return 0u;
}

int main(int argc, char **argv)
{
    HANDLE thread;
    DWORD result;
    vm_composition_live_machine *session;

    if (argc != 2) return 1;
    session = (vm_composition_live_machine *)calloc(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_live_machine_initialize(session);
    vm_composition_live_machine_bind_legacy(session);
    vm_composition_control_initialize(session->control, session);
    if (vm_machine_fdd_insert_for(session->fdd, argv[1]) != 0) {
        vm_composition_control_finalize(session->control, session);
        vm_composition_live_machine_finalize(session);
        free(session);
        return 1;
    }
    vm_composition_control_reset(session->control);
    thread = CreateThread(NULL, 0u, run_full_pc, session->control, 0u, NULL);
    if (thread == NULL) {
        vm_composition_control_finalize(session->control, session);
        vm_composition_live_machine_finalize(session);
        free(session);
        return 1;
    }
    Sleep(10u);
    if (!vm_composition_control_is_running(session->control)) goto fail;

    vm_composition_control_request_pause(session->control, VM_COMPOSITION_PAUSE_EXPLICIT);
    if (!vm_composition_control_wait_for_pause(session->control, 2000u) ||
        vm_composition_control_get_pause_reason(session->control) !=
            VM_COMPOSITION_PAUSE_EXPLICIT ||
        vm_composition_control_is_running(session->control)) goto fail;

    if (!vm_composition_control_step(session->control) ||
        !vm_composition_control_wait_for_pause(session->control, 2000u) ||
        vm_composition_control_get_pause_reason(session->control) != VM_COMPOSITION_PAUSE_STEP)
        goto fail;

    vm_composition_control_continue(session->control);
    Sleep(10u);
    if (!vm_composition_control_is_running(session->control)) goto fail;
    vm_composition_control_stop(session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_composition_control_finalize(session->control, session);
    vm_composition_live_machine_finalize(session);
    free(session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T45:S1:PAUSE-BOUNDARY:OK");
    return 0;

fail:
    vm_composition_control_stop(session->control);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_composition_control_finalize(session->control, session);
    vm_composition_live_machine_finalize(session);
    free(session);
    return 1;
}
