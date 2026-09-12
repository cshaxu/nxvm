#include "common/ui/ui_interface.h"

#include "common/ui/console_host.h"
#include "lib/ui-console/console_interface.h"
#include "lib/ui-window/window_interface.h"

struct common_ui {
    common_ui_console_host *console_host;
    common_ui_input_sink input_sink;
    void *input_context;
    common_ui_failure_sink failure_sink;
    void *failure_context;
    ui_window *window;
    ui_console *console;
    ui_hotkey_registry hotkeys;
    char initial_window_title[UI_WINDOW_TITLE_CAPACITY];
    ui_frame frame;
    common_ui_target target;
};

static lib_status common_ui_input(void *opaque, const ui_input_event *event)
{
    common_ui *ui = opaque;

    return ui == LIB_NULL || ui->input_sink == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        ui->input_sink(ui->input_context, event);
}

static void common_ui_failure(void *opaque, lib_u64 source_identity,
    lib_status status)
{
    common_ui *ui = opaque;

    if (ui != LIB_NULL && ui->failure_sink != LIB_NULL)
        ui->failure_sink(ui->failure_context, source_identity, status);
}

static void common_ui_destroy_leaf(common_ui *ui)
{
    if (ui == LIB_NULL) return;
    if (ui->console != LIB_NULL) {
        (void)common_ui_console_host_release_guest(ui->console_host,
            ui_console_get_console(ui->console));
        ui_console_destroy(ui->console);
        ui->console = LIB_NULL;
    }
    if (ui->window != LIB_NULL) {
        ui_window_destroy(ui->window);
        ui->window = LIB_NULL;
    }
}

static lib_status common_ui_create_leaf(common_ui *ui, common_ui_target target)
{
    ui_component_options component = {0};
    lib_status status;

    component.input_context = ui;
    component.input_sink = common_ui_input;
    component.failure_context = ui;
    component.failure_sink = common_ui_failure;
    component.hotkeys = ui->hotkeys;
    if (target == COMMON_UI_TARGET_WINDOW) {
        ui_window_options options = {0};

        options.component = component;
        options.initial_title = ui->initial_window_title;
        options.initial_frozen = LIB_FALSE;
        status = ui_window_create(&ui->window, &options);
    } else if (target == COMMON_UI_TARGET_CONSOLE) {
        status = ui_console_create(&ui->console, &component);
        if (status == LIB_STATUS_OK)
            status = common_ui_console_host_claim_guest(ui->console_host,
                ui_console_get_console(ui->console));
    } else {
        return LIB_STATUS_OK;
    }
    if (status != LIB_STATUS_OK) common_ui_destroy_leaf(ui);
    return status;
}

static lib_status common_ui_publish_frame(common_ui *ui)
{
    if (ui->target == COMMON_UI_TARGET_NONE) return LIB_STATUS_OK;
    return ui->target == COMMON_UI_TARGET_WINDOW ?
        ui_window_publish_frame(ui->window, &ui->frame) :
        ui_console_publish_frame(ui->console, &ui->frame);
}

lib_status common_ui_create(common_ui **out_ui, const common_ui_options *options)
{
    common_ui *ui;
    lib_status status;

    if (out_ui == LIB_NULL || options == LIB_NULL || options->input_sink == LIB_NULL ||
        options->console_line_sink == LIB_NULL || options->failure_sink == LIB_NULL ||
        options->initial_window_title == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_ui = LIB_NULL;
    ui = lib_allocate_zero(1u, sizeof(*ui));
    if (ui == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    ui->input_sink = options->input_sink;
    ui->input_context = options->input_context;
    ui->failure_sink = options->failure_sink;
    ui->failure_context = options->failure_context;
    ui->hotkeys = options->hotkeys;
    if (lib_text_format(ui->initial_window_title, sizeof(ui->initial_window_title),
            "%s", options->initial_window_title) < 0) {
        lib_release(ui);
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    status = common_ui_console_host_create(&ui->console_host,
        options->console_line_context, options->console_line_sink);
    if (status != LIB_STATUS_OK) {
        lib_release(ui);
        return status;
    }
    *out_ui = ui;
    return LIB_STATUS_OK;
}

void common_ui_destroy(common_ui *ui)
{
    if (ui == LIB_NULL) return;
    common_ui_destroy_leaf(ui);
    common_ui_console_host_destroy(ui->console_host);
    lib_release(ui);
}

lib_status common_ui_request_console_line(common_ui *ui)
{
    return ui == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        common_ui_console_host_request_line(ui->console_host);
}

lib_status common_ui_write_console(common_ui *ui, const char *text)
{
    return ui == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        common_ui_console_host_write(ui->console_host, text);
}

lib_status common_ui_set_target(common_ui *ui, common_ui_target target)
{
    lib_status status;

    if (ui == LIB_NULL || target > COMMON_UI_TARGET_WINDOW)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (ui->target == target) return LIB_STATUS_OK;
    common_ui_destroy_leaf(ui);
    ui->target = COMMON_UI_TARGET_NONE;
    status = common_ui_create_leaf(ui, target);
    if (status != LIB_STATUS_OK) return status;
    ui->target = target;
    return ui->frame.valid ? common_ui_publish_frame(ui) : LIB_STATUS_OK;
}

common_ui_target common_ui_get_target(const common_ui *ui)
{ return ui == LIB_NULL ? COMMON_UI_TARGET_NONE : ui->target; }

lib_status common_ui_apply(common_ui *ui, const common_ui_plan *plan)
{
    lib_status status;

    if (ui == LIB_NULL || plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (plan->target_changed &&
        (status = common_ui_set_target(ui, plan->target)) != LIB_STATUS_OK) return status;
    if (plan->mouse_capturable_changed &&
        (status = common_ui_set_mouse_capturable(ui, plan->mouse_capturable)) != LIB_STATUS_OK)
        return status;
    if (plan->release_mouse &&
        (status = common_ui_release_mouse(ui)) != LIB_STATUS_OK) return status;
    if (!plan->frame_ready) return LIB_STATUS_OK;
    ui->frame = plan->frame;
    return common_ui_publish_frame(ui);
}

lib_status common_ui_set_window_title(common_ui *ui, const char *title)
{
    return ui == LIB_NULL || title == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ui->window == LIB_NULL ? LIB_STATUS_OK : ui_window_set_title(ui->window, title);
}

lib_status common_ui_set_mouse_capturable(common_ui *ui, lib_bool capturable)
{
    return ui == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : ui->window == LIB_NULL ?
        LIB_STATUS_OK : capturable ? ui_window_unfreeze(ui->window) :
        ui_window_freeze(ui->window);
}

lib_status common_ui_release_mouse(common_ui *ui)
{
    return ui == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : ui->window == LIB_NULL ?
        LIB_STATUS_OK : ui_window_release_mouse(ui->window);
}
