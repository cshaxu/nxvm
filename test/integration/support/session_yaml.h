#ifndef TEST_INTEGRATION_SUPPORT_SESSION_YAML_H
#define TEST_INTEGRATION_SUPPORT_SESSION_YAML_H

#include "core/product/session/session_provider.h"
#include "vm/composition/session/session_interface.h"
#include "vm/product/session_catalog.h"

typedef struct integration_yaml_session {
    core_product_session_provider provider;
    vm_product_session_request request;
    vm_session *session;
    STD_SIZE_T copied_floppy_count;
    STD_SIZE_T copied_fixed_disk_count;
} integration_yaml_session;

typedef type_status (*integration_yaml_session_media_transform)(
    vm_product_session_request *request, C_VOID *opaque);

/* The integration boundary accepts only a catalog request.  It copies every
 * mutable media input into the test's own CTest working directory before the
 * product provider sees it. */
C_INT integration_yaml_session_assets_present(
    const vm_product_session_request *request);
type_status integration_yaml_session_open(const C_CHAR *directory,
    const C_CHAR *file_name, integration_yaml_session *out_session);
type_status integration_yaml_session_open_with_media_transform(const C_CHAR *directory,
    const C_CHAR *file_name, integration_yaml_session_media_transform transform,
    C_VOID *opaque, integration_yaml_session *out_session);
type_status integration_yaml_session_restart(integration_yaml_session *session);
C_VOID integration_yaml_session_close(integration_yaml_session *session);

#endif
