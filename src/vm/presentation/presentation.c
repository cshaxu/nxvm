#include "type.h"

#include "lib/ui-base/hotkey_interface.h"
#include "lib/ui-console/console_interface.h"
#include "lib/ui-window/window_interface.h"
#include "vm/presentation/console_host.h"
#include "vm/presentation/presentation.h"
#include "vm/presentation/frame.h"

struct vm_presentation {
    vm_presentation_console_host *console_host;
    vm_presentation_event_sink event_sink;
    C_VOID *event_context;
    ui_window *window;
    ui_console *console;
    ui_hotkey_registry hotkeys;
    ui_frame frame;
    vm_presentation_surface target;
};

static C_INT vm_presentation_input(C_VOID *opaque,
    const ui_input_event *event)
{
    vm_presentation *presentation = opaque;

    return presentation != STD_NULL && presentation->event_sink != STD_NULL &&
        presentation->event_sink(presentation->event_context, event) == TYPE_STATUS_OK;
}

static C_VOID vm_presentation_destroy_leaf(
    vm_presentation *presentation)
{
    if (presentation == STD_NULL) return;
    if (presentation->console != STD_NULL) {
        (C_VOID)vm_presentation_console_host_release_guest(presentation->console_host,
            ui_console_get_console(presentation->console));
        ui_console_destroy(presentation->console);
        presentation->console = STD_NULL;
    }
    if (presentation->window != STD_NULL) {
        ui_window_destroy(presentation->window);
        presentation->window = STD_NULL;
    }
}

static type_status vm_presentation_create_leaf(
    vm_presentation *presentation, vm_presentation_surface target)
{
    ui_component_options component = {0};
    lib_status status;

    component.input_context = presentation;
    component.input_sink = vm_presentation_input;
    component.hotkeys = presentation->hotkeys;
    if (target == VM_PRESENTATION_SURFACE_WINDOW) {
        ui_window_options options = {0};

        options.component = component;
        options.initial_title = "NXVM (Running)";
        options.initial_frozen = LIB_FALSE;
        status = ui_window_create(&presentation->window, &options);
    } else if (target == VM_PRESENTATION_SURFACE_CONSOLE) {
        status = ui_console_create(&presentation->console, &component);
        if (status == LIB_STATUS_OK) {
            status = (lib_status)vm_presentation_console_host_claim_guest(
                presentation->console_host,
                ui_console_get_console(presentation->console));
        }
    } else {
        return TYPE_STATUS_OK;
    }
    if (status != LIB_STATUS_OK) vm_presentation_destroy_leaf(presentation);
    return (type_status)status;
}

type_status vm_presentation_create(vm_presentation **out_presentation,
    vm_presentation_event_sink event_sink, C_VOID *event_context,
    type_status (*line_sink)(C_VOID *context, const C_CHAR *text),
    C_VOID *line_context)
{
    vm_presentation *presentation;

    if (out_presentation == STD_NULL || event_sink == STD_NULL || line_sink == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    *out_presentation = STD_NULL;
    presentation = STD_CALLOC(1u, sizeof(*presentation));
    if (presentation == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    presentation->event_sink = event_sink;
    presentation->event_context = event_context;
    if (vm_presentation_console_host_create(&presentation->console_host,
            line_context, line_sink) != TYPE_STATUS_OK) {
        STD_FREE(presentation);
        return TYPE_STATUS_INVALID_STATE;
    }
    ui_hotkey_registry_initialize(&presentation->hotkeys);
    (C_VOID)ui_hotkey_registry_register(&presentation->hotkeys, 'P',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "pause");
    (C_VOID)ui_hotkey_registry_register(&presentation->hotkeys, 'D',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "cad");
    (C_VOID)ui_hotkey_registry_register(&presentation->hotkeys, 'F',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "alt-enter");
    (C_VOID)ui_hotkey_registry_register(&presentation->hotkeys, 'M',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "release-mouse");
    *out_presentation = presentation;
    return TYPE_STATUS_OK;
}

C_VOID vm_presentation_destroy(vm_presentation *presentation)
{
    if (presentation == STD_NULL) return;
    vm_presentation_destroy_leaf(presentation);
    vm_presentation_console_host_destroy(presentation->console_host);
    STD_FREE(presentation);
}

type_status vm_presentation_request_console_line(vm_presentation *presentation)
{
    return presentation == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        vm_presentation_console_host_request_line(presentation->console_host);
}

type_status vm_presentation_write_console(vm_presentation *presentation,
    const C_CHAR *text)
{
    return presentation == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        vm_presentation_console_host_write(presentation->console_host, text);
}

type_status vm_presentation_set_target(vm_presentation *presentation,
    vm_presentation_surface target)
{
    type_status status;

    if (presentation == STD_NULL || target > VM_PRESENTATION_SURFACE_WINDOW)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (presentation->target == target) return TYPE_STATUS_OK;
    vm_presentation_destroy_leaf(presentation);
    presentation->target = VM_PRESENTATION_SURFACE_NONE;
    status = vm_presentation_create_leaf(presentation, target);
    if (status != TYPE_STATUS_OK) return status;
    presentation->target = target;
    if (presentation->frame.valid) {
        return target == VM_PRESENTATION_SURFACE_WINDOW ?
            (type_status)ui_window_publish_frame(presentation->window, &presentation->frame) :
            target == VM_PRESENTATION_SURFACE_CONSOLE ?
            (type_status)ui_console_publish_frame(presentation->console, &presentation->frame) :
            TYPE_STATUS_OK;
    }
    return TYPE_STATUS_OK;
}

vm_presentation_surface vm_presentation_get_target(
    const vm_presentation *presentation)
{ return presentation == STD_NULL ? VM_PRESENTATION_SURFACE_NONE : presentation->target; }

type_status vm_presentation_apply_plan(vm_presentation *presentation,
    const vm_presentation_plan *plan)
{
    type_status status;

    if (presentation == STD_NULL || plan == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (plan->target_changed &&
        (status = vm_presentation_set_target(presentation, plan->target)) !=
            TYPE_STATUS_OK)
        return status;
    if (plan->title_changed &&
        (status = vm_presentation_set_window_title(presentation, plan->title)) !=
            TYPE_STATUS_OK)
        return status;
    if (plan->mouse_capturable_changed &&
        (status = vm_presentation_set_mouse_capturable(presentation,
            plan->mouse_capturable)) != TYPE_STATUS_OK)
        return status;
    if (plan->release_mouse &&
        (status = vm_presentation_release_mouse(presentation)) != TYPE_STATUS_OK)
        return status;
    return plan->frame_ready ? vm_presentation_publish_frame(presentation,
        &plan->frame) : TYPE_STATUS_OK;
}

type_status vm_presentation_set_window_title(
    vm_presentation *presentation, const C_CHAR *title)
{
    return presentation == STD_NULL || title == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        presentation->window == STD_NULL ? TYPE_STATUS_OK :
        (type_status)ui_window_set_title(presentation->window, title);
}

type_status vm_presentation_set_mouse_capturable(
    vm_presentation *presentation, C_INT capturable)
{
    return presentation == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        presentation->window == STD_NULL ? TYPE_STATUS_OK : (type_status)(capturable ?
            ui_window_unfreeze(presentation->window) : ui_window_freeze(presentation->window));
}

type_status vm_presentation_release_mouse(
    vm_presentation *presentation)
{
    return presentation == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        presentation->window == STD_NULL ? TYPE_STATUS_OK :
        (type_status)ui_window_release_mouse(presentation->window);
}

type_status vm_presentation_publish_frame(
    vm_presentation *presentation,
    const vm_machine_display_event *frame)
{
    type_status status;
    vm_presentation_surface target;

    if (presentation == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = vm_presentation_frame_from_core(frame, &presentation->frame);
    if (status != TYPE_STATUS_OK) return status;
    target = presentation->target;
    if (target == VM_PRESENTATION_SURFACE_NONE) return TYPE_STATUS_OK;
    return target == VM_PRESENTATION_SURFACE_WINDOW ?
        (type_status)ui_window_publish_frame(presentation->window, &presentation->frame) :
        (type_status)ui_console_publish_frame(presentation->console, &presentation->frame);
}
