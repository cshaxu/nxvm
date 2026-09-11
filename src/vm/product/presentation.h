#ifndef VM_PRODUCT_PRESENTATION_H
#define VM_PRODUCT_PRESENTATION_H

#include "type.h"

#include "lib/ui-base/event_interface.h"
#include "vm/events/machine_event.h"

typedef struct vm_product_console_host vm_product_console_host;
typedef struct vm_product_presentation vm_product_presentation;

typedef enum vm_product_presentation_target {
    VM_PRODUCT_PRESENTATION_NONE,
    VM_PRODUCT_PRESENTATION_CONSOLE,
    VM_PRODUCT_PRESENTATION_WINDOW
} vm_product_presentation_target;

typedef type_status (*vm_product_presentation_event_sink)(C_VOID *context,
    const ui_input_event *event);

type_status vm_product_presentation_create(vm_product_presentation **out_presentation,
    vm_product_console_host *console_host,
    vm_product_presentation_event_sink event_sink, C_VOID *event_context);
C_VOID vm_product_presentation_destroy(vm_product_presentation *presentation);
type_status vm_product_presentation_set_target(vm_product_presentation *presentation,
    vm_product_presentation_target target);
vm_product_presentation_target vm_product_presentation_get_target(
    const vm_product_presentation *presentation);
type_status vm_product_presentation_set_window_title(
    vm_product_presentation *presentation, const C_CHAR *title);
type_status vm_product_presentation_set_mouse_capturable(
    vm_product_presentation *presentation, C_INT capturable);
type_status vm_product_presentation_release_mouse(
    vm_product_presentation *presentation);
type_status vm_product_presentation_publish_frame(
    vm_product_presentation *presentation,
    const vm_machine_display_event *frame);

#endif
