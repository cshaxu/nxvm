#include "type.h"

#include <windows.h>

#include "test/integration/support/session_ini.h"
#include "app/config.h"
#include "core/machine/machine_private.h"

static C_INT integration_ini_session_find(const C_CHAR *directory,
    const C_CHAR *file_name, vm_session_request *out_request)
{
    C_CHAR path[VM_SESSION_REQUEST_PATH_MAX];
    STD_SIZE_T directory_bytes;
    STD_SIZE_T file_bytes;

    if (directory == STD_NULL || file_name == STD_NULL || out_request == STD_NULL) return 0;
    directory_bytes = STD_STRLEN(directory);
    file_bytes = STD_STRLEN(file_name);
    if (directory_bytes + 1u + file_bytes >= sizeof(path)) return 0;
    STD_MEMCPY(path, directory, directory_bytes);
    if (directory_bytes != 0u && directory[directory_bytes - 1u] != '/' &&
        directory[directory_bytes - 1u] != '\\') path[directory_bytes++] = '/';
    STD_MEMCPY(path + directory_bytes, file_name, file_bytes + 1u);
    return vm_app_ini_load(path, out_request) == TYPE_STATUS_OK;
}

C_INT integration_ini_session_assets_present(
    const vm_session_request *request)
{
    STD_SIZE_T index;

    if (request == STD_NULL) return 0;
    for (index = 0u; index < request->floppy_count; ++index)
        if (GetFileAttributesA(request->floppy[index]) == INVALID_FILE_ATTRIBUTES) return 0;
    for (index = 0u; index < request->fixed_disk_count; ++index)
        if (GetFileAttributesA(request->fixed_disk[index]) == INVALID_FILE_ATTRIBUTES) return 0;
    return 1;
}

type_status integration_ini_session_restart(integration_ini_session *session)
{
    vm_machine_config configuration;
    type_status status;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (session->common_machine != LIB_NULL) {
        (C_VOID)common_machine_shutdown(session->common_machine);
        (C_VOID)vm_machine_bind_common_machine(session->session, LIB_NULL);
        (C_VOID)common_machine_destroy(session->common_machine);
        session->common_machine = LIB_NULL;
    }
    if (session->session != STD_NULL) {
        vm_machine_destroy(session->session);
        session->session = STD_NULL;
    }
    status = vm_app_configure_machine(&session->request, &configuration);
    if (status == TYPE_STATUS_OK)
        status = vm_machine_create(&configuration, &session->session);
    if (status != TYPE_STATUS_OK || session->session == STD_NULL) return TYPE_STATUS_FAULT;
    if (session->transform != STD_NULL && session->transform(session,
            session->transform_opaque) != TYPE_STATUS_OK) {
        vm_machine_destroy(session->session);
        session->session = STD_NULL;
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

type_status integration_ini_session_start(integration_ini_session *session)
{
    common_machine_driver driver;

    if (session == STD_NULL || session->session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (session->common_machine == LIB_NULL &&
        (vm_machine_describe_common_driver(session->session, &driver) != TYPE_STATUS_OK ||
        common_machine_create(&session->common_machine, &driver) != LIB_STATUS_OK ||
        vm_machine_bind_common_machine(session->session, session->common_machine) !=
            TYPE_STATUS_OK)) return TYPE_STATUS_FAULT;
    return common_machine_start(session->common_machine) ? TYPE_STATUS_OK :
        TYPE_STATUS_INVALID_STATE;
}

type_status integration_ini_session_open(const C_CHAR *directory,
    const C_CHAR *file_name, integration_ini_session *out_session)
{
    return integration_ini_session_open_with_overlay_transform(directory, file_name,
        STD_NULL, STD_NULL, out_session);
}

type_status integration_ini_session_open_with_overlay_transform(const C_CHAR *directory,
    const C_CHAR *file_name, integration_ini_session_overlay_transform transform,
    C_VOID *opaque, integration_ini_session *out_session)
{
    type_status status;

    if (out_session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(out_session, 0, sizeof(*out_session));
    if (!integration_ini_session_find(directory, file_name, &out_session->request)) {
        STD_FPRINTF(STD_STDERR, "T533:INI-SESSION:%s:REQUEST-NOT-FOUND\n",
            file_name == STD_NULL ? "(null)" : file_name);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!integration_ini_session_assets_present(&out_session->request)) {
        STD_FPRINTF(STD_STDERR, "T533:INI-SESSION:%s:DECLARED-ASSET-UNAVAILABLE\n",
            file_name);
        return TYPE_STATUS_UNSUPPORTED;
    }
    out_session->transform = transform;
    out_session->transform_opaque = opaque;
    status = integration_ini_session_restart(out_session);
    if (status != TYPE_STATUS_OK || out_session->session == STD_NULL) {
        STD_FPRINTF(STD_STDERR, "T533:INI-SESSION:%s:OPEN-FAILED:%d\n",
            file_name, (C_INT)status);
        integration_ini_session_close(out_session);
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

type_status integration_ini_session_overlay_read(const integration_ini_session *session,
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

type_status integration_ini_session_overlay_write(integration_ini_session *session,
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

C_VOID integration_ini_session_close(integration_ini_session *session)
{
    if (session == STD_NULL) return;
    (C_VOID)common_machine_shutdown(session->common_machine);
    (C_VOID)vm_machine_bind_common_machine(session->session, LIB_NULL);
    (C_VOID)common_machine_destroy(session->common_machine);
    session->common_machine = LIB_NULL;
    vm_machine_destroy(session->session);
    STD_MEMSET(session, 0, sizeof(*session));
}
