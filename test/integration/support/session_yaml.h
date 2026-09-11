#ifndef TEST_INTEGRATION_SUPPORT_SESSION_YAML_H
#define TEST_INTEGRATION_SUPPORT_SESSION_YAML_H

#include "core/machine/media_interface.h"
#include "vm/machine/runtime/media.h"
#include "vm/machine/runtime/machine_interface.h"
#include "vm/product/catalog.h"

typedef struct integration_yaml_session integration_yaml_session;

typedef type_status (*integration_yaml_session_overlay_transform)(
    integration_yaml_session *session, C_VOID *opaque);

struct integration_yaml_session {
    vm_session_request request;
    vm_machine *session;
    integration_yaml_session_overlay_transform transform;
    C_VOID *transform_opaque;
};

/* The integration boundary accepts one catalog request. VM opens declared
 * external media as discard-only sparse overlays; tests never alter assets. */
C_INT integration_yaml_session_assets_present(
    const vm_session_request *request);
type_status integration_yaml_session_open(const C_CHAR *directory,
    const C_CHAR *file_name, integration_yaml_session *out_session);
type_status integration_yaml_session_open_with_overlay_transform(const C_CHAR *directory,
    const C_CHAR *file_name, integration_yaml_session_overlay_transform transform,
    C_VOID *opaque, integration_yaml_session *out_session);
type_status integration_yaml_session_overlay_read(const integration_yaml_session *session,
    core_machine_media_id id, C_VOID **out_bytes, STD_SIZE_T *out_count);
type_status integration_yaml_session_overlay_write(integration_yaml_session *session,
    core_machine_media_id id, const C_VOID *bytes, STD_SIZE_T byte_count);
type_status integration_yaml_session_restart(integration_yaml_session *session);
C_VOID integration_yaml_session_close(integration_yaml_session *session);

#endif
