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
type_status vdm_session_inject_key(vdm_session *session, uint8_t scan_code);
type_status vdm_session_write_text(vdm_session *session, uint16_t cell,
    uint8_t character, uint8_t attribute);
type_status vdm_session_get_snapshot(const vdm_session *session,
    vdm_machine_text_snapshot *out_snapshot);
type_status vdm_session_port_read(vdm_session *session, uint16_t port,
    uint32_t *out_value);

#ifdef __cplusplus
}
#endif

#endif
