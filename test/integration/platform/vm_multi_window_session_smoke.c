#include "type.h"
#include <windows.h>

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/platform.h"
#include "test/integration/support/session_yaml.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_session *first = STD_NULL;
    vm_session *second = STD_NULL;
    integration_yaml_session first_yaml = {0};
    integration_yaml_session second_yaml = {0};
    C_INT failed = 0;
    C_INT stage = 0;

    if (argc != 2 || integration_yaml_session_open(argv[1],
            "default-pc-at-80386-1440k.yaml", &first_yaml) != TYPE_STATUS_OK ||
        integration_yaml_session_open(argv[1], "default-pc-at-80386-1440k.yaml",
            &second_yaml) != TYPE_STATUS_OK ||
        (first = first_yaml.session) == STD_NULL ||
        (second = second_yaml.session) == STD_NULL) { stage = 1; goto done; }
    vm_platform_run_context_set_window_display(first->platform_run_context, 1);
    vm_platform_run_context_set_window_display(second->platform_run_context, 1);
    vm_session_start(first);
    vm_session_start(second);
    if (!vm_platform_run_handle_is_active(first->platform_run_handle) ||
        !vm_platform_run_handle_is_active(second->platform_run_handle)) { failed = 1; stage = 2; }
    Sleep(50u);
    vm_session_stop(first);
    if (vm_platform_run_handle_is_active(first->platform_run_handle) ||
        !vm_platform_run_handle_is_active(second->platform_run_handle)) { failed = 1; stage = 3; }
    vm_session_stop(second);
    if (vm_platform_run_handle_is_active(second->platform_run_handle)) { failed = 1; stage = 4; }
done:
    integration_yaml_session_close(&second_yaml);
    integration_yaml_session_close(&first_yaml);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "T515:MULTI-WINDOW:stage=%d\n", stage);
        return 1;
    }
    STD_PRINTF("M5:T149:S1:MULTI-WINDOW:OK\n");
    return 0;
}
