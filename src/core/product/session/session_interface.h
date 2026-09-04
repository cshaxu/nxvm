#ifndef CORE_PRODUCT_SESSION_INTERFACE_H
#define CORE_PRODUCT_SESSION_INTERFACE_H

#include "type.h"


typedef type_unsigned_32 core_product_session_id;

typedef enum core_product_session_state {
    CORE_PRODUCT_SESSION_STATE_READY,
    CORE_PRODUCT_SESSION_STATE_RUNNING,
    CORE_PRODUCT_SESSION_STATE_PAUSED,
    CORE_PRODUCT_SESSION_STATE_STOPPED,
    CORE_PRODUCT_SESSION_STATE_FAULTED
} core_product_session_state;

typedef enum core_product_session_display {
    CORE_PRODUCT_SESSION_DISPLAY_UNKNOWN,
    CORE_PRODUCT_SESSION_DISPLAY_CONSOLE,
    CORE_PRODUCT_SESSION_DISPLAY_WINDOW
} core_product_session_display;

typedef struct core_product_session_open_options {
    C_INT argument_count;
    const C_CHAR *const *arguments;
    /* Provider-owned typed input, borrowed only for provider->open.  The
     * manager neither interprets nor retains it; a provider that needs it
     * after open must make its own copy. */
    const C_VOID *request;
    STD_SIZE_T request_bytes;
} core_product_session_open_options;

typedef type_status (*core_product_session_selected_operation)(
    C_VOID *context, C_VOID *session);

typedef struct core_product_session_snapshot {
    core_product_session_id id;
    C_INT selected;
    core_product_session_state state;
    core_product_session_display display;
    C_CHAR details[64];
} core_product_session_snapshot;

typedef struct core_product_session_manager_limits {
    STD_SIZE_T maximum_sessions;
    core_product_session_id last_session_id;
} core_product_session_manager_limits;

typedef struct core_product_session_manager core_product_session_manager;
typedef struct core_product_session_provider core_product_session_provider;

type_status core_product_session_manager_create(
    const core_product_session_provider *provider,
    core_product_session_manager **out_manager);
type_status core_product_session_manager_create_with_limits(
    const core_product_session_provider *provider,
    const core_product_session_manager_limits *limits,
    core_product_session_manager **out_manager);
C_VOID core_product_session_manager_destroy(core_product_session_manager *manager);
type_status core_product_session_manager_open(
    core_product_session_manager *manager, core_product_session_id *out_id);
type_status core_product_session_manager_open_with_options(
    core_product_session_manager *manager,
    const core_product_session_open_options *options,
    core_product_session_id *out_id);
type_status core_product_session_manager_close(
    core_product_session_manager *manager, core_product_session_id id);
type_status core_product_session_manager_select(
    core_product_session_manager *manager, core_product_session_id id);
type_status core_product_session_manager_get_selected_id(
    const core_product_session_manager *manager, core_product_session_id *out_id);
type_status core_product_session_manager_get_selected_snapshot(
    const core_product_session_manager *manager,
    core_product_session_snapshot *out_snapshot);
type_status core_product_session_manager_get_count(
    const core_product_session_manager *manager, STD_SIZE_T *out_count);
type_status core_product_session_manager_apply_selected(
    core_product_session_manager *manager,
    core_product_session_selected_operation operation, C_VOID *context);
type_status core_product_session_manager_apply_all(
    core_product_session_manager *manager,
    core_product_session_selected_operation operation, C_VOID *context);
type_status core_product_session_manager_list(
    const core_product_session_manager *manager,
    core_product_session_snapshot *out_snapshots, STD_SIZE_T capacity,
    STD_SIZE_T *out_count);

#endif
