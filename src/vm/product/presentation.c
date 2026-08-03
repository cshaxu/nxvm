#include "type.h"

#include "vm/product/presentation.h"

C_VOID vm_product_presentation_initialize(
    vm_product_presentation *presentation)
{
    if (presentation == STD_NULL) return;
    core_machine_keyboard_queue_initialize(&presentation->pending_input);
    core_machine_text_snapshot_initialize(&presentation->published_text);
    presentation->command_boundary_open = 0;
}

ntvdm64_status vm_product_presentation_enqueue_input(
    vm_product_presentation *presentation, uint16_t scan_code)
{
    if (presentation == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    return core_machine_keyboard_queue_push(&presentation->pending_input, scan_code);
}

ntvdm64_status vm_product_presentation_open_command_boundary(
    vm_product_presentation *presentation)
{
    if (presentation == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    if (presentation->command_boundary_open) return NTVDM64_STATUS_INVALID_STATE;
    presentation->command_boundary_open = 1;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_product_presentation_apply_input(
    vm_product_presentation *presentation,
    vm_product_input_consumer consumer, C_VOID *context)
{
    uint16_t scan_code;
    ntvdm64_status status;

    if (presentation == STD_NULL || consumer == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    if (!presentation->command_boundary_open) return NTVDM64_STATUS_INVALID_STATE;
    while ((status = core_machine_keyboard_queue_pop(&presentation->pending_input,
                                                   &scan_code)) == NTVDM64_STATUS_OK) {
        status = consumer(context, scan_code);
        if (status != NTVDM64_STATUS_OK) return status;
    }
    return status == NTVDM64_STATUS_UNSUPPORTED ? NTVDM64_STATUS_OK : status;
}

ntvdm64_status vm_product_presentation_publish_text(
    vm_product_presentation *presentation,
    const core_machine_text_snapshot *snapshot)
{
    if (presentation == STD_NULL || snapshot == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    if (!presentation->command_boundary_open) return NTVDM64_STATUS_INVALID_STATE;
    return core_machine_text_snapshot_copy(snapshot, &presentation->published_text);
}

ntvdm64_status vm_product_presentation_capture_text(
    const vm_product_presentation *presentation,
    core_machine_text_snapshot *out_snapshot)
{
    if (presentation == STD_NULL || out_snapshot == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    return core_machine_text_snapshot_copy(&presentation->published_text, out_snapshot);
}

C_VOID vm_product_presentation_close_command_boundary(
    vm_product_presentation *presentation)
{
    if (presentation != STD_NULL) presentation->command_boundary_open = 0;
}
