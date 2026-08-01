#include "vdm/platform/dos_minimal_presentation.h"

#include <stdlib.h>

#define NXVM_DOS_MINIMAL_INPUT_CAPACITY 32u

struct nxvm_dos_minimal_presentation {
    nxvm_runtime_dos_minimal *session;
    nxvm_platform_input_event input[NXVM_DOS_MINIMAL_INPUT_CAPACITY];
    unsigned count;
};

nxvm_core_status nxvm_dos_minimal_presentation_create(
    nxvm_runtime_dos_minimal *session,
    nxvm_dos_minimal_presentation **out_presentation)
{
    nxvm_dos_minimal_presentation *presentation;

    if (session == NULL || out_presentation == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_presentation = NULL;
    presentation = (nxvm_dos_minimal_presentation *)calloc(1u,
                                                            sizeof(*presentation));
    if (presentation == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }
    presentation->session = session;
    *out_presentation = presentation;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_dos_minimal_presentation_enqueue_input(
    nxvm_dos_minimal_presentation *presentation,
    nxvm_platform_input_event event)
{
    if (presentation == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    if (presentation->count == NXVM_DOS_MINIMAL_INPUT_CAPACITY) {
        return NXVM_CORE_STATUS_FAULT;
    }
    presentation->input[presentation->count++] = event;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_dos_minimal_presentation_apply_input(
    nxvm_dos_minimal_presentation *presentation)
{
    unsigned index;
    nxvm_core_status status;

    if (presentation == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < presentation->count; ++index) {
        status = nxvm_runtime_dos_minimal_inject_key(
            presentation->session, presentation->input[index].scan_code);
        if (status != NXVM_CORE_STATUS_OK) {
            return status;
        }
    }
    presentation->count = 0u;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_dos_minimal_presentation_capture_text(
    nxvm_dos_minimal_presentation *presentation,
    uint64_t timestamp,
    nxvm_platform_text_snapshot *out_snapshot)
{
    if (presentation == NULL || out_snapshot == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    out_snapshot->timestamp = timestamp;
    return nxvm_runtime_dos_minimal_get_snapshot(presentation->session,
                                                  &out_snapshot->text);
}

void nxvm_dos_minimal_presentation_destroy(
    nxvm_dos_minimal_presentation *presentation)
{
    free(presentation);
}
