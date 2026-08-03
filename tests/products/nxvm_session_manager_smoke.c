#include "type.h"

#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"
#include "vm/composition/session/provider.h"

C_INT main(C_VOID)
{
    core_product_session_provider provider;
    core_product_session_manager *manager = STD_NULL;
    core_product_session_snapshot snapshots[2];
    C_VOID *first = STD_NULL;
    C_VOID *second = STD_NULL;
    core_product_session_id id;
    STD_SIZE_T count;

    vm_session_provider_initialize(&provider);
    if (core_product_session_manager_create(&provider, &manager) != NTVDM64_STATUS_OK ||
        core_product_session_manager_open(manager, &id) != NTVDM64_STATUS_OK || id != 0u ||
        core_product_session_manager_borrow_selected(manager, &first) != NTVDM64_STATUS_OK ||
        core_product_session_manager_open(manager, &id) != NTVDM64_STATUS_OK || id != 1u ||
        core_product_session_manager_select(manager, 1u) != NTVDM64_STATUS_OK ||
        core_product_session_manager_borrow_selected(manager, &second) != NTVDM64_STATUS_OK ||
        first == second ||
        core_product_session_manager_close(manager, 1u) != NTVDM64_STATUS_OK ||
        core_product_session_manager_get_selected_id(manager, &id) != NTVDM64_STATUS_OK ||
        id != 0u ||
        core_product_session_manager_open(manager, &id) != NTVDM64_STATUS_OK || id != 1u ||
        core_product_session_manager_select(manager, 1u) != NTVDM64_STATUS_OK ||
        core_product_session_manager_list(manager, snapshots, 2u, &count) != NTVDM64_STATUS_OK ||
        count != 2u || snapshots[0].id != 0u || snapshots[0].selected ||
        snapshots[1].id != 1u || !snapshots[1].selected) {
        core_product_session_manager_destroy(manager);
        return 1;
    }
    core_product_session_manager_destroy(manager);
    STD_PRINTF("M5:T147:S1:VM-SESSION-MANAGER:OK\n");
    return 0;
}
