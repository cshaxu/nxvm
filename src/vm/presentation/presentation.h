#ifndef VM_PRESENTATION_H
#define VM_PRESENTATION_H

#include "type.h"

#include "lib/ui-base/event_interface.h"
#include "vm/events/machine_event.h"
#include "vm/events/presentation_plan.h"

typedef struct vm_presentation vm_presentation;

typedef type_status (*vm_presentation_event_sink)(C_VOID *context,
    const ui_input_event *event);

type_status vm_presentation_create(vm_presentation **out_presentation,
    vm_presentation_event_sink event_sink, C_VOID *event_context,
    type_status (*line_sink)(C_VOID *context, const C_CHAR *text),
    C_VOID *line_context);
C_VOID vm_presentation_destroy(vm_presentation *presentation);
type_status vm_presentation_request_console_line(vm_presentation *presentation);
type_status vm_presentation_write_console(vm_presentation *presentation,
    const C_CHAR *text);
type_status vm_presentation_set_target(vm_presentation *presentation,
    vm_presentation_surface target);
vm_presentation_surface vm_presentation_get_target(
    const vm_presentation *presentation);
type_status vm_presentation_apply_plan(vm_presentation *presentation,
    const vm_presentation_plan *plan);
type_status vm_presentation_set_window_title(
    vm_presentation *presentation, const C_CHAR *title);
type_status vm_presentation_set_mouse_capturable(
    vm_presentation *presentation, C_INT capturable);
type_status vm_presentation_release_mouse(
    vm_presentation *presentation);
type_status vm_presentation_publish_frame(
    vm_presentation *presentation,
    const vm_machine_display_event *frame);

#endif
