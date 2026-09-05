#include "type.h"



#include "vm/platform/platform.h"

C_INT main(C_VOID)
{
    vm_platform_run_context *first = STD_NULL;
    vm_platform_run_context *second = STD_NULL;
    vm_platform_run_handle *handle = STD_NULL;

    if (vm_platform_run_context_create(STD_NULL, STD_NULL, STD_NULL, STD_NULL,
            &first) != TYPE_STATUS_OK || vm_platform_run_context_create(STD_NULL,
            STD_NULL, STD_NULL, STD_NULL, &second) != TYPE_STATUS_OK) goto fail;
    if (vm_platform_run_context_get_display_mode(first) !=
            VM_PLATFORM_DISPLAY_CONSOLE ||
        vm_platform_run_context_get_window_display(first)) goto fail;
    vm_platform_run_context_set_window_display(first, 1);
    if (!vm_platform_run_context_get_window_display(first) ||
        vm_platform_run_context_get_window_display(second)) goto fail;
    vm_platform_run_context_set_window_display(second, 1);
    vm_platform_run_context_set_window_display(first, 0);
    if (vm_platform_run_context_get_window_display(first) ||
        vm_platform_run_context_get_display_mode(first) !=
            VM_PLATFORM_DISPLAY_CONSOLE ||
        !vm_platform_run_context_get_window_display(second)) goto fail;
    vm_platform_run_context_set_display_mode(first, VM_PLATFORM_DISPLAY_CONSOLE);
    if (vm_platform_run_context_get_display_mode(first) !=
            VM_PLATFORM_DISPLAY_CONSOLE ||
        vm_platform_run_context_get_window_display(first) ||
        !vm_platform_run_context_request_console_window_start(first) ||
        vm_platform_run_context_get_window_display(first) ||
        !vm_platform_run_context_take_console_window_start(first) ||
        vm_platform_run_context_take_console_window_start(first)) goto fail;
    vm_platform_run_context_confirm_console_window_started(first);
    if (!vm_platform_run_context_get_window_display(first) ||
        !vm_platform_run_context_request_console_window_stop(first) ||
        vm_platform_run_context_get_window_display(first) ||
        !vm_platform_run_context_take_console_window_stop(first) ||
        vm_platform_run_context_take_console_window_stop(first)) goto fail;
    if (vm_platform_run_handle_create(&handle) != TYPE_STATUS_OK) goto fail;
    vm_platform_run_handle_report(handle, VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED);
    if (vm_platform_run_handle_get_last_event(handle) !=
            VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED ||
        !vm_platform_run_handle_take_pause_report(handle) ||
        vm_platform_run_handle_take_pause_report(handle) ||
        vm_platform_run_handle_take_stop_report(handle)) goto fail;
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(second);
    vm_platform_run_context_destroy(first);
    puts("M5:T80:S5:MODE-CONTEXT:OK");
    return 0;

fail:
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(second);
    vm_platform_run_context_destroy(first);
    return 1;
}
