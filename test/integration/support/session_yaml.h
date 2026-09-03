#ifndef TEST_INTEGRATION_SUPPORT_SESSION_YAML_H
#define TEST_INTEGRATION_SUPPORT_SESSION_YAML_H

#include "core/product/session/session_provider.h"
#include "core/machine/media_interface.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session_interface.h"
#include "vm/product/session_catalog.h"

typedef struct integration_yaml_session integration_yaml_session;

typedef type_status (*integration_yaml_session_overlay_transform)(
    integration_yaml_session *session, C_VOID *opaque);

struct integration_yaml_session {
    core_product_session_provider provider;
    vm_product_session_request request;
    vm_session *session;
    integration_yaml_session_overlay_transform transform;
    C_VOID *transform_opaque;
};

/* The integration boundary accepts one catalog request. VM loads the declared
 * external media into its sole in-memory overlay; tests may alter only that
 * owner-backed overlay, never an asset file or a replacement request path. */
C_INT integration_yaml_session_assets_present(
    const vm_product_session_request *request);
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
