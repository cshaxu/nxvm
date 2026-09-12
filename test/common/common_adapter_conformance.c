#include "common/machine/machine_interface.h"
#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"

typedef struct common_adapter {
    lib_u32 input_count;
    lib_u32 line_count;
    lib_u32 request_count;
    lib_u32 lifecycle_count;
    lib_bool paused;
    lib_u32 marker;
} common_adapter;

static void common_adapter_consume(void *context,
    const common_machine_request *request)
{
    common_adapter *adapter = context;

    if (adapter != LIB_NULL && request != LIB_NULL) ++adapter->request_count;
}

static lib_bool common_adapter_paused(void *context)
{
    const common_adapter *adapter = context;
    return adapter != LIB_NULL && adapter->paused;
}

static lib_status common_adapter_debug(void *context,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result)
{
    common_adapter *adapter = context;

    if (adapter == LIB_NULL || request == LIB_NULL || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    out_result->value = adapter->marker + request->address;
    return LIB_STATUS_OK;
}

static lib_status common_adapter_input(void *context, const ui_input_event *event)
{
    common_adapter *adapter = context;

    if (adapter == LIB_NULL || event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++adapter->input_count;
    return LIB_STATUS_OK;
}

static lib_status common_adapter_line(void *context, const char *line)
{
    common_adapter *adapter = context;

    if (adapter == LIB_NULL || line == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++adapter->line_count;
    return LIB_STATUS_OK;
}

static void common_adapter_failure(void *context, lib_u64 source_identity,
    lib_status status)
{
    (void)context;
    (void)source_identity;
    (void)status;
}

static lib_status common_adapter_cli(void *context, const char *line,
    common_session_cli_result *out_result)
{
    common_adapter *adapter = context;

    if (adapter == LIB_NULL || line == LIB_NULL || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++adapter->line_count;
    out_result->keep_active = LIB_TRUE;
    out_result->lifecycle_request = COMMON_SESSION_LIFECYCLE_PAUSE;
    return LIB_STATUS_OK;
}

static lib_status common_adapter_lifecycle(void *context,
    common_session_lifecycle_request_kind request)
{
    common_adapter *adapter = context;

    if (adapter == LIB_NULL || request != COMMON_SESSION_LIFECYCLE_PAUSE)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++adapter->lifecycle_count;
    return LIB_STATUS_OK;
}

static int common_adapter_conform(common_adapter *adapter, lib_u32 run_id)
{
    common_machine *machine = LIB_NULL;
    common_session *session = LIB_NULL;
    common_ui *ui = LIB_NULL;
    common_machine_debug_lease lease;
    common_machine_debug_result debug_result;
    common_session_fact fact;
    common_session_plan plan;
    ui_frame frame = {0};
    common_ui_options ui_options = {0};
    common_machine_driver driver = {
        .consume_request = common_adapter_consume,
        .is_paused = common_adapter_paused,
        .execute_debug = common_adapter_debug,
        .context = adapter
    };

    ui_options.input_context = adapter;
    ui_options.input_sink = common_adapter_input;
    ui_options.console_line_context = adapter;
    ui_options.console_line_sink = common_adapter_line;
    ui_options.failure_context = adapter;
    ui_options.failure_sink = common_adapter_failure;
    ui_options.initial_window_title = "Common adapter conformance";
    ui_hotkey_registry_initialize(&ui_options.hotkeys);
    if (common_machine_create(&machine) != LIB_STATUS_OK ||
        common_machine_bind_driver(machine, &driver) != LIB_STATUS_OK ||
        common_machine_bind_run(machine, run_id) != LIB_STATUS_OK ||
        common_machine_submit(machine, &(common_machine_request) {
            .kind = COMMON_MACHINE_REQUEST_PAUSE, .run_id = run_id }) != LIB_STATUS_OK ||
        common_machine_observe_safe_point(machine) != LIB_STATUS_OK ||
        common_machine_debug_acquire(machine, &lease) != LIB_STATUS_OK ||
        common_machine_debug_execute_with_lease(machine, &lease,
            &(common_machine_debug_request) {
                .operation = COMMON_MACHINE_DEBUG_READ_LINEAR, .address = 4u },
            &debug_result) != LIB_STATUS_OK || debug_result.value != adapter->marker + 4u ||
        common_session_create(&session) != LIB_STATUS_OK ||
        common_session_set_target(session, COMMON_SESSION_TARGET_NONE) != LIB_STATUS_OK ||
        common_session_set_cli_provider(session, common_adapter_cli, adapter) != LIB_STATUS_OK ||
        common_session_set_lifecycle_sink(session, common_adapter_lifecycle, adapter) !=
            LIB_STATUS_OK ||
        common_session_begin_run(session, &plan) == 0u ||
        common_session_publish_console_line(session, "command") != LIB_STATUS_OK ||
        common_session_take(session, &fact, &frame, 0u) != LIB_STATUS_OK ||
        common_session_reduce_fact(session, &fact, &frame, &plan) != LIB_STATUS_OK ||
        common_ui_create(&ui, &ui_options) != LIB_STATUS_OK ||
        common_ui_get_target(ui) != COMMON_UI_TARGET_NONE) {
        common_ui_destroy(ui);
        common_session_destroy(session);
        common_machine_destroy(machine);
        return 1;
    }
    common_ui_destroy(ui);
    common_session_destroy(session);
    common_machine_destroy(machine);
    return adapter->request_count == 1u && adapter->line_count == 1u &&
        adapter->lifecycle_count == 1u ? 0 : 1;
}

int main(void)
{
    common_adapter first = { .paused = LIB_TRUE, .marker = 0x1000u };
    common_adapter second = { .paused = LIB_TRUE, .marker = 0x2000u };

    return common_adapter_conform(&first, 1u) ||
        common_adapter_conform(&second, 2u);
}
