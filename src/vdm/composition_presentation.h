#ifndef NXVM_ADAPTERS_DOS_MINIMAL_PRESENTATION_H
#define NXVM_ADAPTERS_DOS_MINIMAL_PRESENTATION_H

#include "core/platform/presentation.h"
#include "vdm/machine/dos_minimal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_dos_minimal_presentation nxvm_dos_minimal_presentation;

typedef struct nxvm_vdm_presentation_snapshot {
    uint64_t timestamp;
    nxvm_core_text_snapshot text;
} nxvm_vdm_presentation_snapshot;

nxvm_core_status nxvm_dos_minimal_presentation_create(
    nxvm_runtime_dos_minimal *session,
    nxvm_dos_minimal_presentation **out_presentation);
nxvm_core_status nxvm_dos_minimal_presentation_enqueue_input(
    nxvm_dos_minimal_presentation *presentation,
    nxvm_platform_input_event event);
nxvm_core_status nxvm_dos_minimal_presentation_apply_input(
    nxvm_dos_minimal_presentation *presentation);
nxvm_core_status nxvm_dos_minimal_presentation_capture_text(
    nxvm_dos_minimal_presentation *presentation,
    uint64_t timestamp,
    nxvm_vdm_presentation_snapshot *out_snapshot);
void nxvm_dos_minimal_presentation_destroy(
    nxvm_dos_minimal_presentation *presentation);

#ifdef __cplusplus
}
#endif

#endif
