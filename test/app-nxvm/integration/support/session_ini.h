#include "lib/types/types_interface.h"
#ifndef TEST_INTEGRATION_SUPPORT_SESSION_INI_H
#define TEST_INTEGRATION_SUPPORT_SESSION_INI_H

#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/machine/media/media.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/product/ini_interface.h"

typedef struct integration_ini_session integration_ini_session;

typedef lib_status (*integration_ini_session_overlay_transform)(
    integration_ini_session *session, void *opaque);

struct integration_ini_session {
    vm_session_request request;
    vm_machine *session;
    common_machine *common_machine;
    integration_ini_session_overlay_transform transform;
    void *transform_opaque;
};

/* The integration boundary accepts one ini request. VM opens declared
 * external media as discard-only sparse overlays; tests never alter assets. */
lib_i32 integration_ini_session_assets_present(
    const vm_session_request *request);
lib_status integration_ini_session_open(const char *directory,
    const char *file_name, integration_ini_session *out_session);
lib_status integration_ini_session_open_with_overlay_transform(const char *directory,
    const char *file_name, integration_ini_session_overlay_transform transform,
    void *opaque, integration_ini_session *out_session);
lib_status integration_ini_session_overlay_read(const integration_ini_session *session,
    core_machine_media_id id, void **out_bytes, lib_size *out_count);
lib_status integration_ini_session_overlay_write(integration_ini_session *session,
    core_machine_media_id id, const void *bytes, lib_size byte_count);
lib_status integration_ini_session_restart(integration_ini_session *session);
/* Integration uses the same composed Common executor as the product whenever
 * a test drives a running guest.  Direct Core loops remain for owner-local
 * controller tests that deliberately do not submit host input. */
lib_status integration_ini_session_start(integration_ini_session *session);
lib_i32 integration_ini_session_wait_for_state(const integration_ini_session *session,
    common_machine_state state, lib_u32 milliseconds);
lib_status integration_ini_session_pause(integration_ini_session *session,
    lib_u32 milliseconds);
lib_status integration_ini_session_resume(integration_ini_session *session,
    lib_u32 milliseconds);
lib_status integration_ini_session_reset(integration_ini_session *session,
    lib_u32 milliseconds);
void integration_ini_session_close(integration_ini_session *session);

#endif
