#include "type.h"

#include "core/product/session/command_interface.h"
#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"
#include "vm/composition/session/provider.h"

static type_status count_selected(C_VOID *context, C_VOID *session)
{
    C_INT *count = (C_INT *)context;

    if (count == STD_NULL || session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++*count;
    return TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    core_product_session_provider provider;
    core_product_session_manager *manager = STD_NULL;
    core_product_session_snapshot snapshots[2];
    core_product_session_id id;
    STD_SIZE_T count;
    C_INT selected_count = 0;
    const C_CHAR *invalid_command[] = {"session"};

    vm_session_provider_initialize(&provider);
    if (core_product_session_manager_create(&provider, &manager) != TYPE_STATUS_OK ||
        core_product_session_command_execute(manager, 1, invalid_command,
            STD_NULL, STD_NULL) ||
        core_product_session_manager_get_selected_snapshot(manager, &snapshots[0]) !=
            TYPE_STATUS_INVALID_STATE ||
        core_product_session_manager_apply_selected(manager, count_selected,
            &selected_count) != TYPE_STATUS_INVALID_STATE ||
        selected_count != 0 ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK || id != 0u ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK || id != 1u ||
        core_product_session_manager_select(manager, 1u) != TYPE_STATUS_OK ||
        core_product_session_manager_get_selected_id(manager, &id) != TYPE_STATUS_OK ||
        id != 1u ||
        core_product_session_manager_close(manager, 1u) != TYPE_STATUS_OK ||
        core_product_session_manager_get_selected_id(manager, &id) != TYPE_STATUS_OK ||
        id != 0u ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK || id != 2u ||
        core_product_session_manager_select(manager, 2u) != TYPE_STATUS_OK ||
        core_product_session_manager_get_selected_snapshot(manager, &snapshots[0]) !=
            TYPE_STATUS_OK || snapshots[0].id != 2u || !snapshots[0].selected ||
        core_product_session_manager_apply_selected(manager, count_selected,
            &selected_count) != TYPE_STATUS_OK || selected_count != 1 ||
        core_product_session_manager_list(manager, snapshots, 2u, &count) != TYPE_STATUS_OK ||
        count != 2u || snapshots[0].id != 0u || snapshots[0].selected ||
        snapshots[1].id != 2u || !snapshots[1].selected) {
        core_product_session_manager_destroy(manager);
        return 1;
    }
    core_product_session_manager_destroy(manager);
    STD_PRINTF("M5:T147:S1:VM-SESSION-MANAGER:OK\n");
    return 0;
}
