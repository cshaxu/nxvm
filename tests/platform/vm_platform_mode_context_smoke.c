#include "type.h"



#include "vm/platform/platform.h"

C_INT main(C_VOID)
{
    vm_platform_run_context first;
    vm_platform_run_context second;

    vm_platform_run_context_initialize(&first, STD_NULL, STD_NULL, STD_NULL,
        STD_NULL, STD_NULL);
    vm_platform_run_context_initialize(&second, STD_NULL, STD_NULL, STD_NULL,
        STD_NULL, STD_NULL);
    vm_platform_run_context_set_window_display(&first, 1);
    if (!vm_platform_run_context_get_window_display(&first) ||
        vm_platform_run_context_get_window_display(&second)) return 1;
    vm_platform_run_context_set_window_display(&second, 1);
    vm_platform_run_context_set_window_display(&first, 0);
    if (vm_platform_run_context_get_window_display(&first) ||
        !vm_platform_run_context_get_window_display(&second)) return 1;
    vm_platform_run_context_set_display_mode(&first, VM_PLATFORM_DISPLAY_AUTO);
    if (vm_platform_run_context_get_display_mode(&first) !=
            VM_PLATFORM_DISPLAY_AUTO ||
        vm_platform_run_context_get_window_display(&first) ||
        !vm_platform_run_context_request_graphics_promotion(&first) ||
        !vm_platform_run_context_get_window_display(&first) ||
        !vm_platform_run_context_take_auto_promotion(&first) ||
        vm_platform_run_context_take_auto_promotion(&first)) return 1;
    puts("M5:T80:S5:MODE-CONTEXT:OK");
    return 0;
}
