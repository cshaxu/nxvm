#include "common/ui/ui_interface.h"

#include "common/ui/console_host.h"
#include "lib/kvm-console/console_interface.h"
#include "lib/kvm-window/window_interface.h"

struct common_ui {
    common_ui_console_host *console_host;
    common_ui_input_sink input_sink;
    void *input_context;
    common_ui_failure_sink failure_sink;
    void *failure_context;
    kvm_window *window;
    kvm_console *console;
    kvm_hotkey_registry hotkeys;
    char initial_window_title[KVM_WINDOW_TITLE_CAPACITY];
    kvm_frame frame;
    lib_bool raw_console_current;
};

static int common_ui_input(void *opaque, const kvm_input_event *event)
{
    common_ui *ui = opaque;

    return ui != LIB_NULL && ui->input_sink != LIB_NULL &&
        ui->input_sink(ui->input_context, event) == LIB_STATUS_OK;
}

static void common_ui_failure(void *opaque, lib_u64 source_identity,
    lib_status status)
{
    common_ui *ui = opaque;

    if (ui != LIB_NULL && ui->failure_sink != LIB_NULL)
        ui->failure_sink(ui->failure_context, source_identity, status);
}

static common_ui_surface_facts common_ui_surface_facts_of(const common_ui *ui)
{
    common_ui_surface_facts facts = {0};

    if (ui == LIB_NULL) return facts;
    facts.window_exists = ui->window != LIB_NULL;
    facts.raw_console_exists = ui->console != LIB_NULL;
    facts.raw_console_current = facts.raw_console_exists && ui->raw_console_current;
    return facts;
}

static void common_ui_destroy_raw_console(common_ui *ui)
{
    if (ui == LIB_NULL || ui->console == LIB_NULL) return;
    if (ui->console != LIB_NULL) {
        if (ui->raw_console_current)
            (void)common_ui_console_host_release_guest(ui->console_host,
                kvm_console_get_console(ui->console));
        kvm_console_destroy(ui->console);
        ui->console = LIB_NULL;
        ui->raw_console_current = LIB_FALSE;
    }
}

static void common_ui_destroy_window(common_ui *ui)
{
    if (ui != LIB_NULL && ui->window != LIB_NULL) {
        kvm_window_destroy(ui->window);
        ui->window = LIB_NULL;
    }
}

static lib_status common_ui_create_window(common_ui *ui)
{
    kvm_component_options component = {0};
    kvm_window_options options = {0};

    if (ui == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (ui->window != LIB_NULL) return LIB_STATUS_OK;
    component.input_context = ui;
    component.input_sink = common_ui_input;
    component.failure_context = ui;
    component.failure_sink = common_ui_failure;
    component.hotkeys = ui->hotkeys;
    options.component = component;
    options.initial_title = ui->initial_window_title;
    options.initial_frozen = LIB_FALSE;
    return kvm_window_create(&ui->window, &options);
}

static lib_status common_ui_create_raw_console(common_ui *ui)
{
    kvm_component_options component = {0};

    if (ui == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (ui->console != LIB_NULL) return LIB_STATUS_OK;
    component.input_context = ui;
    component.input_sink = common_ui_input;
    component.failure_context = ui;
    component.failure_sink = common_ui_failure;
    component.hotkeys = ui->hotkeys;
    return kvm_console_create(&ui->console, &component);
}

static lib_status common_ui_publish_frame(common_ui *ui)
{
    lib_status status;

    if (ui == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (ui->window != LIB_NULL &&
        (status = kvm_window_publish_frame(ui->window, &ui->frame)) != LIB_STATUS_OK)
        return status;
    return ui->console == LIB_NULL ? LIB_STATUS_OK :
        kvm_console_publish_frame(ui->console, &ui->frame);
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
    if (lib_text_length(options->initial_window_title) >=
            sizeof(ui->initial_window_title)) {
        lib_release(ui);
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    lib_memory_copy(ui->initial_window_title, options->initial_window_title,
        lib_text_length(options->initial_window_title) + 1u);
    status = common_ui_console_host_create(&ui->console_host,
        options->console_line_context, options->console_line_sink,
        ui, common_ui_failure);
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
    common_ui_destroy_raw_console(ui);
    common_ui_destroy_window(ui);
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

common_ui_surface_facts common_ui_get_surface_facts(const common_ui *ui)
{ return common_ui_surface_facts_of(ui); }

lib_status common_ui_apply_action(common_ui *ui, const common_ui_action *action,
    common_ui_completion *out_completion)
{
    lib_status status = LIB_STATUS_OK;

    if (ui == LIB_NULL || action == LIB_NULL || out_completion == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    out_completion->action = action->kind;
    out_completion->facts = common_ui_surface_facts_of(ui);
    switch (action->kind) {
    case COMMON_UI_ACTION_CREATE_WINDOW:
        status = common_ui_create_window(ui);
        break;
    case COMMON_UI_ACTION_DESTROY_WINDOW:
        common_ui_destroy_window(ui);
        break;
    case COMMON_UI_ACTION_CREATE_RAW_CONSOLE:
        status = common_ui_create_raw_console(ui);
        break;
    case COMMON_UI_ACTION_DESTROY_RAW_CONSOLE:
        common_ui_destroy_raw_console(ui);
        break;
    case COMMON_UI_ACTION_BIND_RAW_CONSOLE:
        if (ui->console == LIB_NULL) return LIB_STATUS_INVALID_STATE;
        status = common_ui_console_host_claim_guest(ui->console_host,
            kvm_console_get_console(ui->console));
        if (status == LIB_STATUS_OK) ui->raw_console_current = LIB_TRUE;
        break;
    case COMMON_UI_ACTION_BIND_MONITOR_CONSOLE:
        if (ui->console != LIB_NULL) status = common_ui_console_host_release_guest(
            ui->console_host, kvm_console_get_console(ui->console));
        if (status == LIB_STATUS_OK) ui->raw_console_current = LIB_FALSE;
        break;
    case COMMON_UI_ACTION_SET_WINDOW_TITLE:
        if (action->value.title == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
        if (ui->window != LIB_NULL)
            status = kvm_window_set_title(ui->window, action->value.title);
        break;
    case COMMON_UI_ACTION_SET_MOUSE_CAPTURABLE:
        if (ui->window != LIB_NULL) status = action->value.mouse_capturable ?
            kvm_window_unfreeze(ui->window) : kvm_window_freeze(ui->window);
        break;
    case COMMON_UI_ACTION_RELEASE_MOUSE:
        if (ui->window != LIB_NULL) status = kvm_window_release_mouse(ui->window);
        break;
    default:
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (status != LIB_STATUS_OK) return status;
    out_completion->facts = common_ui_surface_facts_of(ui);
    return LIB_STATUS_OK;
}

lib_status common_ui_apply(common_ui *ui, const common_ui_plan *plan)
{
    common_ui_action action = {0};
    common_ui_completion completion;
    lib_status status;

    if (ui == LIB_NULL || plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (plan->mouse_capturable_changed) {
        action.kind = COMMON_UI_ACTION_SET_MOUSE_CAPTURABLE;
        action.value.mouse_capturable = plan->mouse_capturable;
        status = common_ui_apply_action(ui, &action, &completion);
        if (status != LIB_STATUS_OK) return status;
    }
    if (plan->release_mouse) {
        action.kind = COMMON_UI_ACTION_RELEASE_MOUSE;
        status = common_ui_apply_action(ui, &action, &completion);
        if (status != LIB_STATUS_OK) return status;
    }
    if (!plan->frame_ready) return LIB_STATUS_OK;
    ui->frame = plan->frame;
    return common_ui_publish_frame(ui);
}

lib_status common_ui_set_window_title(common_ui *ui, const char *title)
{
    common_ui_action action = { COMMON_UI_ACTION_SET_WINDOW_TITLE, {0} };
    common_ui_completion completion;
    action.value.title = title;
    return common_ui_apply_action(ui, &action, &completion);
}
