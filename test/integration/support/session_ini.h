#ifndef TEST_INTEGRATION_SUPPORT_SESSION_INI_H
#define TEST_INTEGRATION_SUPPORT_SESSION_INI_H

#include "core/devices/media_interface.h"
#include "core/machine/media/media.h"
#include "core/machine/machine_interface.h"
#include "app/ini_interface.h"

typedef struct integration_ini_session integration_ini_session;

typedef type_status (*integration_ini_session_overlay_transform)(
    integration_ini_session *session, C_VOID *opaque);

struct integration_ini_session {
    vm_session_request request;
    vm_machine *session;
    integration_ini_session_overlay_transform transform;
    C_VOID *transform_opaque;
};

/* The integration boundary accepts one ini request. VM opens declared
 * external media as discard-only sparse overlays; tests never alter assets. */
C_INT integration_ini_session_assets_present(
    const vm_session_request *request);
type_status integration_ini_session_open(const C_CHAR *directory,
    const C_CHAR *file_name, integration_ini_session *out_session);
type_status integration_ini_session_open_with_overlay_transform(const C_CHAR *directory,
    const C_CHAR *file_name, integration_ini_session_overlay_transform transform,
    C_VOID *opaque, integration_ini_session *out_session);
type_status integration_ini_session_overlay_read(const integration_ini_session *session,
    core_machine_media_id id, C_VOID **out_bytes, STD_SIZE_T *out_count);
type_status integration_ini_session_overlay_write(integration_ini_session *session,
    core_machine_media_id id, const C_VOID *bytes, STD_SIZE_T byte_count);
type_status integration_ini_session_restart(integration_ini_session *session);
C_VOID integration_ini_session_close(integration_ini_session *session);

#endif
