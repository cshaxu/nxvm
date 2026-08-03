#include "type.h"

#include "vdm/composition/presentation.h"



#define NXVM_DOS_MINIMAL_INPUT_CAPACITY 32u

struct vdm_presentation {
    vdm_session *session;
    vdm_composition_input_event input[NXVM_DOS_MINIMAL_INPUT_CAPACITY];
    C_UINT count;
};

ntvdm64_status vdm_presentation_create(
    vdm_session *session,
    vdm_presentation **out_presentation)
{
    vdm_presentation *presentation;

    if (session == STD_NULL || out_presentation == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    *out_presentation = STD_NULL;
    presentation = (vdm_presentation *)STD_CALLOC(1u,
                                                            sizeof(*presentation));
    if (presentation == STD_NULL) {
        return NTVDM64_STATUS_NO_MEMORY;
    }
    presentation->session = session;
    *out_presentation = presentation;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vdm_presentation_enqueue_input(
    vdm_presentation *presentation,
    vdm_composition_input_event event)
{
    if (presentation == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (presentation->count == NXVM_DOS_MINIMAL_INPUT_CAPACITY) {
        return NTVDM64_STATUS_FAULT;
    }
    presentation->input[presentation->count++] = event;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vdm_presentation_apply_input(
    vdm_presentation *presentation)
{
    C_UINT index;
    ntvdm64_status status;

    if (presentation == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < presentation->count; ++index) {
        status = vdm_session_inject_key(
            presentation->session, presentation->input[index].scan_code);
        if (status != NTVDM64_STATUS_OK) {
            return status;
        }
    }
    presentation->count = 0u;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vdm_presentation_capture_text(
    vdm_presentation *presentation,
    uint64_t timestamp,
    vdm_presentation_snapshot *out_snapshot)
{
    vdm_machine_text_snapshot text;
    ntvdm64_status status;

    if (presentation == STD_NULL || out_snapshot == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    out_snapshot->timestamp = timestamp;
    status = vdm_session_get_snapshot(presentation->session, &text);
    if (status != NTVDM64_STATUS_OK) return status;
    out_snapshot->text = text.text;
    return NTVDM64_STATUS_OK;
}

C_VOID vdm_presentation_destroy(
    vdm_presentation *presentation)
{
    STD_FREE(presentation);
}

