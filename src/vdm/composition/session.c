#include "type.h"

#include "vdm/composition/session.h"




#include "vdm/machine/dos_minimal.h"

struct vdm_session {
    vdm_machine_dos_minimal *dos_minimal;
};

type_status vdm_session_create(
    vdm_session **out_session)
{
    vdm_session *session;
    type_status status;

    if (out_session == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (vdm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) {
        return TYPE_STATUS_NO_MEMORY;
    }
    status = vdm_machine_dos_minimal_create(&session->dos_minimal);
    if (status != TYPE_STATUS_OK) {
        STD_FREE(session);
        return status;
    }
    *out_session = session;
    return TYPE_STATUS_OK;
}

type_status vdm_session_reset(
    vdm_session *session)
{
    if (session == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return vdm_machine_dos_minimal_reset(session->dos_minimal);
}

C_VOID vdm_session_destroy(vdm_session *session)
{
    if (session != STD_NULL) {
        vdm_machine_dos_minimal_destroy(session->dos_minimal);
        STD_FREE(session);
    }
}

type_status vdm_session_inject_key(vdm_session *session, uint8_t scan_code)
{ return session == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
    vdm_machine_dos_minimal_inject_key(session->dos_minimal, scan_code); }
type_status vdm_session_write_text(vdm_session *session, uint16_t cell,
    uint8_t character, uint8_t attribute)
{ return session == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
    vdm_machine_dos_minimal_write_text(session->dos_minimal, cell, character, attribute); }
type_status vdm_session_get_snapshot(const vdm_session *session,
    vdm_machine_text_snapshot *out_snapshot)
{ return session == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
    vdm_machine_dos_minimal_get_snapshot(session->dos_minimal, out_snapshot); }
type_status vdm_session_port_read(vdm_session *session, uint16_t port,
    uint32_t *out_value)
{ return session == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
    vdm_machine_dos_minimal_port_read(session->dos_minimal, port, out_value); }

