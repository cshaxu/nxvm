#include "type.h"
#include "vm/composition/session/session.h"

#include <windows.h>

#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/provider.h"
#include "vm/composition/session/session_interface.h"
#include "vm/platform/platform.h"

C_INT main(C_VOID)
{
    core_product_session_provider provider;
    core_product_session_manager *manager = STD_NULL;
    vm_session *first = STD_NULL;
    vm_session *second = STD_NULL;
    C_VOID *opaque = STD_NULL;
    core_product_session_id id;
    C_INT failed = 0;

    vm_session_provider_initialize(&provider);
    if (core_product_session_manager_create(&provider, &manager) != TYPE_STATUS_OK ||
        core_product_session_manager_borrow_selected(manager, &opaque) != TYPE_STATUS_OK) {
        core_product_session_manager_destroy(manager);
        return 1;
    }
    first = (vm_session *)opaque;
    if (core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK ||
        core_product_session_manager_select(manager, id) != TYPE_STATUS_OK ||
        core_product_session_manager_borrow_selected(manager, &opaque) != TYPE_STATUS_OK) {
        core_product_session_manager_destroy(manager);
        return 1;
    }
    second = (vm_session *)opaque;
    vm_platform_run_context_set_window_display(&first->platform_run_context, 1);
    vm_platform_run_context_set_window_display(&second->platform_run_context, 1);
    vm_session_start(first);
    vm_session_start(second);
    if (!vm_platform_run_handle_is_active(&first->platform_run_handle) ||
        !vm_platform_run_handle_is_active(&second->platform_run_handle)) failed = 1;
    Sleep(50u);
    vm_session_stop(first);
    if (vm_platform_run_handle_is_active(&first->platform_run_handle) ||
        !vm_platform_run_handle_is_active(&second->platform_run_handle)) failed = 1;
    vm_session_stop(second);
    if (vm_platform_run_handle_is_active(&second->platform_run_handle)) failed = 1;
    core_product_session_manager_destroy(manager);
    if (failed) return 1;
    STD_PRINTF("M5:T149:S1:MULTI-WINDOW:OK\n");
    return 0;
}
