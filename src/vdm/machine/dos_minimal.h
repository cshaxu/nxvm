#ifndef NXVM_RUNTIME_DOS_MINIMAL_H
#define NXVM_RUNTIME_DOS_MINIMAL_H


#include "core/machine/presentation_interface.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_machine_dos_minimal vdm_machine_dos_minimal;

typedef struct vdm_machine_text_snapshot {
    uint64_t pit_ticks;
    uint8_t keyboard_irq_pending;
    core_machine_text_snapshot text;
} vdm_machine_text_snapshot;

ntvdm64_status vdm_machine_dos_minimal_create(
    vdm_machine_dos_minimal **out_session);
ntvdm64_status vdm_machine_dos_minimal_reset(
    vdm_machine_dos_minimal *session);
ntvdm64_status vdm_machine_dos_minimal_tick(
    vdm_machine_dos_minimal *session,
    uint32_t ticks);
ntvdm64_status vdm_machine_dos_minimal_inject_key(
    vdm_machine_dos_minimal *session,
    uint8_t scan_code);
ntvdm64_status vdm_machine_dos_minimal_write_text(
    vdm_machine_dos_minimal *session,
    uint16_t cell,
    uint8_t character,
    uint8_t attribute);
ntvdm64_status vdm_machine_dos_minimal_get_snapshot(
    const vdm_machine_dos_minimal *session,
    vdm_machine_text_snapshot *out_snapshot);
ntvdm64_status vdm_machine_dos_minimal_port_read(
    vdm_machine_dos_minimal *session,
    uint16_t port,
    uint32_t *out_value);
C_VOID vdm_machine_dos_minimal_destroy(vdm_machine_dos_minimal *session);

#ifdef __cplusplus
}
#endif

#endif

