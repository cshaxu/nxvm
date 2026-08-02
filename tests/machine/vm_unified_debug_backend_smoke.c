#include <windows.h>

#include <stdio.h>

#include "core/product/debug/debug_access.h"
#include "vm/composition_control.h"
#include "vm/composition_machine.h"
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
    vm_composition_live_machine session = {0};

    if (argc != 2) return 1;
    machineInit(&session);
    if (vm_machine_fdd_insert_for(session.fdd, argv[1]) != 0) goto fail;
    vm_composition_control_reset(session.control);
    thread = CreateThread(NULL, 0u, run_full_pc, session.control, 0u, NULL);
    if (thread == NULL) goto fail;
    Sleep(10u);
    if (!core_product_debug_is_running() ||
        !core_product_debug_request_pause(CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT) ||
        !vm_composition_control_wait_for_pause(session.control, 2000u) ||
        !core_product_debug_is_paused() ||
        core_product_debug_get_pause_reason() != CORE_PRODUCT_DEBUG_PAUSE_EXPLICIT ||
        !core_product_debug_step() ||
        !vm_composition_control_wait_for_pause(session.control, 2000u) ||
        core_product_debug_get_pause_reason() != CORE_PRODUCT_DEBUG_PAUSE_STEP) goto fail_thread;
    core_product_debug_continue();
    Sleep(10u);
    if (!core_product_debug_is_running()) goto fail_thread;
    vm_composition_control_stop(session.control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    machineFinal(&session);
    if (result != WAIT_OBJECT_0) return 1;
    puts("M5:T46:S1:UNIFIED-DEBUG-BACKEND:OK");
    return 0;

fail_thread:
    vm_composition_control_stop(session.control);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
fail:
    machineFinal(&session);
    return 1;
}
