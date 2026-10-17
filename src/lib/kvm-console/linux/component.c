#include "lib/kvm-console/console.h"

lib_status kvm_console_worker_start(kvm_console *console)
{
    (void)console;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status kvm_console_worker_join(kvm_console *console, lib_u32 timeout_ms)
{
    (void)console;
    (void)timeout_ms;
    return LIB_STATUS_OK;
}
