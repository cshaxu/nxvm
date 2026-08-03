#include "type.h"

#include <windows.h>


#include <stdio.h>


#include "vm/composition_control.h"

#include "vm/machine/fdd.h"

#include "vm/profile/default_profile/firmware/bios.h"

#include "core/product/runtime/execution_context.h"

static DWORD WINAPI run_device(LPVOID parameter)
{
    vm_composition_control_start((vm_composition_control_state *)parameter);
    return 0u;
}
C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    vm_composition_live_machine session = {0};

    if (argc != 2) {
        return 1;
    }
    vm_composition_live_machine_initialize(&session);
    vm_composition_control_initialize(session.control, &session);
    if (vm_machine_fdd_insert_for(session.fdd, argv[1]) != 0) {
        vm_composition_control_finalize(session.control, &session);
    vm_composition_live_machine_finalize(&session);
        return 1;
    }
    vm_profile_default_bios_set_boot_hdd(session.default_bios, 0);
    vm_composition_control_reset(session.control);
    thread = CreateThread(NULL, 0u, run_device, session.control, 0u, NULL);
    if (thread == NULL) {
        fputs("M5:T10:S4:CONTEXT-LIFECYCLE:THREAD-CREATE-FAILED\n", stderr);
        vm_composition_control_finalize(session.control, &session);
    vm_composition_live_machine_finalize(&session);
        return 1;
    }

    Sleep(10u);
    if (!vm_composition_control_is_running(session.control)) {
        fputs("M5:T10:S4:CONTEXT-LIFECYCLE:DEVICE-DID-NOT-START\n", stderr);
        vm_composition_control_stop(session.control);
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        vm_composition_control_finalize(session.control, &session);
    vm_composition_live_machine_finalize(&session);
        return 1;
    }
    vm_composition_control_reset(session.control);
    Sleep(10u);
    vm_composition_control_stop(session.control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_composition_control_finalize(session.control, &session);
    vm_composition_live_machine_finalize(&session);

    if (result != WAIT_OBJECT_0) {
        fprintf(stderr,
            "M5:T10:S4:CONTEXT-LIFECYCLE:STOP-FAILED:%lu:%d\n",
            (C_ULONG)result,
            0);
        return 1;
    }
    puts("M5:T10:S4:CONTEXT-LIFECYCLE:OK");
    return 0;
}
