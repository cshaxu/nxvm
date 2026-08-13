#ifndef VDM_MACHINE_DOS_MINIMAL_H
#define VDM_MACHINE_DOS_MINIMAL_H


#include "core/machine/presentation_interface.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_machine_dos_minimal vdm_machine_dos_minimal;

typedef struct vdm_machine_text_snapshot {
    type_unsigned_64 pit_ticks;
    type_unsigned_8 keyboard_irq_pending;
    core_machine_text_snapshot text;
} vdm_machine_text_snapshot;

type_status vdm_machine_dos_minimal_create(
    vdm_machine_dos_minimal **out_session);
type_status vdm_machine_dos_minimal_reset(
    vdm_machine_dos_minimal *session);
type_status vdm_machine_dos_minimal_tick(
    vdm_machine_dos_minimal *session,
    type_unsigned_32 ticks);
type_status vdm_machine_dos_minimal_inject_key(
    vdm_machine_dos_minimal *session,
    type_unsigned_8 scan_code);
type_status vdm_machine_dos_minimal_write_text(
    vdm_machine_dos_minimal *session,
    type_unsigned_16 cell,
    type_unsigned_8 character,
    type_unsigned_8 attribute);
type_status vdm_machine_dos_minimal_get_snapshot(
    const vdm_machine_dos_minimal *session,
    vdm_machine_text_snapshot *out_snapshot);
type_status vdm_machine_dos_minimal_port_read(
    vdm_machine_dos_minimal *session,
    type_unsigned_16 port,
    type_unsigned_32 *out_value);
C_VOID vdm_machine_dos_minimal_destroy(vdm_machine_dos_minimal *session);

#ifdef __cplusplus
}
#endif

#endif

