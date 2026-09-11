#include "type.h"

#include "lib/ui-base/hotkey_interface.h"
#include "lib/ui-console/console_interface.h"
#include "lib/ui-window/window_interface.h"
#include "vm/product/console_host.h"
#include "vm/product/presentation.h"
#include "vm/product/presentation_frame.h"

struct vm_product_presentation {
    vm_product_console_host *console_host;
    vm_product_presentation_event_sink event_sink;
    C_VOID *event_context;
    ui_window *window;
    ui_console *console;
    ui_hotkey_registry hotkeys;
    ui_frame frame;
    vm_product_presentation_target target;
};

static C_INT vm_product_presentation_input(C_VOID *opaque,
    const ui_input_event *event)
{
    vm_product_presentation *presentation = opaque;

    return presentation != STD_NULL && presentation->event_sink != STD_NULL &&
        presentation->event_sink(presentation->event_context, event) == TYPE_STATUS_OK;
}

static C_VOID vm_product_presentation_destroy_leaf(
    vm_product_presentation *presentation)
{
    if (presentation == STD_NULL) return;
    if (presentation->console != STD_NULL) {
        (C_VOID)vm_product_console_host_release_guest(presentation->console_host,
            ui_console_get_console(presentation->console));
        ui_console_destroy(presentation->console);
        presentation->console = STD_NULL;
    }
    if (presentation->window != STD_NULL) {
        ui_window_destroy(presentation->window);
        presentation->window = STD_NULL;
    }
}

static type_status vm_product_presentation_create_leaf(
    vm_product_presentation *presentation, vm_product_presentation_target target)
{
    ui_component_options component = {0};
    lib_status status;

    component.input_context = presentation;
    component.input_sink = vm_product_presentation_input;
    component.hotkeys = presentation->hotkeys;
    if (target == VM_PRODUCT_PRESENTATION_WINDOW) {
        ui_window_options options = {0};

        options.component = component;
        options.initial_title = "NXVM (Running)";
        options.initial_frozen = LIB_FALSE;
        status = ui_window_create(&presentation->window, &options);
    } else if (target == VM_PRODUCT_PRESENTATION_CONSOLE) {
        status = ui_console_create(&presentation->console, &component);
        if (status == LIB_STATUS_OK) {
            status = (lib_status)vm_product_console_host_claim_guest(
                presentation->console_host,
                ui_console_get_console(presentation->console));
        }
    } else {
        return TYPE_STATUS_OK;
    }
    if (status != LIB_STATUS_OK) vm_product_presentation_destroy_leaf(presentation);
    return (type_status)status;
}

type_status vm_product_presentation_create(vm_product_presentation **out_presentation,
    vm_product_console_host *console_host,
    vm_product_presentation_event_sink event_sink, C_VOID *event_context)
{
    vm_product_presentation *presentation;

    if (out_presentation == STD_NULL || console_host == STD_NULL || event_sink == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    *out_presentation = STD_NULL;
    presentation = STD_CALLOC(1u, sizeof(*presentation));
    if (presentation == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    presentation->console_host = console_host;
    presentation->event_sink = event_sink;
    presentation->event_context = event_context;
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

C_VOID vm_product_presentation_destroy(vm_product_presentation *presentation)
{
    if (presentation == STD_NULL) return;
    vm_product_presentation_destroy_leaf(presentation);
    STD_FREE(presentation);
}

type_status vm_product_presentation_set_target(vm_product_presentation *presentation,
    vm_product_presentation_target target)
{
    type_status status;

    if (presentation == STD_NULL || target > VM_PRODUCT_PRESENTATION_WINDOW)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (presentation->target == target) return TYPE_STATUS_OK;
    vm_product_presentation_destroy_leaf(presentation);
    presentation->target = VM_PRODUCT_PRESENTATION_NONE;
    status = vm_product_presentation_create_leaf(presentation, target);
    if (status != TYPE_STATUS_OK) return status;
    presentation->target = target;
    if (presentation->frame.valid) {
        return target == VM_PRODUCT_PRESENTATION_WINDOW ?
            (type_status)ui_window_publish_frame(presentation->window, &presentation->frame) :
            target == VM_PRODUCT_PRESENTATION_CONSOLE ?
            (type_status)ui_console_publish_frame(presentation->console, &presentation->frame) :
            TYPE_STATUS_OK;
    }
    return TYPE_STATUS_OK;
}

vm_product_presentation_target vm_product_presentation_get_target(
    const vm_product_presentation *presentation)
{ return presentation == STD_NULL ? VM_PRODUCT_PRESENTATION_NONE : presentation->target; }

type_status vm_product_presentation_set_window_title(
    vm_product_presentation *presentation, const C_CHAR *title)
{
    return presentation == STD_NULL || title == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        presentation->window == STD_NULL ? TYPE_STATUS_OK :
        (type_status)ui_window_set_title(presentation->window, title);
}

type_status vm_product_presentation_set_mouse_capturable(
    vm_product_presentation *presentation, C_INT capturable)
{
    return presentation == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        presentation->window == STD_NULL ? TYPE_STATUS_OK : (type_status)(capturable ?
            ui_window_unfreeze(presentation->window) : ui_window_freeze(presentation->window));
}

type_status vm_product_presentation_release_mouse(
    vm_product_presentation *presentation)
{
    return presentation == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        presentation->window == STD_NULL ? TYPE_STATUS_OK :
        (type_status)ui_window_release_mouse(presentation->window);
}

type_status vm_product_presentation_publish_frame(
    vm_product_presentation *presentation,
    const vm_machine_display_event *frame)
{
    type_status status;
    vm_product_presentation_target target;

    if (presentation == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = vm_product_presentation_frame_from_core(frame, &presentation->frame);
    if (status != TYPE_STATUS_OK) return status;
    target = presentation->target;
    if (target == VM_PRODUCT_PRESENTATION_NONE) return TYPE_STATUS_OK;
    return target == VM_PRODUCT_PRESENTATION_WINDOW ?
        (type_status)ui_window_publish_frame(presentation->window, &presentation->frame) :
        (type_status)ui_console_publish_frame(presentation->console, &presentation->frame);
}
