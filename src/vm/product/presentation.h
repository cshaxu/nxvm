#ifndef VM_PRODUCT_PRESENTATION_H
#define VM_PRODUCT_PRESENTATION_H

#include "type.h"

#include "core/machine/presentation_interface.h"

typedef struct vm_product_presentation {
    core_machine_keyboard_queue pending_input;
    core_machine_text_snapshot published_text;
    C_INT command_boundary_open;
} vm_product_presentation;

typedef ntvdm64_status (*vm_product_input_consumer)(
    C_VOID *context, uint16_t scan_code);

C_VOID vm_product_presentation_initialize(
    vm_product_presentation *presentation);
ntvdm64_status vm_product_presentation_enqueue_input(
    vm_product_presentation *presentation, uint16_t scan_code);
ntvdm64_status vm_product_presentation_open_command_boundary(
    vm_product_presentation *presentation);
ntvdm64_status vm_product_presentation_apply_input(
    vm_product_presentation *presentation,
    vm_product_input_consumer consumer, C_VOID *context);
ntvdm64_status vm_product_presentation_publish_text(
    vm_product_presentation *presentation,
    const core_machine_text_snapshot *snapshot);
ntvdm64_status vm_product_presentation_capture_text(
    const vm_product_presentation *presentation,
    core_machine_text_snapshot *out_snapshot);
C_VOID vm_product_presentation_close_command_boundary(
    vm_product_presentation *presentation);

#endif
