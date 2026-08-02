#include <windows.h>

#include <stdio.h>

#include "vm/composition_control.h"
#include "vm/machine/fdd.h"

static DWORD WINAPI run_full_pc(void *opaque)
{
    (void)opaque;
    vm_composition_control_start();
    return 0u;
}

int main(int argc, char **argv)
{
    HANDLE thread;
    DWORD result;
    vm_composition_live_machine session = {0};

    if (argc != 2) return 1;
    vm_composition_live_machine_initialize(&session);
    vm_composition_live_machine_bind_legacy(&session);
    vm_composition_control_initialize(&session);
    if (vm_machine_fdd_insert(argv[1]) != 0) {
        vm_composition_control_finalize(&session);
    vm_composition_live_machine_finalize(&session);
        return 1;
    }
    vm_composition_control_reset();
    thread = CreateThread(NULL, 0u, run_full_pc, NULL, 0u, NULL);
    if (thread == NULL) {
        vm_composition_control_finalize(&session);
    vm_composition_live_machine_finalize(&session);
        return 1;
    }
    Sleep(10u);
    if (!vm_composition_control_is_running()) goto fail;

    vm_composition_control_request_pause(VM_COMPOSITION_PAUSE_EXPLICIT);
    if (!vm_composition_control_wait_for_pause(2000u) ||
        vm_composition_control_get_pause_reason() !=
            VM_COMPOSITION_PAUSE_EXPLICIT ||
        vm_composition_control_is_running()) goto fail;

    if (!vm_composition_control_step() ||
        !vm_composition_control_wait_for_pause(2000u) ||
        vm_composition_control_get_pause_reason() != VM_COMPOSITION_PAUSE_STEP)
        goto fail;

    vm_composition_control_continue();
    Sleep(10u);
    if (!vm_composition_control_is_running()) goto fail;
    vm_composition_control_stop();
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_composition_control_finalize(&session);
    vm_composition_live_machine_finalize(&session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T45:S1:PAUSE-BOUNDARY:OK");
    return 0;

fail:
    vm_composition_control_stop();
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_composition_control_finalize(&session);
    vm_composition_live_machine_finalize(&session);
    return 1;
}
