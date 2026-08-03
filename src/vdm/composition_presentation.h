#ifndef NXVM_ADAPTERS_DOS_MINIMAL_PRESENTATION_H
#define NXVM_ADAPTERS_DOS_MINIMAL_PRESENTATION_H

#include "core/platform/presentation.h"
#include "vdm/machine/dos_minimal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_composition_presentation vdm_composition_presentation;

typedef struct nxvm_vdm_presentation_snapshot {
    uint64_t timestamp;
    core_machine_text_snapshot text;
} nxvm_vdm_presentation_snapshot;

ntvdm64_status vdm_composition_presentation_create(
    core_product_runtime_dos_minimal *session,
    vdm_composition_presentation **out_presentation);
ntvdm64_status vdm_composition_presentation_enqueue_input(
    vdm_composition_presentation *presentation,
    nxvm_platform_input_event event);
ntvdm64_status vdm_composition_presentation_apply_input(
    vdm_composition_presentation *presentation);
ntvdm64_status vdm_composition_presentation_capture_text(
    vdm_composition_presentation *presentation,
    uint64_t timestamp,
    nxvm_vdm_presentation_snapshot *out_snapshot);
void vdm_composition_presentation_destroy(
    vdm_composition_presentation *presentation);

#ifdef __cplusplus
}
#endif

#endif
