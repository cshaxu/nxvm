#include <stdio.h>

#include "vm/platform/platform.h"

int main(void)
{
    vm_platform_run_context first;
    vm_platform_run_context second;

    vm_platform_run_context_initialize(&first, NULL, NULL, NULL, NULL);
    vm_platform_run_context_initialize(&second, NULL, NULL, NULL, NULL);
    vm_platform_run_context_set_window_display(&first, 1);
    if (!vm_platform_run_context_get_window_display(&first) ||
        vm_platform_run_context_get_window_display(&second)) return 1;
    vm_platform_run_context_set_window_display(&second, 1);
    vm_platform_run_context_set_window_display(&first, 0);
    if (vm_platform_run_context_get_window_display(&first) ||
        !vm_platform_run_context_get_window_display(&second)) return 1;
    puts("M5:T80:S5:MODE-CONTEXT:OK");
    return 0;
}
