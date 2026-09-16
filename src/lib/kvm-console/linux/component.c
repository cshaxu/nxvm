#include "lib/kvm-console/console.h"

/* No native input exists while worker_start is unsupported. */
lib_u8 kvm_console_mouse_buttons(lib_u32 buttons)
{
    (void)buttons;
    return 0u;
}

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
