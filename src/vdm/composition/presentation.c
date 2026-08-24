#include "type.h"

#include "vdm/composition/presentation.h"



#define VDM_COMPOSITION_DOS_MINIMAL_INPUT_CAPACITY 32u

struct vdm_presentation {
    vdm_machine_dos_minimal *session;
    vdm_composition_input_event input[VDM_COMPOSITION_DOS_MINIMAL_INPUT_CAPACITY];
    C_UINT count;
};

type_status vdm_presentation_create(
    vdm_machine_dos_minimal *session,
    vdm_presentation **out_presentation)
{
    vdm_presentation *presentation;

    if (session == STD_NULL || out_presentation == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_presentation = STD_NULL;
    presentation = (vdm_presentation *)STD_CALLOC(1u,
                                                            sizeof(*presentation));
    if (presentation == STD_NULL) {
        return TYPE_STATUS_NO_MEMORY;
    }
    presentation->session = session;
    *out_presentation = presentation;
    return TYPE_STATUS_OK;
}

type_status vdm_presentation_enqueue_input(
    vdm_presentation *presentation,
    vdm_composition_input_event event)
{
    if (presentation == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (presentation->count == VDM_COMPOSITION_DOS_MINIMAL_INPUT_CAPACITY) {
        return TYPE_STATUS_FAULT;
    }
    presentation->input[presentation->count++] = event;
    return TYPE_STATUS_OK;
}

type_status vdm_presentation_apply_input(
    vdm_presentation *presentation)
{
    C_UINT index;
    type_status status;

    if (presentation == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < presentation->count; ++index) {
        status = vdm_machine_dos_minimal_submit_native_keyboard_byte(
            presentation->session, presentation->input[index].native_byte);
        if (status != TYPE_STATUS_OK) {
            return status;
        }
    }
    presentation->count = 0u;
    return TYPE_STATUS_OK;
}

type_status vdm_presentation_capture_text(
    vdm_presentation *presentation,
    type_unsigned_64 timestamp,
    vdm_presentation_snapshot *out_snapshot)
{
    vdm_machine_text_snapshot text;
    type_status status;

    if (presentation == STD_NULL || out_snapshot == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    out_snapshot->timestamp = timestamp;
    status = vdm_machine_dos_minimal_get_snapshot(presentation->session, &text);
    if (status != TYPE_STATUS_OK) return status;
    out_snapshot->text = text.text;
    return TYPE_STATUS_OK;
}

C_VOID vdm_presentation_destroy(
    vdm_presentation *presentation)
{
    STD_FREE(presentation);
}
