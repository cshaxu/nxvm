#include "type.h"

#include <windows.h>

#include "test/integration/support/session_yaml.h"
#include "vm/composition/session/provider.h"

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

static C_INT integration_yaml_session_copy_media(C_CHAR *path,
    STD_SIZE_T capacity, const C_CHAR *name)
{
    C_CHAR sidecar[80];

    if (path == STD_NULL || name == STD_NULL || path[0] == '\0' ||
        STD_STRLEN(name) + 1u > capacity ||
        STD_SNPRINTF(sidecar, sizeof(sidecar), "%s.nxvm-fdd.json", name) < 0) return 0;
    /* The FDD adapter owns this derived sidecar.  A test-owned media copy
     * must not inherit its geometry/address marks from an earlier case. */
    (C_VOID)DeleteFileA(sidecar);
    if (!CopyFileA(path, name, FALSE)) return 0;
    STD_MEMCPY(path, name, STD_STRLEN(name) + 1u);
    return 1;
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
    return status == TYPE_STATUS_OK && session->session != STD_NULL ?
        TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}

type_status integration_yaml_session_open(const C_CHAR *directory,
    const C_CHAR *file_name, integration_yaml_session *out_session)
{
    return integration_yaml_session_open_with_media_transform(directory, file_name,
        STD_NULL, STD_NULL, out_session);
}

type_status integration_yaml_session_open_with_media_transform(const C_CHAR *directory,
    const C_CHAR *file_name, integration_yaml_session_media_transform transform,
    C_VOID *opaque, integration_yaml_session *out_session)
{
    STD_SIZE_T index;
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
    for (index = 0u; index < out_session->request.floppy_count; ++index) {
        C_CHAR name[64];

        if (STD_SNPRINTF(name, sizeof(name), "session-floppy-%u.img",
                (unsigned int)index) < 0 || !integration_yaml_session_copy_media(
                out_session->request.floppy[index],
                sizeof(out_session->request.floppy[index]), name)) return TYPE_STATUS_FAULT;
        ++out_session->copied_floppy_count;
    }
    for (index = 0u; index < out_session->request.fixed_disk_count; ++index) {
        C_CHAR name[64];

        if (STD_SNPRINTF(name, sizeof(name), "session-fixed-disk-%u.img",
                (unsigned int)index) < 0 || !integration_yaml_session_copy_media(
                out_session->request.fixed_disk[index],
                sizeof(out_session->request.fixed_disk[index]), name)) return TYPE_STATUS_FAULT;
        ++out_session->copied_fixed_disk_count;
    }
    if (transform != STD_NULL && transform(&out_session->request, opaque) != TYPE_STATUS_OK) {
        integration_yaml_session_close(out_session);
        return TYPE_STATUS_FAULT;
    }
    vm_session_provider_initialize(&out_session->provider);
    status = integration_yaml_session_restart(out_session);
    if (status != TYPE_STATUS_OK || out_session->session == STD_NULL) {
        STD_FPRINTF(STD_STDERR, "T515:YAML-SESSION:%s:OPEN-FAILED:%d\n",
            file_name, (C_INT)status);
        integration_yaml_session_close(out_session);
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

C_VOID integration_yaml_session_close(integration_yaml_session *session)
{
    STD_SIZE_T index;

    if (session == STD_NULL) return;
    if (session->session != STD_NULL) (C_VOID)session->provider.close(
        session->provider.context, session->session);
    if (session->copied_floppy_count != 0u || session->copied_fixed_disk_count != 0u) {
        for (index = 0u; index < session->copied_floppy_count; ++index) {
            C_CHAR sidecar[80];

            if (STD_SNPRINTF(sidecar, sizeof(sidecar), "%s.nxvm-fdd.json",
                    session->request.floppy[index]) >= 0) (C_VOID)DeleteFileA(sidecar);
            (C_VOID)DeleteFileA(session->request.floppy[index]);
        }
        for (index = 0u; index < session->copied_fixed_disk_count; ++index)
            (C_VOID)DeleteFileA(session->request.fixed_disk[index]);
    }
    STD_MEMSET(session, 0, sizeof(*session));
}
