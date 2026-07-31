#ifndef NXVM_PRODUCT_NXVM_PRESENTATION_H
#define NXVM_PRODUCT_NXVM_PRESENTATION_H

#include "core/presentation.h"

typedef struct nxvm_product_nxvm_presentation {
    nxvm_core_keyboard_queue pending_input;
    nxvm_core_text_snapshot published_text;
    int command_boundary_open;
} nxvm_product_nxvm_presentation;

typedef nxvm_core_status (*nxvm_product_nxvm_input_consumer)(
    void *context, uint16_t scan_code);

void nxvm_product_nxvm_presentation_initialize(
    nxvm_product_nxvm_presentation *presentation);
nxvm_core_status nxvm_product_nxvm_presentation_enqueue_input(
    nxvm_product_nxvm_presentation *presentation, uint16_t scan_code);
nxvm_core_status nxvm_product_nxvm_presentation_open_command_boundary(
    nxvm_product_nxvm_presentation *presentation);
nxvm_core_status nxvm_product_nxvm_presentation_apply_input(
    nxvm_product_nxvm_presentation *presentation,
    nxvm_product_nxvm_input_consumer consumer, void *context);
nxvm_core_status nxvm_product_nxvm_presentation_publish_text(
    nxvm_product_nxvm_presentation *presentation,
    const nxvm_core_text_snapshot *snapshot);
nxvm_core_status nxvm_product_nxvm_presentation_capture_text(
    const nxvm_product_nxvm_presentation *presentation,
    nxvm_core_text_snapshot *out_snapshot);
void nxvm_product_nxvm_presentation_close_command_boundary(
    nxvm_product_nxvm_presentation *presentation);

#endif
