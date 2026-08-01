#include "vm/product/presentation.h"

void nxvm_product_nxvm_presentation_initialize(
    nxvm_product_nxvm_presentation *presentation)
{
    if (presentation == NULL) return;
    nxvm_core_keyboard_queue_initialize(&presentation->pending_input);
    nxvm_core_text_snapshot_initialize(&presentation->published_text);
    presentation->command_boundary_open = 0;
}

nxvm_core_status nxvm_product_nxvm_presentation_enqueue_input(
    nxvm_product_nxvm_presentation *presentation, uint16_t scan_code)
{
    if (presentation == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    return nxvm_core_keyboard_queue_push(&presentation->pending_input, scan_code);
}

nxvm_core_status nxvm_product_nxvm_presentation_open_command_boundary(
    nxvm_product_nxvm_presentation *presentation)
{
    if (presentation == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (presentation->command_boundary_open) return NXVM_CORE_STATUS_INVALID_STATE;
    presentation->command_boundary_open = 1;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_presentation_apply_input(
    nxvm_product_nxvm_presentation *presentation,
    nxvm_product_nxvm_input_consumer consumer, void *context)
{
    uint16_t scan_code;
    nxvm_core_status status;

    if (presentation == NULL || consumer == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (!presentation->command_boundary_open) return NXVM_CORE_STATUS_INVALID_STATE;
    while ((status = nxvm_core_keyboard_queue_pop(&presentation->pending_input,
                                                   &scan_code)) == NXVM_CORE_STATUS_OK) {
        status = consumer(context, scan_code);
        if (status != NXVM_CORE_STATUS_OK) return status;
    }
    return status == NXVM_CORE_STATUS_UNSUPPORTED ? NXVM_CORE_STATUS_OK : status;
}

nxvm_core_status nxvm_product_nxvm_presentation_publish_text(
    nxvm_product_nxvm_presentation *presentation,
    const nxvm_core_text_snapshot *snapshot)
{
    if (presentation == NULL || snapshot == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (!presentation->command_boundary_open) return NXVM_CORE_STATUS_INVALID_STATE;
    return nxvm_core_text_snapshot_copy(snapshot, &presentation->published_text);
}

nxvm_core_status nxvm_product_nxvm_presentation_capture_text(
    const nxvm_product_nxvm_presentation *presentation,
    nxvm_core_text_snapshot *out_snapshot)
{
    if (presentation == NULL || out_snapshot == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    return nxvm_core_text_snapshot_copy(&presentation->published_text, out_snapshot);
}

void nxvm_product_nxvm_presentation_close_command_boundary(
    nxvm_product_nxvm_presentation *presentation)
{
    if (presentation != NULL) presentation->command_boundary_open = 0;
}
