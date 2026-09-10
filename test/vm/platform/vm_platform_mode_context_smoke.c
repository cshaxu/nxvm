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
    if (vm_platform_run_context_get_display_mode(first) != VM_PLATFORM_DISPLAY_CONSOLE ||
        vm_platform_run_context_get_window_display(first)) goto fail;
    if (vm_platform_run_context_set_display_mode(first, VM_PLATFORM_DISPLAY_WINDOW) !=
            TYPE_STATUS_OK || vm_platform_run_context_get_display_mode(first) !=
            VM_PLATFORM_DISPLAY_WINDOW || vm_platform_run_context_get_window_display(first) ||
        vm_platform_run_context_get_display_mode(second) != VM_PLATFORM_DISPLAY_CONSOLE) goto fail;
    if (vm_platform_run_handle_create(&handle) != TYPE_STATUS_OK) goto fail;
    vm_platform_run_handle_report(handle, VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED);
    if (!vm_platform_run_handle_take_pause_report(handle) ||
        vm_platform_run_handle_take_pause_report(handle) ||
        vm_platform_run_handle_take_stop_report(handle)) goto fail;
    vm_platform_run_handle_report(handle,
        VM_PLATFORM_RUN_EVENT_WINDOW_CLOSE_REQUESTED);
    if (!vm_platform_run_handle_take_window_close_report(handle) ||
        vm_platform_run_handle_take_window_close_report(handle) ||
        vm_platform_run_handle_take_pause_report(handle)) goto fail;
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(second); vm_platform_run_context_destroy(first);
    return 0;
fail:
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(second); vm_platform_run_context_destroy(first);
    return 1;
}
