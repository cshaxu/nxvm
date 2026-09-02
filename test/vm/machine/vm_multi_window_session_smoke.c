#include "type.h"
#include "vm/composition/session/session_private.h"

#include <windows.h>

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/platform/platform.h"

C_INT main(C_VOID)
{
    vm_session *first = STD_NULL;
    vm_session *second = STD_NULL;
    const vm_session_config config = {
        .font_path = "default-cp437-8x16.bin"
    };
    C_INT failed = 0;

    if (vm_session_create(&config, &first) != TYPE_STATUS_OK ||
        vm_session_create(&config, &second) != TYPE_STATUS_OK) goto done;
    vm_platform_run_context_set_window_display(first->platform_run_context, 1);
    vm_platform_run_context_set_window_display(second->platform_run_context, 1);
    vm_session_start(first);
    vm_session_start(second);
    if (!vm_platform_run_handle_is_active(first->platform_run_handle) ||
        !vm_platform_run_handle_is_active(second->platform_run_handle)) failed = 1;
    Sleep(50u);
    vm_session_stop(first);
    if (vm_platform_run_handle_is_active(first->platform_run_handle) ||
        !vm_platform_run_handle_is_active(second->platform_run_handle)) failed = 1;
    vm_session_stop(second);
    if (vm_platform_run_handle_is_active(second->platform_run_handle)) failed = 1;
done:
    vm_session_destroy(second);
    vm_session_destroy(first);
    if (failed) return 1;
    STD_PRINTF("M5:T149:S1:MULTI-WINDOW:OK\n");
    return 0;
}
