#include "lib/types/types_interface.h"
#include "type.h"

#include <windows.h>

#include "test/app-nxvm/integration/support/session_ini.h"
#include "app-nxvm/product/config.h"
#include "app-nxvm/machine/machine_private.h"
#include "lib/base/sync_interface.h"

static C_INT integration_ini_session_find(const C_CHAR *directory,
    const C_CHAR *file_name, vm_session_request *out_request)
{
    C_CHAR path[VM_SESSION_REQUEST_PATH_MAX];
    lib_size directory_bytes;
    lib_size file_bytes;

    if (directory == LIB_NULL || file_name == LIB_NULL || out_request == LIB_NULL) return 0;
    directory_bytes = lib_text_length(directory);
    file_bytes = lib_text_length(file_name);
    if (directory_bytes + 1u + file_bytes >= sizeof(path)) return 0;
    lib_memory_copy(path, directory, directory_bytes);
    if (directory_bytes != 0u && directory[directory_bytes - 1u] != '/' &&
        directory[directory_bytes - 1u] != '\\') path[directory_bytes++] = '/';
    lib_memory_copy(path + directory_bytes, file_name, file_bytes + 1u);
    return vm_app_ini_load(path, out_request) == TYPE_STATUS_OK;
}

C_INT integration_ini_session_assets_present(
    const vm_session_request *request)
{
    lib_size index;

    if (request == LIB_NULL) return 0;
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

    if (session == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (session->common_machine != LIB_NULL) {
        (C_VOID)common_machine_shutdown(session->common_machine);
        (C_VOID)vm_machine_bind_common_machine(session->session, LIB_NULL);
        (C_VOID)common_machine_destroy(session->common_machine);
        session->common_machine = LIB_NULL;
    }
    if (session->session != LIB_NULL) {
        vm_machine_destroy(session->session);
        session->session = LIB_NULL;
    }
    status = vm_app_configure_machine(&session->request, &configuration);
    if (status == TYPE_STATUS_OK)
        status = vm_machine_create(&configuration, &session->session);
    if (status != TYPE_STATUS_OK || session->session == LIB_NULL) return TYPE_STATUS_FAULT;
    if (session->transform != LIB_NULL && session->transform(session,
            session->transform_opaque) != TYPE_STATUS_OK) {
        vm_machine_destroy(session->session);
        session->session = LIB_NULL;
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

type_status integration_ini_session_start(integration_ini_session *session)
{
    common_machine_driver driver;

    if (session == LIB_NULL || session->session == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (session->common_machine == LIB_NULL &&
        (vm_machine_describe_common_driver(session->session, &driver) != TYPE_STATUS_OK ||
        common_machine_create(&session->common_machine, &driver) != LIB_STATUS_OK ||
        vm_machine_bind_common_machine(session->session, session->common_machine) !=
            TYPE_STATUS_OK)) return TYPE_STATUS_FAULT;
    return common_machine_start(session->common_machine) ? TYPE_STATUS_OK :
        TYPE_STATUS_INVALID_STATE;
}

C_INT integration_ini_session_wait_for_state(const integration_ini_session *session,
    common_machine_state state, C_UINT milliseconds)
{
    C_UINT elapsed;

    if (session == LIB_NULL || session->common_machine == LIB_NULL) return 0;
    for (elapsed = 0u; elapsed < milliseconds; ++elapsed) {
        if (common_machine_state_get(session->common_machine) == state) return 1;
        base_sync_sleep_milliseconds(1u);
    }
    return common_machine_state_get(session->common_machine) == state;
}

type_status integration_ini_session_pause(integration_ini_session *session,
    C_UINT milliseconds)
{
    if (session == LIB_NULL || session->common_machine == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (!integration_ini_session_wait_for_state(session, COMMON_MACHINE_RUNNING,
            milliseconds)) return TYPE_STATUS_INVALID_STATE;
    if (!common_machine_pause(session->common_machine)) return TYPE_STATUS_INVALID_STATE;
    return integration_ini_session_wait_for_state(session, COMMON_MACHINE_PAUSED,
        milliseconds) ? TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}

type_status integration_ini_session_resume(integration_ini_session *session,
    C_UINT milliseconds)
{
    (C_VOID)milliseconds;
    if (session == LIB_NULL || session->common_machine == LIB_NULL ||
        !common_machine_resume(session->common_machine)) return TYPE_STATUS_INVALID_STATE;
    return TYPE_STATUS_OK;
}

type_status integration_ini_session_reset(integration_ini_session *session,
    C_UINT milliseconds)
{
    lib_u32 generation;
    C_UINT elapsed;

    if (session == LIB_NULL || session->common_machine == LIB_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    generation = common_machine_run_generation(session->common_machine);
    if (!common_machine_reset(session->common_machine)) return TYPE_STATUS_INVALID_STATE;
    for (elapsed = 0u; elapsed < milliseconds; ++elapsed) {
        if (common_machine_run_generation(session->common_machine) != generation &&
            common_machine_state_get(session->common_machine) == COMMON_MACHINE_PAUSED)
            return TYPE_STATUS_OK;
        base_sync_sleep_milliseconds(1u);
    }
    return common_machine_run_generation(session->common_machine) != generation &&
        common_machine_state_get(session->common_machine) == COMMON_MACHINE_PAUSED ?
        TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}

type_status integration_ini_session_open(const C_CHAR *directory,
    const C_CHAR *file_name, integration_ini_session *out_session)
{
    return integration_ini_session_open_with_overlay_transform(directory, file_name,
        LIB_NULL, LIB_NULL, out_session);
}

type_status integration_ini_session_open_with_overlay_transform(const C_CHAR *directory,
    const C_CHAR *file_name, integration_ini_session_overlay_transform transform,
    C_VOID *opaque, integration_ini_session *out_session)
{
    type_status status;

    if (out_session == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_session, 0, sizeof(*out_session));
    if (!integration_ini_session_find(directory, file_name, &out_session->request)) {
        STD_FPRINTF(STD_STDERR, "T533:INI-SESSION:%s:REQUEST-NOT-FOUND\n",
            file_name == LIB_NULL ? "(null)" : file_name);
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
    if (status != TYPE_STATUS_OK || out_session->session == LIB_NULL) {
        STD_FPRINTF(STD_STDERR, "T533:INI-SESSION:%s:OPEN-FAILED:%d\n",
            file_name, (C_INT)status);
        integration_ini_session_close(out_session);
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

type_status integration_ini_session_overlay_read(const integration_ini_session *session,
    core_machine_media_id id, C_VOID **out_bytes, lib_size *out_count)
{
    core_machine_media_info info;
    core_machine_media_result result;
    lib_size count;
    C_VOID *bytes;

    if (session == LIB_NULL || session->session == LIB_NULL || out_bytes == LIB_NULL ||
        out_count == LIB_NULL || core_machine_media_query(session->session->media_registry,
            id, &info, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        info.geometry.bytes_per_sector == 0u ||
        info.geometry.logical_sector_count > (lib_size)-1 / info.geometry.bytes_per_sector) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    count = (lib_size)info.geometry.logical_sector_count * info.geometry.bytes_per_sector;
    if (count == 0u || count > TYPE_MAX_UNSIGNED_32) return TYPE_STATUS_INVALID_ARGUMENT;
    bytes = lib_allocate(count);
    if (bytes == LIB_NULL || core_machine_media_read_bytes(session->session->media_registry,
            id, 0u, bytes, (lib_u32)count, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK) {
        lib_release(bytes);
        return TYPE_STATUS_FAULT;
    }
    *out_bytes = bytes;
    *out_count = count;
    return TYPE_STATUS_OK;
}

type_status integration_ini_session_overlay_write(integration_ini_session *session,
    core_machine_media_id id, const C_VOID *bytes, lib_size byte_count)
{
    core_machine_media_result result;

    if (session == LIB_NULL || session->session == LIB_NULL || bytes == LIB_NULL ||
        byte_count > TYPE_MAX_UNSIGNED_32 || core_machine_media_write_bytes(
            session->session->media_registry, id, 0u, bytes,
            (lib_u32)byte_count, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK) return TYPE_STATUS_INVALID_ARGUMENT;
    return TYPE_STATUS_OK;
}

C_VOID integration_ini_session_close(integration_ini_session *session)
{
    if (session == LIB_NULL) return;
    (C_VOID)common_machine_shutdown(session->common_machine);
    (C_VOID)vm_machine_bind_common_machine(session->session, LIB_NULL);
    (C_VOID)common_machine_destroy(session->common_machine);
    session->common_machine = LIB_NULL;
    vm_machine_destroy(session->session);
    lib_memory_set(session, 0, sizeof(*session));
}
