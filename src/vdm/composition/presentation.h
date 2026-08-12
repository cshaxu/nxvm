#ifndef VDM_COMPOSITION_DOS_MINIMAL_PRESENTATION_H
#define VDM_COMPOSITION_DOS_MINIMAL_PRESENTATION_H

#include "type.h"

#include "vdm/composition/input_event.h"

#include "vdm/composition/session.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_presentation vdm_presentation;

typedef struct vdm_presentation_snapshot {
    type_unsigned_64 timestamp;
    core_machine_text_snapshot text;
} vdm_presentation_snapshot;

type_status vdm_presentation_create(
    vdm_session *session,
    vdm_presentation **out_presentation);
type_status vdm_presentation_enqueue_input(
    vdm_presentation *presentation,
    vdm_composition_input_event event);
type_status vdm_presentation_apply_input(
    vdm_presentation *presentation);
type_status vdm_presentation_capture_text(
    vdm_presentation *presentation,
    type_unsigned_64 timestamp,
    vdm_presentation_snapshot *out_snapshot);
C_VOID vdm_presentation_destroy(
    vdm_presentation *presentation);

#ifdef __cplusplus
}
#endif

#endif

