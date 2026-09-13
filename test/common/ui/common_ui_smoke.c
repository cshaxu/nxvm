#include "common/ui/ui_interface.h"

void common_ui_fake_reset(void);
void common_ui_fake_fail_window_create(lib_status status);
void common_ui_fake_fail_claim(lib_status status);
int common_ui_fake_emit_window_input(const ui_input_event *event);

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
    common_ui_action action = {0};
    common_ui_completion completion = {0};
    common_ui_surface_facts facts;
    ui_input_event event = {0};
    common_ui *ui;

    options.input_sink = common_ui_smoke_input;
    options.console_line_sink = common_ui_smoke_line;
    options.initial_window_title = "Common UI smoke";
    ui_hotkey_registry_initialize(&options.hotkeys);
    if (common_ui_create(&ui, &options) != LIB_STATUS_INVALID_ARGUMENT) return 1;
    options.failure_sink = common_ui_smoke_failure;
    common_ui_fake_reset();
    if (common_ui_create(&ui, &options) != LIB_STATUS_OK) return 2;
    facts = common_ui_get_surface_facts(ui);
    if (facts.window_exists || facts.raw_console_exists || facts.raw_console_current)
        return 3;
    action.kind = COMMON_UI_ACTION_BIND_MONITOR_CONSOLE;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK ||
        completion.action != COMMON_UI_ACTION_BIND_MONITOR_CONSOLE ||
        completion.facts.window_exists || completion.facts.raw_console_exists ||
        completion.facts.raw_console_current) {
        common_ui_destroy(ui);
        return 4;
    }
    action.kind = COMMON_UI_ACTION_SET_WINDOW_TITLE;
    action.value.title = "ignored without a window";
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK) {
        common_ui_destroy(ui);
        return 5;
    }
    action.kind = COMMON_UI_ACTION_SET_MOUSE_CAPTURABLE;
    action.value.mouse_capturable = LIB_FALSE;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK) {
        common_ui_destroy(ui);
        return 6;
    }
    action.kind = COMMON_UI_ACTION_CREATE_WINDOW;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK ||
        !completion.facts.window_exists || completion.facts.raw_console_exists) {
        common_ui_destroy(ui);
        return 7;
    }
    event.type = UI_EVENT_KEY;
    event.data.key.pressed = LIB_TRUE;
    if (!common_ui_fake_emit_window_input(&event)) {
        common_ui_destroy(ui);
        return 71;
    }
    action.kind = COMMON_UI_ACTION_DESTROY_WINDOW;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK ||
        completion.facts.window_exists) {
        common_ui_destroy(ui);
        return 8;
    }
    common_ui_fake_fail_window_create(LIB_STATUS_IO_ERROR);
    action.kind = COMMON_UI_ACTION_CREATE_WINDOW;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_IO_ERROR ||
        completion.action != COMMON_UI_ACTION_CREATE_WINDOW ||
        common_ui_get_surface_facts(ui).window_exists) {
        common_ui_destroy(ui);
        return 9;
    }
    common_ui_fake_reset();
    action.kind = COMMON_UI_ACTION_CREATE_RAW_CONSOLE;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK ||
        !completion.facts.raw_console_exists || completion.facts.raw_console_current) {
        common_ui_destroy(ui);
        return 10;
    }
    common_ui_fake_fail_claim(LIB_STATUS_IO_ERROR);
    action.kind = COMMON_UI_ACTION_BIND_RAW_CONSOLE;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_IO_ERROR ||
        common_ui_get_surface_facts(ui).raw_console_current) {
        common_ui_destroy(ui);
        return 11;
    }
    common_ui_fake_reset();
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK ||
        !completion.facts.raw_console_current) {
        common_ui_destroy(ui);
        return 12;
    }
    action.kind = COMMON_UI_ACTION_BIND_MONITOR_CONSOLE;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK ||
        completion.facts.raw_console_current) {
        common_ui_destroy(ui);
        return 13;
    }
    action.kind = COMMON_UI_ACTION_DESTROY_RAW_CONSOLE;
    if (common_ui_apply_action(ui, &action, &completion) != LIB_STATUS_OK ||
        completion.facts.raw_console_exists) {
        common_ui_destroy(ui);
        return 14;
    }
    plan.frame_ready = LIB_TRUE;
    plan.frame.valid = LIB_TRUE;
    plan.frame.sequence = 1u;
    if (common_ui_apply(ui, &plan) != LIB_STATUS_OK) {
        common_ui_destroy(ui);
        return 15;
    }
    common_ui_destroy(ui);
    return 0;
}
