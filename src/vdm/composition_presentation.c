#include "vdm/composition_presentation.h"

#include <stdlib.h>

#define NXVM_DOS_MINIMAL_INPUT_CAPACITY 32u

struct nxvm_dos_minimal_presentation {
    core_product_runtime_dos_minimal *session;
    nxvm_platform_input_event input[NXVM_DOS_MINIMAL_INPUT_CAPACITY];
    unsigned count;
};

ntvdm64_status nxvm_dos_minimal_presentation_create(
    core_product_runtime_dos_minimal *session,
    nxvm_dos_minimal_presentation **out_presentation)
{
    nxvm_dos_minimal_presentation *presentation;

    if (session == NULL || out_presentation == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    *out_presentation = NULL;
    presentation = (nxvm_dos_minimal_presentation *)calloc(1u,
                                                            sizeof(*presentation));
    if (presentation == NULL) {
        return NTVDM64_STATUS_NO_MEMORY;
    }
    presentation->session = session;
    *out_presentation = presentation;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status nxvm_dos_minimal_presentation_enqueue_input(
    nxvm_dos_minimal_presentation *presentation,
    nxvm_platform_input_event event)
{
    if (presentation == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (presentation->count == NXVM_DOS_MINIMAL_INPUT_CAPACITY) {
        return NTVDM64_STATUS_FAULT;
    }
    presentation->input[presentation->count++] = event;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status nxvm_dos_minimal_presentation_apply_input(
    nxvm_dos_minimal_presentation *presentation)
{
    unsigned index;
    ntvdm64_status status;

    if (presentation == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < presentation->count; ++index) {
        status = core_product_runtime_dos_minimal_inject_key(
            presentation->session, presentation->input[index].scan_code);
        if (status != NTVDM64_STATUS_OK) {
            return status;
        }
    }
    presentation->count = 0u;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status nxvm_dos_minimal_presentation_capture_text(
    nxvm_dos_minimal_presentation *presentation,
    uint64_t timestamp,
    nxvm_vdm_presentation_snapshot *out_snapshot)
{
    core_product_runtime_text_snapshot text;
    ntvdm64_status status;

    if (presentation == NULL || out_snapshot == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    out_snapshot->timestamp = timestamp;
    status = core_product_runtime_dos_minimal_get_snapshot(presentation->session, &text);
    if (status != NTVDM64_STATUS_OK) return status;
    out_snapshot->text = text.text;
    return NTVDM64_STATUS_OK;
}

void nxvm_dos_minimal_presentation_destroy(
    nxvm_dos_minimal_presentation *presentation)
{
    free(presentation);
}
