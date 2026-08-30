#include "type.h"

#include <windows.h>

#include "core/platform/presentation_mailbox_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_PROFILE_FLOPPY_BOOT_TIMEOUT_MILLISECONDS 60000u
#define VM_PROFILE_FLOPPY_BOOT_POLL_MILLISECONDS 10u
#define VM_PROFILE_FLOPPY_TEXT_CELLS (80u * 25u)
#define VM_PROFILE_FLOPPY_UNAVAILABLE 77

typedef enum vm_profile_floppy_boot_terminal {
    VM_PROFILE_FLOPPY_BOOT_TERMINAL_NONE,
    VM_PROFILE_FLOPPY_BOOT_TERMINAL_DOS_PROMPT,
    VM_PROFILE_FLOPPY_BOOT_TERMINAL_DATE_INPUT,
    VM_PROFILE_FLOPPY_BOOT_TERMINAL_INSTALLER
} vm_profile_floppy_boot_terminal;

typedef struct vm_profile_floppy_boot_row {
    const C_CHAR *id;
    vm_session_profile_kind profile_kind;
    core_machine_cpu_profile cpu_profile;
    vm_session_floppy_format floppy_format;
} vm_profile_floppy_boot_row;

static const vm_profile_floppy_boot_row vm_profile_floppy_boot_rows[] = {
    {"ibm-5160-8088-360k", VM_SESSION_PROFILE_IBM_5160_MODEL_268,
        CORE_MACHINE_CPU_PROFILE_8088, VM_SESSION_FLOPPY_FORMAT_360K},
    {"ibm-5170-80286-360k", VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        CORE_MACHINE_CPU_PROFILE_80286, VM_SESSION_FLOPPY_FORMAT_360K},
    {"ibm-5170-80286-1200k", VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        CORE_MACHINE_CPU_PROFILE_80286, VM_SESSION_FLOPPY_FORMAT_1200K},
    {"default-at-8086-360k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_8086, VM_SESSION_FLOPPY_FORMAT_360K},
    {"default-at-8086-720k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_8086, VM_SESSION_FLOPPY_FORMAT_720K},
    {"default-at-8086-1200k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_8086, VM_SESSION_FLOPPY_FORMAT_1200K},
    {"default-at-8086-1440k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_8086, VM_SESSION_FLOPPY_FORMAT_1440K},
    {"default-at-80186-360k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80186, VM_SESSION_FLOPPY_FORMAT_360K},
    {"default-at-80186-720k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80186, VM_SESSION_FLOPPY_FORMAT_720K},
    {"default-at-80186-1200k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80186, VM_SESSION_FLOPPY_FORMAT_1200K},
    {"default-at-80186-1440k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80186, VM_SESSION_FLOPPY_FORMAT_1440K},
    {"default-at-80286-360k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80286, VM_SESSION_FLOPPY_FORMAT_360K},
    {"default-at-80286-720k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80286, VM_SESSION_FLOPPY_FORMAT_720K},
    {"default-at-80286-1200k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80286, VM_SESSION_FLOPPY_FORMAT_1200K},
    {"default-at-80286-1440k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80286, VM_SESSION_FLOPPY_FORMAT_1440K},
    {"default-at-80386-360k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80386, VM_SESSION_FLOPPY_FORMAT_360K},
    {"default-at-80386-720k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80386, VM_SESSION_FLOPPY_FORMAT_720K},
    {"default-at-80386-1200k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80386, VM_SESSION_FLOPPY_FORMAT_1200K},
    {"default-at-80386-1440k", VM_SESSION_PROFILE_DEFAULT_PC_AT,
        CORE_MACHINE_CPU_PROFILE_80386, VM_SESSION_FLOPPY_FORMAT_1440K},
    {"model40-80386-1200k", VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        CORE_MACHINE_CPU_PROFILE_80386, VM_SESSION_FLOPPY_FORMAT_1200K}
};

static const vm_profile_floppy_boot_row *vm_profile_floppy_boot_row_find(
    const C_CHAR *id)
{
    STD_SIZE_T index;

    if (id == STD_NULL) return STD_NULL;
    for (index = 0u; index < sizeof(vm_profile_floppy_boot_rows) /
            sizeof(vm_profile_floppy_boot_rows[0]); ++index) {
        if (!STD_STRCMP(vm_profile_floppy_boot_rows[index].id, id)) {
            return &vm_profile_floppy_boot_rows[index];
        }
    }
    return STD_NULL;
}

static C_INT vm_profile_floppy_boot_text_has(const core_platform_display_frame *frame,
    const C_CHAR *text)
{
    const STD_SIZE_T length = text == STD_NULL ? 0u : STD_STRLEN(text);
    STD_SIZE_T cell;

    if (frame == STD_NULL || length == 0u || length > VM_PROFILE_FLOPPY_TEXT_CELLS) {
        return 0;
    }
    for (cell = 0u; cell + length <= VM_PROFILE_FLOPPY_TEXT_CELLS; ++cell) {
        if (STD_MEMCMP(&frame->characters[cell], text, length) == 0) return 1;
    }
    return 0;
}

static vm_profile_floppy_boot_terminal vm_profile_floppy_boot_terminal_get(
    const vm_session *session)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;

    if (session == STD_NULL || core_platform_presentation_mailbox_capture(
            session->presentation_mailbox, &frame) != TYPE_STATUS_OK ||
        frame.kind != CORE_PLATFORM_DISPLAY_KIND_TEXT) return VM_PROFILE_FLOPPY_BOOT_TERMINAL_NONE;
    for (cell = 0u; cell + 3u < VM_PROFILE_FLOPPY_TEXT_CELLS; ++cell) {
        if (STD_ISALPHA((C_UCHAR)frame.characters[cell]) &&
            frame.characters[cell + 1u] == ':' && frame.characters[cell + 2u] == '\\' &&
            frame.characters[cell + 3u] == '>') return VM_PROFILE_FLOPPY_BOOT_TERMINAL_DOS_PROMPT;
    }
    if (vm_profile_floppy_boot_text_has(&frame, "Enter new date")) {
        return VM_PROFILE_FLOPPY_BOOT_TERMINAL_DATE_INPUT;
    }
    return vm_profile_floppy_boot_text_has(&frame, "ENTER=Continue") ?
        VM_PROFILE_FLOPPY_BOOT_TERMINAL_INSTALLER : VM_PROFILE_FLOPPY_BOOT_TERMINAL_NONE;
}

static const C_CHAR *vm_profile_floppy_boot_terminal_name(
    vm_profile_floppy_boot_terminal terminal)
{
    if (terminal == VM_PROFILE_FLOPPY_BOOT_TERMINAL_DOS_PROMPT) return "dos-prompt";
    if (terminal == VM_PROFILE_FLOPPY_BOOT_TERMINAL_DATE_INPUT) return "date-input";
    return terminal == VM_PROFILE_FLOPPY_BOOT_TERMINAL_INSTALLER ? "installer-ready" :
        "none";
}

static DWORD WINAPI vm_profile_floppy_boot_start(C_VOID *opaque)
{
    (C_VOID)vm_session_start((vm_session *)opaque);
    return 0u;
}

static C_INT vm_profile_floppy_boot_configure(const vm_profile_floppy_boot_row *row,
    C_INT argc, C_CHAR **argv, vm_session_config *out_config)
{
    if (row == STD_NULL || argv == STD_NULL || out_config == STD_NULL) return 0;
    *out_config = (vm_session_config) {0};
    out_config->profile_kind = row->profile_kind;
    out_config->fdd_image = argv[2];
    if (row->profile_kind == VM_SESSION_PROFILE_DEFAULT_PC_AT) {
        if (argc != 3) return 0;
        out_config->cpu_profile = row->cpu_profile;
        out_config->fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
        out_config->floppy_format = row->floppy_format;
        return 1;
    }
    if (row->profile_kind == VM_SESSION_PROFILE_IBM_5170_MODEL_339) {
        if (argc != 3) return 0;
        out_config->floppy_format = row->floppy_format;
        return 1;
    }
    if (argc != 7) return 0;
    if (row->profile_kind == VM_SESSION_PROFILE_IBM_5160_MODEL_268) {
        out_config->xt_firmware = (vm_profile_xt_5160_268_byob_manifest) {
            argv[3], argv[4], argv[5], argv[6], "owner-provided integration input"};
        return 1;
    }
    if (row->profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        out_config->model40_firmware = (vm_profile_model40_byob_manifest) {
            argv[3], argv[4], argv[5], argv[6], "owner-provided integration input"};
        return 1;
    }
    return 0;
}

static C_INT vm_profile_floppy_boot_inputs_are_present(
    const vm_profile_floppy_boot_row *row, C_INT argc, C_CHAR **argv)
{
    C_INT index;

    if (row == STD_NULL || argv == STD_NULL || argc < 3 ||
        GetFileAttributesA(argv[2]) == INVALID_FILE_ATTRIBUTES) return 0;
    if (row->profile_kind != VM_SESSION_PROFILE_IBM_5160_MODEL_268 &&
        row->profile_kind != VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) return 1;
    if (argc != 7) return 0;
    for (index = 3; index < 7; index += 2) {
        if (GetFileAttributesA(argv[index]) == INVALID_FILE_ATTRIBUTES) return 0;
    }
    return 1;
}

static C_INT vm_profile_floppy_boot_validate(C_VOID)
{
    STD_SIZE_T index;
    STD_SIZE_T peer;

    for (index = 0u; index < sizeof(vm_profile_floppy_boot_rows) /
            sizeof(vm_profile_floppy_boot_rows[0]); ++index) {
        const vm_profile_floppy_boot_row *row = &vm_profile_floppy_boot_rows[index];

        if (row->id == STD_NULL || row->floppy_format < VM_SESSION_FLOPPY_FORMAT_360K ||
            row->floppy_format > VM_SESSION_FLOPPY_FORMAT_1440K) return 0;
        for (peer = 0u; peer < index; ++peer) {
            if (!STD_STRCMP(row->id, vm_profile_floppy_boot_rows[peer].id)) return 0;
        }
    }
    return sizeof(vm_profile_floppy_boot_rows) / sizeof(vm_profile_floppy_boot_rows[0]) == 20u;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    const vm_profile_floppy_boot_row *row;
    vm_session_config config;
    vm_session *session = STD_NULL;
    HANDLE thread = STD_NULL;
    vm_profile_floppy_boot_terminal terminal = VM_PROFILE_FLOPPY_BOOT_TERMINAL_NONE;
    DWORD elapsed;
    C_INT result = 1;

    if (argc == 2 && !STD_STRCMP(argv[1], "--validate")) {
        if (!vm_profile_floppy_boot_validate()) return 1;
        STD_PRINTF("T513:PROFILE-FLOPPY-MATRIX:VALID:20\n");
        return 0;
    }
    if (argc < 3 || !vm_profile_floppy_boot_validate() ||
        (row = vm_profile_floppy_boot_row_find(argv[1])) == STD_NULL) return 1;
    if (!vm_profile_floppy_boot_configure(row, argc, argv, &config)) return 1;
    if (!vm_profile_floppy_boot_inputs_are_present(row, argc, argv)) {
        STD_PRINTF("T513:PROFILE-FLOPPY-MATRIX:%s:UNAVAILABLE\n", row->id);
        return VM_PROFILE_FLOPPY_UNAVAILABLE;
    }
    if (vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL) {
        STD_PRINTF("T513:PROFILE-FLOPPY-MATRIX:%s:SESSION-CREATE-FAILED\n", row->id);
        return 1;
    }
    thread = CreateThread(STD_NULL, 0u, vm_profile_floppy_boot_start, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto done;
    for (elapsed = 0u; elapsed < VM_PROFILE_FLOPPY_BOOT_TIMEOUT_MILLISECONDS;
            elapsed += VM_PROFILE_FLOPPY_BOOT_POLL_MILLISECONDS) {
        terminal = vm_profile_floppy_boot_terminal_get(session);
        if (terminal != VM_PROFILE_FLOPPY_BOOT_TERMINAL_NONE) break;
        Sleep(VM_PROFILE_FLOPPY_BOOT_POLL_MILLISECONDS);
    }
    vm_session_stop(session);
    if (WaitForSingleObject(thread, 2000u) != WAIT_OBJECT_0) {
        CloseHandle(thread);
        STD_PRINTF("T513:PROFILE-FLOPPY-MATRIX:%s:STOP-TIMEOUT\n", row->id);
        return 1;
    }
    CloseHandle(thread);
    thread = STD_NULL;
    if (terminal == VM_PROFILE_FLOPPY_BOOT_TERMINAL_NONE) {
        STD_PRINTF("T513:PROFILE-FLOPPY-MATRIX:%s:TERMINAL-TIMEOUT\n", row->id);
        goto done;
    }
    STD_PRINTF("T513:PROFILE-FLOPPY-MATRIX:%s:%s\n", row->id,
        vm_profile_floppy_boot_terminal_name(terminal));
    result = 0;

done:
    if (thread != STD_NULL) CloseHandle(thread);
    vm_session_destroy(session);
    return result;
}
