#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include <windows.h>

#include "test/x86-nxvm/setup/session_ini.h"
#include "x86-nxvm/product/factory_interface.h"
#include VM_PRODUCT_BINDING_HEADER
#include "x86-nxvm/machine/machine_private.h"
#include "lib/base/sync_interface.h"

static lib_i32 integration_ini_session_find(const char *directory,
    const char *file_name, x86_nxvm_product_request *out_request)
{
    char path[X86_NXVM_PRODUCT_REQUEST_PATH_MAX];
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
    return x86_nxvm_product_ini_load(path, out_request) == LIB_STATUS_OK;
}

lib_i32 integration_ini_session_assets_present(
    const x86_nxvm_product_request *request)
{
    lib_size index;

    if (request == LIB_NULL) return 0;
    for (index = 0u; index < request->floppy_count; ++index)
        if (GetFileAttributesA(request->floppy[index]) == INVALID_FILE_ATTRIBUTES) return 0;
    for (index = 0u; index < request->fixed_disk_count; ++index)
        if (GetFileAttributesA(request->fixed_disk[index]) == INVALID_FILE_ATTRIBUTES) return 0;
    return 1;
}

lib_status integration_ini_session_restart(integration_ini_session *session)
{
    x86_nxvm_product_machine composed = {0};
    void *machine = LIB_NULL;
    lib_status status;

    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (session->emulator_machine != LIB_NULL) {
        (void)emulator_machine_shutdown(session->emulator_machine);
        (void)x86_nxvm_machine_bind_emulator_machine(session->session, LIB_NULL);
        (void)emulator_machine_destroy(session->emulator_machine);
        session->emulator_machine = LIB_NULL;
    }
    if (session->session != LIB_NULL) {
        x86_nxvm_machine_destroy(session->session);
        session->session = LIB_NULL;
    }
    status = x86_nxvm_product_compose_machine(&vm_app_machine, &session->request, &composed);
    machine = composed.composition.machine;
    session->session = machine;
    if (status != LIB_STATUS_OK) return status;
    if (session->session == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    if (session->transform != LIB_NULL) {
        status = session->transform(session, session->transform_opaque);
    }
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_destroy(session->session);
        session->session = LIB_NULL;
        return status;
    }
    return LIB_STATUS_OK;
}

static void integration_ini_session_state_fact(void *context,
    emulator_machine_state state, lib_u32 run_generation)
{
    integration_ini_session *session = context;

    (void)run_generation;
    if (state == EMULATOR_MACHINE_PAUSED) {
        (void)lib_atomic_u32_fetch_add_explicit(&session->paused_count,
            1u, LIB_MEMORY_ORDER_SEQ_CST);
    }
}

lib_status integration_ini_session_start(integration_ini_session *session)
{
    emulator_machine_driver driver;

    if (session == LIB_NULL || session->session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (session->emulator_machine == LIB_NULL) {
        lib_atomic_u32_initialize(&session->paused_count, 0u);
        if (x86_nxvm_machine_describe_emulator_driver(session->session, &driver) != LIB_STATUS_OK ||
            emulator_machine_create(&session->emulator_machine, &driver) != LIB_STATUS_OK ||
            x86_nxvm_machine_bind_emulator_machine(session->session, session->emulator_machine) !=
                LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
        emulator_machine_set_state_sink(session->emulator_machine,
            integration_ini_session_state_fact, session);
    }
    return emulator_machine_start(session->emulator_machine) ? LIB_STATUS_OK :
        LIB_STATUS_INVALID_STATE;
}

lib_i32 integration_ini_session_wait_for_state(const integration_ini_session *session,
    emulator_machine_state state, lib_u32 milliseconds)
{
    lib_u32 elapsed;

    if (session == LIB_NULL || session->emulator_machine == LIB_NULL) return 0;
    for (elapsed = 0u; elapsed < milliseconds; ++elapsed) {
        if (emulator_machine_state_get(session->emulator_machine) == state) return 1;
        base_sync_sleep_milliseconds(1u);
    }
    return emulator_machine_state_get(session->emulator_machine) == state;
}

lib_status integration_ini_session_pause(integration_ini_session *session,
    lib_u32 milliseconds)
{
    if (session == LIB_NULL || session->emulator_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!integration_ini_session_wait_for_state(session, EMULATOR_MACHINE_RUNNING,
            milliseconds)) return LIB_STATUS_INVALID_STATE;
    if (!emulator_machine_pause(session->emulator_machine)) return LIB_STATUS_INVALID_STATE;
    return integration_ini_session_wait_for_state(session, EMULATOR_MACHINE_PAUSED,
        milliseconds) ? LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}

lib_status integration_ini_session_resume(integration_ini_session *session,
    lib_u32 milliseconds)
{
    (void)milliseconds;
    if (session == LIB_NULL || session->emulator_machine == LIB_NULL ||
        !emulator_machine_resume(session->emulator_machine)) return LIB_STATUS_INVALID_STATE;
    return LIB_STATUS_OK;
}

lib_status integration_ini_session_resume_until_paused(integration_ini_session *session,
    lib_u32 milliseconds)
{
    lib_u32 before_pause;
    lib_u32 elapsed;
    lib_status status;

    if (session == LIB_NULL || session->emulator_machine == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    before_pause = lib_atomic_u32_load_explicit(&session->paused_count,
        LIB_MEMORY_ORDER_SEQ_CST);
    status = integration_ini_session_resume(session, milliseconds);
    if (status != LIB_STATUS_OK) return status;
    for (elapsed = 0u; elapsed < milliseconds; ++elapsed) {
        if (lib_atomic_u32_load_explicit(&session->paused_count,
                LIB_MEMORY_ORDER_SEQ_CST) != before_pause) return LIB_STATUS_OK;
        base_sync_sleep_milliseconds(1u);
    }
    return lib_atomic_u32_load_explicit(&session->paused_count,
        LIB_MEMORY_ORDER_SEQ_CST) != before_pause ? LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}

lib_status integration_ini_session_reset(integration_ini_session *session,
    lib_u32 milliseconds)
{
    lib_u32 generation;
    lib_u32 elapsed;

    if (session == LIB_NULL || session->emulator_machine == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    generation = emulator_machine_run_generation(session->emulator_machine);
    if (!emulator_machine_reset(session->emulator_machine)) return LIB_STATUS_INVALID_STATE;
    for (elapsed = 0u; elapsed < milliseconds; ++elapsed) {
        if (emulator_machine_run_generation(session->emulator_machine) != generation &&
            emulator_machine_state_get(session->emulator_machine) == EMULATOR_MACHINE_PAUSED)
            return LIB_STATUS_OK;
        base_sync_sleep_milliseconds(1u);
    }
    return emulator_machine_run_generation(session->emulator_machine) != generation &&
        emulator_machine_state_get(session->emulator_machine) == EMULATOR_MACHINE_PAUSED ?
        LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}

lib_status integration_ini_session_open(const char *directory,
    const char *file_name, integration_ini_session *out_session)
{
    return integration_ini_session_open_with_overlay_transform(directory, file_name,
        LIB_NULL, LIB_NULL, out_session);
}

lib_status integration_ini_session_open_with_overlay_transform(const char *directory,
    const char *file_name, integration_ini_session_overlay_transform transform,
    void *opaque, integration_ini_session *out_session)
{
    lib_status status;

    if (out_session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_session, 0, sizeof(*out_session));
    if (!integration_ini_session_find(directory, file_name, &out_session->request)) {
        fprintf(stderr, "NXVM:INI-SESSION:%s:REQUEST-NOT-FOUND\n",
            file_name == LIB_NULL ? "(null)" : file_name);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!integration_ini_session_assets_present(&out_session->request)) {
        fprintf(stderr, "NXVM:INI-SESSION:%s:DECLARED-ASSET-UNAVAILABLE\n",
            file_name);
        return LIB_STATUS_UNSUPPORTED;
    }
    out_session->transform = transform;
    out_session->transform_opaque = opaque;
    status = integration_ini_session_restart(out_session);
    if (status != LIB_STATUS_OK || out_session->session == LIB_NULL) {
        fprintf(stderr, "NXVM:INI-SESSION:%s:OPEN-FAILED:%d\n",
            file_name, (lib_i32)status);
        integration_ini_session_close(out_session);
        return status == LIB_STATUS_OK ? LIB_STATUS_INTERNAL_ERROR : status;
    }
    return LIB_STATUS_OK;
}

lib_status integration_ini_session_overlay_read(const integration_ini_session *session,
    x86_nxvm_board_media_id id, void **out_bytes, lib_size *out_count)
{
    x86_nxvm_board_media_info info;
    x86_nxvm_board_media_result result;
    lib_size count;
    void *bytes;

    if (session == LIB_NULL || session->session == LIB_NULL || out_bytes == LIB_NULL ||
        out_count == LIB_NULL || x86_nxvm_board_media_query(session->session->media_registry,
            id, &info, &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        info.geometry.bytes_per_sector == 0u ||
        info.geometry.logical_sector_count > (lib_size)-1 / info.geometry.bytes_per_sector) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    count = (lib_size)info.geometry.logical_sector_count * info.geometry.bytes_per_sector;
    if (count == 0u || count > LIB_UINT32_MAX) return LIB_STATUS_INVALID_ARGUMENT;
    bytes = lib_allocate(count);
    if (bytes == LIB_NULL || x86_nxvm_board_media_read_bytes(session->session->media_registry,
            id, 0u, bytes, (lib_u32)count, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_OK) {
        lib_release(bytes);
        return LIB_STATUS_INTERNAL_ERROR;
    }
    *out_bytes = bytes;
    *out_count = count;
    return LIB_STATUS_OK;
}

lib_status integration_ini_session_overlay_write(integration_ini_session *session,
    x86_nxvm_board_media_id id, const void *bytes, lib_size byte_count)
{
    x86_nxvm_board_media_result result;

    if (session == LIB_NULL || session->session == LIB_NULL || bytes == LIB_NULL ||
        byte_count > LIB_UINT32_MAX || x86_nxvm_board_media_write_bytes(
            session->session->media_registry, id, 0u, bytes,
            (lib_u32)byte_count, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_OK) return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}

void integration_ini_session_close(integration_ini_session *session)
{
    if (session == LIB_NULL) return;
    (void)emulator_machine_shutdown(session->emulator_machine);
    (void)x86_nxvm_machine_bind_emulator_machine(session->session, LIB_NULL);
    (void)emulator_machine_destroy(session->emulator_machine);
    session->emulator_machine = LIB_NULL;
    x86_nxvm_machine_destroy(session->session);
    lib_memory_set(session, 0, sizeof(*session));
}
