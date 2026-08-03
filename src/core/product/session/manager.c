#include "type.h"

#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"

typedef struct core_product_session_entry {
    core_product_session_id id;
    C_VOID *session;
} core_product_session_entry;

struct core_product_session_manager {
    core_product_session_provider provider;
    core_product_session_entry *entries;
    STD_SIZE_T count;
    core_product_session_id selected_id;
};

static C_INT core_product_session_manager_find(
    const core_product_session_manager *manager, core_product_session_id id,
    STD_SIZE_T *out_index)
{
    STD_SIZE_T index;

    if (manager == STD_NULL) return 0;
    for (index = 0u; index < manager->count; ++index) {
        if (manager->entries[index].id == id) {
            if (out_index != STD_NULL) *out_index = index;
            return 1;
        }
    }
    return 0;
}

ntvdm64_status core_product_session_manager_create(
    const core_product_session_provider *provider,
    core_product_session_manager **out_manager)
{
    core_product_session_manager *manager;
    ntvdm64_status status;

    if (provider == STD_NULL || out_manager == STD_NULL ||
        provider->open == STD_NULL || provider->describe == STD_NULL ||
        provider->close == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    *out_manager = STD_NULL;
    manager = (core_product_session_manager *)STD_CALLOC(1u, sizeof(*manager));
    if (manager == STD_NULL) return NTVDM64_STATUS_NO_MEMORY;
    manager->provider = *provider;
    status = core_product_session_manager_open(manager, STD_NULL);
    if (status != NTVDM64_STATUS_OK) {
        STD_FREE(manager);
        return status;
    }
    *out_manager = manager;
    return NTVDM64_STATUS_OK;
}

C_VOID core_product_session_manager_destroy(core_product_session_manager *manager)
{
    STD_SIZE_T index;

    if (manager == STD_NULL) return;
    for (index = 0u; index < manager->count; ++index) {
        (C_VOID)manager->provider.close(manager->provider.context,
            manager->entries[index].session);
    }
    STD_FREE(manager->entries);
    STD_FREE(manager);
}

ntvdm64_status core_product_session_manager_open(
    core_product_session_manager *manager, core_product_session_id *out_id)
{
    return core_product_session_manager_open_with_options(manager, STD_NULL,
        out_id);
}

ntvdm64_status core_product_session_manager_open_with_options(
    core_product_session_manager *manager,
    const core_product_session_open_options *options,
    core_product_session_id *out_id)
{
    core_product_session_entry *entries;
    core_product_session_id id;
    C_VOID *session = STD_NULL;
    ntvdm64_status status;

    if (manager == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    id = manager->count == 0u ? 0u : manager->entries[manager->count - 1u].id + 1u;
    status = manager->provider.open(manager->provider.context, id, options,
        &session);
    if (status != NTVDM64_STATUS_OK || session == STD_NULL) {
        return status == NTVDM64_STATUS_OK ? NTVDM64_STATUS_FAULT : status;
    }
    entries = (core_product_session_entry *)STD_MALLOC(
        (manager->count + 1u) * sizeof(*entries));
    if (entries == STD_NULL) {
        (C_VOID)manager->provider.close(manager->provider.context, session);
        return NTVDM64_STATUS_NO_MEMORY;
    }
    if (manager->count != 0u) {
        STD_MEMCPY(entries, manager->entries,
            manager->count * sizeof(*entries));
    }
    STD_FREE(manager->entries);
    manager->entries = entries;
    manager->entries[manager->count].id = id;
    manager->entries[manager->count].session = session;
    ++manager->count;
    if (manager->count == 1u) manager->selected_id = id;
    if (out_id != STD_NULL) *out_id = id;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_session_manager_close(
    core_product_session_manager *manager, core_product_session_id id)
{
    STD_SIZE_T index;
    ntvdm64_status status;

    if (manager == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    /* A product-visible manager is never allowed to become empty. */
    if (manager->count <= 1u) return NTVDM64_STATUS_INVALID_STATE;
    if (!core_product_session_manager_find(manager, id, &index)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    status = manager->provider.close(manager->provider.context,
        manager->entries[index].session);
    if (status != NTVDM64_STATUS_OK) return status;
    if (index + 1u < manager->count) {
        STD_MEMCPY(&manager->entries[index], &manager->entries[index + 1u],
            (manager->count - index - 1u) * sizeof(*manager->entries));
    }
    --manager->count;
    if (manager->selected_id == id) manager->selected_id = manager->entries[0].id;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_session_manager_select(
    core_product_session_manager *manager, core_product_session_id id)
{
    if (!core_product_session_manager_find(manager, id, STD_NULL)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    manager->selected_id = id;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_session_manager_get_selected_id(
    const core_product_session_manager *manager, core_product_session_id *out_id)
{
    if (manager == STD_NULL || out_id == STD_NULL || manager->count == 0u) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    *out_id = manager->selected_id;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_session_manager_get_count(
    const core_product_session_manager *manager, STD_SIZE_T *out_count)
{
    if (manager == STD_NULL || out_count == STD_NULL || manager->count == 0u) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    *out_count = manager->count;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_session_manager_borrow_selected(
    core_product_session_manager *manager, C_VOID **out_session)
{
    STD_SIZE_T index;

    if (manager == STD_NULL || out_session == STD_NULL || manager->count == 0u ||
        !core_product_session_manager_find(manager, manager->selected_id, &index)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    *out_session = manager->entries[index].session;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_session_manager_list(
    const core_product_session_manager *manager,
    core_product_session_snapshot *out_snapshots, STD_SIZE_T capacity,
    STD_SIZE_T *out_count)
{
    STD_SIZE_T index;

    if (manager == STD_NULL || out_count == STD_NULL ||
        (manager->count != 0u && (out_snapshots == STD_NULL || capacity < manager->count))) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < manager->count; ++index) {
        ntvdm64_status status = manager->provider.describe(manager->provider.context,
            manager->entries[index].session, &out_snapshots[index]);
        if (status != NTVDM64_STATUS_OK) return status;
        out_snapshots[index].id = manager->entries[index].id;
        out_snapshots[index].selected =
            manager->entries[index].id == manager->selected_id;
    }
    *out_count = manager->count;
    return NTVDM64_STATUS_OK;
}
