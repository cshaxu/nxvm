#ifndef VDM_MINIMAL_SESSION_H
#define VDM_MINIMAL_SESSION_H

#include "type.h"
#include "vdm/machine/dos_minimal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_session vdm_session;

type_status vdm_session_create(
    vdm_session **out_session);
type_status vdm_session_reset(
    vdm_session *session);
C_VOID vdm_session_destroy(vdm_session *session);
type_status vdm_session_submit_native_keyboard_byte(vdm_session *session,
    type_unsigned_8 native_byte);
type_status vdm_session_write_text(vdm_session *session, type_unsigned_16 cell,
    type_unsigned_8 character, type_unsigned_8 attribute);
type_status vdm_session_get_snapshot(const vdm_session *session,
    vdm_machine_text_snapshot *out_snapshot);
type_status vdm_session_port_read(vdm_session *session, type_unsigned_16 port,
    type_unsigned_32 *out_value);

#ifdef __cplusplus
}
#endif

#endif
