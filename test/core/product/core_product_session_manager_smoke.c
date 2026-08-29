#include "type.h"

#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"

typedef struct session_fixture {
    C_INT value;
} session_fixture;

static type_status fixture_open(C_VOID *context, core_product_session_id id,
    const core_product_session_open_options *options, C_VOID **out_session)
{
    session_fixture *fixture = (session_fixture *)STD_MALLOC(sizeof(*fixture));
    (C_VOID)options;
    if (context != STD_NULL) ++*(STD_SIZE_T *)context;
    if (fixture == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    fixture->value = (C_INT)id;
    *out_session = fixture;
    return TYPE_STATUS_OK;
}

static type_status fixture_describe(C_VOID *context, const C_VOID *session,
    core_product_session_snapshot *snapshot)
{
    (C_VOID)context;
    if (session == STD_NULL || snapshot == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    snapshot->state = CORE_PRODUCT_SESSION_STATE_STOPPED;
    snapshot->display = CORE_PRODUCT_SESSION_DISPLAY_CONSOLE;
    return TYPE_STATUS_OK;
}

static type_status fixture_close(C_VOID *context, C_VOID *session)
{
    (C_VOID)context;
    STD_FREE(session);
    return TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    STD_SIZE_T open_count = 0u;
    const core_product_session_provider provider = {
        fixture_open, fixture_describe, fixture_close, &open_count
    };
    const core_product_session_manager_limits capacity_limits = { 2u, 10u };
    const core_product_session_manager_limits id_limits = { 2u, 0u };
    core_product_session_manager *manager = STD_NULL;
    core_product_session_snapshot snapshot;
    core_product_session_id id;
    STD_SIZE_T count;

    if (core_product_session_manager_create(&provider, &manager) != TYPE_STATUS_OK ||
        core_product_session_manager_get_count(manager, &count) != TYPE_STATUS_OK ||
        count != 0u ||
        core_product_session_manager_get_selected_id(manager, &id) != TYPE_STATUS_INVALID_STATE ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK || id != 0u ||
        core_product_session_manager_select(manager, 0u) != TYPE_STATUS_OK ||
        core_product_session_manager_close(manager, 0u) != TYPE_STATUS_OK ||
        core_product_session_manager_get_count(manager, &count) != TYPE_STATUS_OK ||
        count != 0u ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK || id != 1u ||
        core_product_session_manager_select(manager, 1u) != TYPE_STATUS_OK ||
        core_product_session_manager_close(manager, 1u) != TYPE_STATUS_OK ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK || id != 2u ||
        core_product_session_manager_get_selected_id(manager, &id) != TYPE_STATUS_OK ||
        id != 2u ||
        core_product_session_manager_list(manager, &snapshot, 1u, &count) != TYPE_STATUS_OK ||
        count != 1u || snapshot.id != 2u || !snapshot.selected) {
        core_product_session_manager_destroy(manager);
        return 1;
    }
    core_product_session_manager_destroy(manager);
    manager = STD_NULL;
    open_count = 0u;
    if (core_product_session_manager_create_with_limits(&provider,
            &capacity_limits, &manager) != TYPE_STATUS_OK ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_INVALID_STATE ||
        open_count != 2u || core_product_session_manager_get_count(manager,
            &count) != TYPE_STATUS_OK || count != 2u) {
        core_product_session_manager_destroy(manager);
        return 2;
    }
    core_product_session_manager_destroy(manager);
    manager = STD_NULL;
    open_count = 0u;
    if (core_product_session_manager_create_with_limits(&provider, &id_limits,
            &manager) != TYPE_STATUS_OK ||
        core_product_session_manager_open(manager, &id) != TYPE_STATUS_OK ||
        id != 0u || core_product_session_manager_open(manager, &id) !=
            TYPE_STATUS_INVALID_STATE || open_count != 1u ||
        core_product_session_manager_get_count(manager, &count) != TYPE_STATUS_OK ||
        count != 1u) {
        core_product_session_manager_destroy(manager);
        return 3;
    }
    core_product_session_manager_destroy(manager);
    STD_PRINTF("M5:T146:S1:SESSION-MANAGER:OK\n");
    return 0;
}
