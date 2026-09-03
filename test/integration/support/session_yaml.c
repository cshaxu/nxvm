#include "type.h"

#include <windows.h>

#include "test/integration/support/session_yaml.h"
#include "vm/composition/session/provider.h"
#include "vm/composition/session/session_private.h"

static C_INT integration_yaml_session_find(const C_CHAR *directory,
    const C_CHAR *file_name, vm_product_session_request *out_request)
{
    vm_product_session_catalog *catalog = STD_NULL;
    STD_SIZE_T index;
    C_INT found = 0;

    if (directory == STD_NULL || file_name == STD_NULL || out_request == STD_NULL ||
        vm_product_session_catalog_create(directory, &catalog) != TYPE_STATUS_OK) return 0;
    for (index = 0u; index < vm_product_session_catalog_count(catalog); ++index) {
        vm_product_session_request request;

        if (vm_product_session_catalog_get_request(catalog, index, &request) ==
                TYPE_STATUS_OK && !STD_STRCMP(request.file_name, file_name)) {
            *out_request = request;
            found = 1;
            break;
        }
    }
    vm_product_session_catalog_destroy(catalog);
    return found;
}

C_INT integration_yaml_session_assets_present(
    const vm_product_session_request *request)
{
    STD_SIZE_T index;

    if (request == STD_NULL) return 0;
    for (index = 0u; index < request->bios_count; ++index)
        if (GetFileAttributesA(request->bios[index]) == INVALID_FILE_ATTRIBUTES) return 0;
    for (index = 0u; index < request->floppy_count; ++index)
        if (GetFileAttributesA(request->floppy[index]) == INVALID_FILE_ATTRIBUTES) return 0;
    for (index = 0u; index < request->fixed_disk_count; ++index)
        if (GetFileAttributesA(request->fixed_disk[index]) == INVALID_FILE_ATTRIBUTES) return 0;
    return (request->video[0] == '\0' || GetFileAttributesA(request->video) !=
            INVALID_FILE_ATTRIBUTES) &&
        (request->cmos[0] == '\0' || GetFileAttributesA(request->cmos) !=
            INVALID_FILE_ATTRIBUTES) &&
        (request->font[0] == '\0' || GetFileAttributesA(request->font) !=
            INVALID_FILE_ATTRIBUTES);
}

type_status integration_yaml_session_restart(integration_yaml_session *session)
{
    type_status status;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (session->session != STD_NULL) {
        (C_VOID)session->provider.close(session->provider.context, session->session);
        session->session = STD_NULL;
    }
    status = session->provider.open(session->provider.context, 1u,
        &(core_product_session_open_options) {0u, STD_NULL, &session->request,
            sizeof(session->request)}, (C_VOID **)&session->session);
    if (status != TYPE_STATUS_OK || session->session == STD_NULL) return TYPE_STATUS_FAULT;
    if (session->transform != STD_NULL && session->transform(session,
            session->transform_opaque) != TYPE_STATUS_OK) {
        (C_VOID)session->provider.close(session->provider.context, session->session);
        session->session = STD_NULL;
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

type_status integration_yaml_session_open(const C_CHAR *directory,
    const C_CHAR *file_name, integration_yaml_session *out_session)
{
    return integration_yaml_session_open_with_overlay_transform(directory, file_name,
        STD_NULL, STD_NULL, out_session);
}

type_status integration_yaml_session_open_with_overlay_transform(const C_CHAR *directory,
    const C_CHAR *file_name, integration_yaml_session_overlay_transform transform,
    C_VOID *opaque, integration_yaml_session *out_session)
{
    type_status status;

    if (out_session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(out_session, 0, sizeof(*out_session));
    if (!integration_yaml_session_find(directory, file_name, &out_session->request)) {
        STD_FPRINTF(STD_STDERR, "T515:YAML-SESSION:%s:CATALOG-REQUEST-NOT-FOUND\n",
            file_name == STD_NULL ? "(null)" : file_name);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!integration_yaml_session_assets_present(&out_session->request)) {
        STD_FPRINTF(STD_STDERR, "T515:YAML-SESSION:%s:DECLARED-ASSET-UNAVAILABLE\n",
            file_name);
        return TYPE_STATUS_UNSUPPORTED;
    }
    vm_session_provider_initialize(&out_session->provider);
    out_session->transform = transform;
    out_session->transform_opaque = opaque;
    status = integration_yaml_session_restart(out_session);
    if (status != TYPE_STATUS_OK || out_session->session == STD_NULL) {
        STD_FPRINTF(STD_STDERR, "T515:YAML-SESSION:%s:OPEN-FAILED:%d\n",
            file_name, (C_INT)status);
        integration_yaml_session_close(out_session);
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

type_status integration_yaml_session_overlay_read(const integration_yaml_session *session,
    core_machine_media_id id, C_VOID **out_bytes, STD_SIZE_T *out_count)
{
    core_machine_media_info info;
    core_machine_media_result result;
    STD_SIZE_T count;
    C_VOID *bytes;

    if (session == STD_NULL || session->session == STD_NULL || out_bytes == STD_NULL ||
        out_count == STD_NULL || core_machine_media_query(session->session->media_registry,
            id, &info, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        info.geometry.bytes_per_sector == 0u ||
        info.geometry.logical_sector_count > (STD_SIZE_T)-1 / info.geometry.bytes_per_sector) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    count = (STD_SIZE_T)info.geometry.logical_sector_count * info.geometry.bytes_per_sector;
    if (count == 0u || count > TYPE_MAX_UNSIGNED_32) return TYPE_STATUS_INVALID_ARGUMENT;
    bytes = STD_MALLOC(count);
    if (bytes == STD_NULL || core_machine_media_read_bytes(session->session->media_registry,
            id, 0u, bytes, (type_unsigned_32)count, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK) {
        STD_FREE(bytes);
        return TYPE_STATUS_FAULT;
    }
    *out_bytes = bytes;
    *out_count = count;
    return TYPE_STATUS_OK;
}

type_status integration_yaml_session_overlay_write(integration_yaml_session *session,
    core_machine_media_id id, const C_VOID *bytes, STD_SIZE_T byte_count)
{
    core_machine_media_result result;

    if (session == STD_NULL || session->session == STD_NULL || bytes == STD_NULL ||
        byte_count > TYPE_MAX_UNSIGNED_32 || core_machine_media_write_bytes(
            session->session->media_registry, id, 0u, bytes,
            (type_unsigned_32)byte_count, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK) return TYPE_STATUS_INVALID_ARGUMENT;
    return TYPE_STATUS_OK;
}

C_VOID integration_yaml_session_close(integration_yaml_session *session)
{
    if (session == STD_NULL) return;
    if (session->session != STD_NULL) (C_VOID)session->provider.close(
        session->provider.context, session->session);
    STD_MEMSET(session, 0, sizeof(*session));
}
