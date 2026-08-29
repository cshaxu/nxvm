#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_INT15_MEMORY_BOOT_BUDGET 100000u

static C_INT vm_int15_memory_write_fixture(C_CHAR path[MAX_PATH],
    STD_SIZE_T image_bytes)
{
    static const type_unsigned_8 boot_code[] = {
        0x31u, 0xc0u,             /* xor ax,ax */
        0x8eu, 0xd8u,             /* mov ds,ax */
        0xb4u, 0x88u,             /* mov ah,88h */
        0xcdu, 0x15u,             /* int 15h */
        0xa3u, 0x00u, 0x05u,      /* mov [0500h],ax */
        0x74u, 0x04u,            /* jz success */
        0xb0u, 0x00u, 0xebu, 0x02u, /* mov al,0; jmp store */
        0xb0u, 0x40u,            /* success: mov al,40h */
        0xa2u, 0x02u, 0x05u,      /* store: mov [0502h],al */
        0xf4u, 0xebu, 0xfeu       /* hlt; jmp $ */
    };
    type_unsigned_8 sector[512] = {0};
    type_unsigned_8 zeros[512] = {0};
    STD_FILE *file;
    DWORD length;
    STD_SIZE_T written;

    if (image_bytes < sizeof(sector) || image_bytes % sizeof(sector) != 0u) {
        return 0;
    }
    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMCPY(sector, boot_code, sizeof(boot_code));
    sector[510u] = 0x55u;
    sector[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FWRITE(sector, 1u, sizeof(sector), file) !=
            sizeof(sector)) {
        if (file != STD_NULL) STD_FCLOSE(file);
        DeleteFileA(path);
        return 0;
    }
    for (written = sizeof(sector); written < image_bytes;
        written += sizeof(zeros)) {
        if (STD_FWRITE(zeros, 1u, sizeof(zeros), file) != sizeof(zeros)) {
            STD_FCLOSE(file);
            DeleteFileA(path);
            return 0;
        }
    }
    STD_FCLOSE(file);
    return 1;
}

static C_INT vm_int15_move_write_fixture(C_CHAR path[MAX_PATH],
    STD_SIZE_T image_bytes)
{
    static const type_unsigned_8 boot_code[] = {
        0x31u, 0xc0u,             /* xor ax,ax */
        0x8eu, 0xd8u,             /* mov ds,ax */
        0xb8u, 0x60u, 0x00u,      /* mov ax,0060h */
        0x8eu, 0xc0u,             /* mov es,ax */
        0xbeu, 0x00u, 0x00u,      /* mov si,0000h */
        0xb9u, 0x02u, 0x00u,      /* mov cx,2 */
        0xb8u, 0x00u, 0x87u,      /* mov ax,8700h */
        0xcdu, 0x15u,             /* int 15h */
        0xa3u, 0x00u, 0x05u,      /* mov [0500h],ax */
        0xf4u, 0xebu, 0xfeu       /* hlt; jmp $ */
    };
    type_unsigned_8 sector[512] = {0};
    type_unsigned_8 zeros[512] = {0};
    STD_FILE *file;
    DWORD length;
    STD_SIZE_T written;

    if (image_bytes < sizeof(sector) || image_bytes % sizeof(sector) != 0u) {
        return 0;
    }
    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMCPY(sector, boot_code, sizeof(boot_code));
    sector[510u] = 0x55u;
    sector[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FWRITE(sector, 1u, sizeof(sector), file) !=
            sizeof(sector)) {
        if (file != STD_NULL) STD_FCLOSE(file);
        DeleteFileA(path);
        return 0;
    }
    for (written = sizeof(sector); written < image_bytes;
        written += sizeof(zeros)) {
        if (STD_FWRITE(zeros, 1u, sizeof(zeros), file) != sizeof(zeros)) {
            STD_FCLOSE(file);
            DeleteFileA(path);
            return 0;
        }
    }
    STD_FCLOSE(file);
    return 1;
}

static C_INT vm_int15_memory_run(vm_session_profile_kind profile_kind,
    vm_session_floppy_format format, STD_SIZE_T image_bytes,
    type_unsigned_16 expected_kib)
{
    const core_machine_run_budget budget = {1u, 0u};
    vm_session_config config = {0};
    core_machine_run_result result;
    vm_session *session = STD_NULL;
    C_CHAR path[MAX_PATH] = {0};
    type_unsigned_16 kib = 0xffffu;
    type_unsigned_8 flags = 0xffu;
    type_unsigned_32 step = 0u;
    C_INT passed = 0;

    if (!vm_int15_memory_write_fixture(path, image_bytes)) goto done;
    config.profile_kind = profile_kind;
    config.floppy_format = format;
    config.fdd_image = path;
    if (vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto done;
    for (step = 0u; step < VM_INT15_MEMORY_BOOT_BUDGET; ++step) {
        if (core_machine_run(session->core_machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) goto done;
        if (core_machine_memory_read(session->core_machine, 0x0500u, &kib,
                sizeof(kib)) != TYPE_STATUS_OK || kib != expected_kib) continue;
        if (core_machine_memory_read(session->core_machine, 0x0502u, &flags,
                sizeof(flags)) != TYPE_STATUS_OK) goto done;
        passed = (flags & 1u) == 0u;
        break;
    }
done:
    if (session != STD_NULL) vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    return passed;
}

static C_INT vm_int15_move_run(vm_session_profile_kind profile_kind,
    vm_session_floppy_format format, STD_SIZE_T image_bytes, C_INT replace_vector,
    type_unsigned_32 target_physical, const type_unsigned_8 *expected_target,
    C_INT expected_success)
{
    type_unsigned_8 table[0x30] = {
        [0x10] = 0x03u, [0x12] = 0x00u, [0x13] = 0x07u,
        [0x18] = 0x03u, [0x1a] = 0x00u, [0x1b] = 0x08u
    };
    static const type_unsigned_8 source[] = { 0x12u, 0x34u, 0x56u, 0x78u };
    static const type_unsigned_8 iret[] = { 0xcfu };
    static const type_unsigned_8 replacement_vector[] = { 0x00u, 0x09u, 0u, 0u };
    const type_unsigned_16 sentinel = 0xffffu;
    const core_machine_run_budget budget = {1u, 0u};
    vm_session_config config = {0};
    core_machine_run_result result;
    vm_session *session = STD_NULL;
    C_CHAR path[MAX_PATH] = {0};
    type_unsigned_16 ax = 0xffffu;
    type_unsigned_8 target[sizeof(source)] = {0};
    type_unsigned_8 low_sentinel = 0xa5u;
    core_machine_cpu_state cpu_state = {0};
    type_unsigned_32 step = 0u;
    C_INT passed = 0;

    if (!vm_int15_move_write_fixture(path, image_bytes)) goto done;
    table[0x1au] = TYPE_MASK_UNSIGNED_8(target_physical);
    table[0x1bu] = TYPE_MASK_UNSIGNED_8(target_physical >> 8u);
    table[0x1cu] = TYPE_MASK_UNSIGNED_8(target_physical >> 16u);
    config.profile_kind = profile_kind;
    config.floppy_format = format;
    config.fdd_image = path;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    if (vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL ||
        core_machine_memory_write(session->core_machine, 0x0600u, table,
            sizeof(table)) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine, 0x0700u, source,
            sizeof(source)) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine, 0x0500u, &sentinel,
            sizeof(sentinel)) != TYPE_STATUS_OK) goto done;
    if (target_physical >= 0x00100000u &&
        core_machine_memory_write(session->core_machine, 0u, &low_sentinel,
            sizeof(low_sentinel)) != TYPE_STATUS_OK) goto done;
    if (replace_vector &&
        (core_machine_memory_write(session->core_machine, 0x054u,
            replacement_vector, sizeof(replacement_vector)) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine, 0x0900u, iret,
            sizeof(iret)) != TYPE_STATUS_OK)) goto done;
    for (step = 0u; step < VM_INT15_MEMORY_BOOT_BUDGET; ++step) {
        if (core_machine_run(session->core_machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) goto done;
        if (core_machine_memory_read(session->core_machine, 0x0500u, &ax,
                sizeof(ax)) != TYPE_STATUS_OK || ax == 0xffffu) continue;
        if (expected_success && target_physical < 0x00100000u &&
            core_machine_memory_read(session->core_machine,
                target_physical, target, sizeof(target)) != TYPE_STATUS_OK) goto done;
        if (core_machine_get_cpu_state(session->core_machine, &cpu_state) !=
            TYPE_STATUS_OK) goto done;
        if (replace_vector) {
            passed = ax == 0x8700u &&
                STD_MEMCMP(target, (const type_unsigned_8[sizeof(target)]){0},
                    sizeof(target)) == 0;
        } else if (expected_success) {
            passed = expected_target != STD_NULL && ax == 0u &&
                (cpu_state.eflags & 0x41u) == 0x40u &&
                (target_physical >= 0x00100000u ?
                    core_machine_memory_read(session->core_machine, 0u,
                        &target[0u], sizeof(target[0u])) == TYPE_STATUS_OK &&
                    target[0u] == low_sentinel :
                    STD_MEMCMP(target, expected_target, sizeof(target)) == 0);
        } else {
            passed = ax == 0x0200u && (cpu_state.eflags & 0x01u) != 0u;
        }
        break;
    }
done:
    if (session != STD_NULL) vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    return passed;
}

C_INT main(C_VOID)
{
    if (!vm_int15_memory_run(VM_SESSION_PROFILE_IBM_5170_MODEL_339,
            VM_SESSION_FLOPPY_FORMAT_1200K, 1200u * 1024u, 0u) ||
        !vm_int15_memory_run(VM_SESSION_PROFILE_DEFAULT_PC_AT,
            VM_SESSION_FLOPPY_FORMAT_1440K, 1440u * 1024u, 0x3c00u) ||
        !vm_int15_move_run(VM_SESSION_PROFILE_IBM_5170_MODEL_339,
            VM_SESSION_FLOPPY_FORMAT_1200K, 1200u * 1024u, 0, 0x0800u,
            (const type_unsigned_8[]){0x12u, 0x34u, 0x56u, 0x78u}, 1) ||
        !vm_int15_move_run(VM_SESSION_PROFILE_DEFAULT_PC_AT,
            VM_SESSION_FLOPPY_FORMAT_1440K, 1440u * 1024u, 0, 0x0800u,
            (const type_unsigned_8[]){0x12u, 0x34u, 0x56u, 0x78u}, 1) ||
        !vm_int15_move_run(VM_SESSION_PROFILE_DEFAULT_PC_AT,
            VM_SESSION_FLOPPY_FORMAT_1440K, 1440u * 1024u, 1, 0x0800u,
            (const type_unsigned_8[]){0x12u, 0x34u, 0x56u, 0x78u}, 1) ||
        !vm_int15_move_run(VM_SESSION_PROFILE_DEFAULT_PC_AT,
            VM_SESSION_FLOPPY_FORMAT_1440K, 1440u * 1024u, 0, 0x0702u,
            (const type_unsigned_8[]){0x12u, 0x34u, 0x12u, 0x34u}, 1) ||
        !vm_int15_move_run(VM_SESSION_PROFILE_IBM_5170_MODEL_339,
            VM_SESSION_FLOPPY_FORMAT_1200K, 1200u * 1024u, 0, 0x00100000u,
            (const type_unsigned_8[]){0u, 0u, 0u, 0u}, 1)) {
        return 1;
    }
    puts("M5:T498:S5:INT15-MEMORY:OK");
    return 0;
}
