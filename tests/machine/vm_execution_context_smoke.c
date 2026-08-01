#include <windows.h>

#include <stdio.h>

#include "vm/machine/device.h"
#include "machine/vm/execution_context.h"

static DWORD WINAPI run_device(LPVOID parameter)
{
    (void)parameter;
    deviceStart();
    return 0u;
}

int main(int argc, char **argv)
{
    HANDLE thread;
    DWORD result;

    if (argc != 2) {
        return 1;
    }
    deviceInit();
    if (deviceConnectFloppyInsert(argv[1]) != 0) {
        deviceFinal();
        return 1;
    }
    deviceConnectBiosSetBoot(0);
    deviceReset();
    thread = CreateThread(NULL, 0u, run_device, NULL, 0u, NULL);
    if (thread == NULL) {
        fputs("M5:T10:S4:CONTEXT-LIFECYCLE:THREAD-CREATE-FAILED\n", stderr);
        deviceFinal();
        return 1;
    }

    Sleep(10u);
    if (!device.flagRun) {
        fputs("M5:T10:S4:CONTEXT-LIFECYCLE:DEVICE-DID-NOT-START\n", stderr);
        deviceStop();
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        deviceFinal();
        return 1;
    }
    deviceReset();
    Sleep(10u);
    deviceStop();
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    deviceFinal();

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
