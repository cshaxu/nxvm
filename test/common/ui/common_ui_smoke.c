#include "common/ui/ui_interface.h"

static lib_status common_ui_smoke_input(void *context, const ui_input_event *event)
{
    (void)context;
    return event == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

static lib_status common_ui_smoke_line(void *context, const char *line)
{
    (void)context;
    return line == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

static void common_ui_smoke_failure(void *context, lib_u64 source_identity,
    lib_status status)
{
    (void)context;
    (void)source_identity;
    (void)status;
}

int main(void)
{
    common_ui_options options = {0};
    common_ui_plan plan = {0};
    common_ui *ui;

    options.input_sink = common_ui_smoke_input;
    options.console_line_sink = common_ui_smoke_line;
    options.initial_window_title = "Common UI smoke";
    ui_hotkey_registry_initialize(&options.hotkeys);
    if (common_ui_create(&ui, &options) != LIB_STATUS_INVALID_ARGUMENT) return 1;
    options.failure_sink = common_ui_smoke_failure;
    if (common_ui_create(&ui, &options) != LIB_STATUS_OK ||
        common_ui_get_target(ui) != COMMON_UI_TARGET_NONE) return 2;
    plan.frame_ready = LIB_TRUE;
    plan.frame.valid = LIB_TRUE;
    plan.frame.sequence = 1u;
    if (common_ui_apply(ui, &plan) != LIB_STATUS_OK) {
        common_ui_destroy(ui);
        return 3;
    }
    common_ui_destroy(ui);
    return 0;
}
