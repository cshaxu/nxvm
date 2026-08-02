#include <windows.h>

#include <stdio.h>

#include "vm/composition_control.h"
#include "vm/machine/fdd.h"
#include "vm/profile/default_profile/firmware/vbios.h"
#include "core/product/runtime/execution_context.h"

static DWORD WINAPI run_device(LPVOID parameter)
{
    (void)parameter;
    vm_composition_control_start();
    return 0u;
}

int main(int argc, char **argv)
{
    HANDLE thread;
    DWORD result;

    if (argc != 2) {
        return 1;
    }
    vm_composition_control_initialize();
    if (vm_machine_fdd_insert(argv[1]) != 0) {
        vm_composition_control_finalize();
        return 1;
    }
    vm_profile_default_bios_set_boot_hdd(0);
    vm_composition_control_reset();
    thread = CreateThread(NULL, 0u, run_device, NULL, 0u, NULL);
    if (thread == NULL) {
        fputs("M5:T10:S4:CONTEXT-LIFECYCLE:THREAD-CREATE-FAILED\n", stderr);
        vm_composition_control_finalize();
        return 1;
    }

    Sleep(10u);
    if (!vm_composition_control_is_running()) {
        fputs("M5:T10:S4:CONTEXT-LIFECYCLE:DEVICE-DID-NOT-START\n", stderr);
        vm_composition_control_stop();
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        vm_composition_control_finalize();
        return 1;
    }
    vm_composition_control_reset();
    Sleep(10u);
    vm_composition_control_stop();
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_composition_control_finalize();

    if (result != WAIT_OBJECT_0 || nxvm_execution_context_current() != NULL) {
        fprintf(stderr,
            "M5:T10:S4:CONTEXT-LIFECYCLE:STOP-FAILED:%lu:%d\n",
            (unsigned long)result,
            nxvm_execution_context_current() != NULL);
        return 1;
    }
    puts("M5:T10:S4:CONTEXT-LIFECYCLE:OK");
    return 0;
}
