#include "type.h"

#include <windows.h>

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "core/platform/presentation_mailbox_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_T287_TEXT_CELLS (80u * 25u)
#define VM_T287_BOOT_TIMEOUT_MILLISECONDS 60000u
#define VM_T287_COMMAND_TIMEOUT_MILLISECONDS 5000u

static DWORD WINAPI vm_t287_run_machine(C_VOID *opaque)
{
    vm_session_control_start(&((vm_session *)opaque)->control);
    return 0u;
}

static C_INT vm_t287_has_text(const vm_session *session, const C_CHAR *text)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;
    STD_SIZE_T character;
    STD_SIZE_T length = STD_STRLEN(text);

    if (session == STD_NULL || text == STD_NULL || length == 0u ||
        core_platform_presentation_mailbox_capture(session->presentation_mailbox,
            &frame) != TYPE_STATUS_OK) return 0;
    for (cell = 0u; cell + length <= VM_T287_TEXT_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (frame.characters[cell + character] != (C_UCHAR)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static const C_CHAR *vm_t287_wait_for_text(const vm_session *session,
    const C_CHAR *first, const C_CHAR *second, DWORD timeout)
{
    DWORD elapsed;

    for (elapsed = 0u; elapsed < timeout; elapsed += 10u) {
        if (vm_t287_has_text(session, first)) return first;
        if (second != STD_NULL && vm_t287_has_text(session, second)) return second;
        if (elapsed >= 500u && !vm_session_control_is_running(&session->control)) {
            return STD_NULL;
        }
        Sleep(10u);
    }
    return STD_NULL;
}

static C_INT vm_t287_submit(const vm_session *session, const type_unsigned_8 *codes,
    STD_SIZE_T count)
{
    STD_SIZE_T index;

    if (session == STD_NULL || codes == STD_NULL) return 0;
    for (index = 0u; index < count; ++index) {
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
                codes[index]) != TYPE_STATUS_OK) return 0;
        Sleep(25u);
    }
    return 1;
}

static C_VOID vm_t287_report(const vm_session *session, const C_CHAR *stage)
{
    core_platform_display_frame frame;
    core_machine_cpu_diagnostic diagnostic = {0};
    STD_SIZE_T row;
    STD_SIZE_T column;

    if (session == STD_NULL) return;
    STD_PRINTF("M5:T287:S2:WINDOWS31:CHECKPOINT:FAIL stage=%s running=%d "
        "ata_commands=%u last_command=%02X\n", stage,
        vm_session_control_is_running(&session->control),
        session->core_machine->hdc.data.command_count,
        session->core_machine->hdc.data.last_command);
    if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
            TYPE_STATUS_OK && diagnostic.first_fault.valid) {
        STD_SIZE_T index;

        STD_PRINTF("M5:T287:S17:FAULT cs=%04X ip=%08X opcode=%02X%02X%02X "
            "eax=%08X ebx=%08X ecx=%08X edx=%08X esi=%08X edi=%08X\n",
            diagnostic.first_fault.point.cs, diagnostic.first_fault.point.eip,
            diagnostic.first_fault.point.bytes[0], diagnostic.first_fault.point.bytes[1],
            diagnostic.first_fault.point.bytes[2], diagnostic.first_fault.eax,
            diagnostic.first_fault.ebx, diagnostic.first_fault.ecx,
            diagnostic.first_fault.edx, diagnostic.first_fault.esi,
            diagnostic.first_fault.edi);
        for (index = 0u; index < diagnostic.recent_count; ++index) {
            STD_PRINTF("M5:T287:S17:RECENT cs=%04X ip=%08X opcode=%02X%02X%02X\n",
                diagnostic.recent[index].cs, diagnostic.recent[index].eip,
                diagnostic.recent[index].bytes[0], diagnostic.recent[index].bytes[1],
                diagnostic.recent[index].bytes[2]);
        }
    }
    if (core_platform_presentation_mailbox_capture(session->presentation_mailbox,
            &frame) != TYPE_STATUS_OK) return;
    for (row = 0u; row < 25u; ++row) {
        for (column = 0u; column < 80u; ++column) {
            C_UCHAR character = frame.characters[row * 80u + column];
            STD_PRINTF("%c", character == 0u ? ' ' : character);
        }
        STD_PRINTF("\n");
    }
}

static C_VOID vm_t287_report_frame(const vm_session *session)
{
    core_platform_display_frame frame;
    STD_SIZE_T row;
    STD_SIZE_T column;

    if (session == STD_NULL || core_platform_presentation_mailbox_capture(
            session->presentation_mailbox, &frame) != TYPE_STATUS_OK) return;
    for (row = 0u; row < 25u; ++row) {
        for (column = 0u; column < 80u; ++column) {
            C_UCHAR character = frame.characters[row * 80u + column];
            STD_PRINTF("%c", character == 0u ? ' ' : character);
        }
        STD_PRINTF("\n");
    }
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    const vm_session_config config = {
        .fdd_image = argc == 3 ? argv[1] : STD_NULL,
        .hdd_image = argc == 3 ? argv[2] : STD_NULL,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const type_unsigned_8 enter[] = {0x5au};
    const type_unsigned_8 select_c[] = {0x21u, 0x12u, 0x4cu, 0xf0u, 0x12u, 0x5au};
    HANDLE thread = STD_NULL;
    vm_session *session = STD_NULL;
    type_unsigned_8 hdd_count = 0u;
    type_unsigned_8 hdd_bda[4] = {0};
    C_UINT ata_commands = 0u;
    type_unsigned_8 last_command = 0u;
    C_INT c_present;
    C_INT c_absent;
    const C_CHAR *boot_text;
    const C_CHAR *stage = "create";

    if (argc != 3 || vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto fail;
    thread = CreateThread(STD_NULL, 0u, vm_t287_run_machine, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    stage = "date";
    boot_text = vm_t287_wait_for_text(session, "Enter new date", "A:\\>",
        VM_T287_BOOT_TIMEOUT_MILLISECONDS);
    if (boot_text == STD_NULL) goto fail;
    if (STD_STRCMP(boot_text, "A:\\>") != 0 &&
        !vm_t287_submit(session, enter, sizeof(enter))) goto fail;
    stage = "time";
    if (STD_STRCMP(boot_text, "A:\\>") != 0) {
        boot_text = vm_t287_wait_for_text(session, "Enter new time", "A:\\>",
            VM_T287_BOOT_TIMEOUT_MILLISECONDS);
        if (boot_text == STD_NULL) goto fail;
        if (STD_STRCMP(boot_text, "A:\\>") != 0 &&
            !vm_t287_submit(session, enter, sizeof(enter))) goto fail;
    }
    stage = "prompt";
    if (vm_t287_wait_for_text(session, "A:\\>", STD_NULL,
            VM_T287_BOOT_TIMEOUT_MILLISECONDS) == STD_NULL) {
        goto fail;
    }
    stage = "bda-hdd-count";
    vm_session_control_request_pause(&session->control, VM_SESSION_PAUSE_EXPLICIT);
    if (!vm_session_control_wait_for_pause(&session->control, 2000u) ||
        core_machine_debug_read_memory(session->core_machine, 0x0474u, hdd_bda,
            sizeof(hdd_bda)) != TYPE_STATUS_OK) goto fail;
    hdd_count = hdd_bda[1];
    vm_session_control_continue(&session->control);
    stage = "c-command";
    if (!vm_t287_submit(session, select_c, sizeof(select_c))) goto fail;
    stage = "c-drive";
    c_present = vm_t287_wait_for_text(session, "C:\\>", STD_NULL,
        VM_T287_COMMAND_TIMEOUT_MILLISECONDS) != STD_NULL;
    c_absent = vm_t287_wait_for_text(session, "Invalid drive specification", STD_NULL,
        VM_T287_COMMAND_TIMEOUT_MILLISECONDS) != STD_NULL;
    vm_t287_report_frame(session);
    ata_commands = session->core_machine->hdc.data.command_count;
    last_command = session->core_machine->hdc.data.last_command;
    vm_session_stop(session);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    if (c_present) {
        STD_PRINTF("M5:T287:S2:WINDOWS31:CHECKPOINT:OK result=c-drive-present "
            "bda_hdd_count=%u ata_commands=%u\n", hdd_count,
            ata_commands);
        vm_session_destroy(session);
        return 0;
    }
    if (c_absent) {
        STD_PRINTF("M5:T287:S2:WINDOWS31:CHECKPOINT:OK result=c-drive-absent "
            "category=bios-firmware bda_hdd=%02X/%02X/%02X/%02X ata_commands=%u last_command=%02X\n",
            hdd_bda[0], hdd_count, hdd_bda[2], hdd_bda[3], ata_commands,
            last_command);
        vm_session_destroy(session);
        return 0;
    }

fail:
    vm_t287_report(session, stage);
    if (session != STD_NULL) vm_session_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    vm_session_destroy(session);
    return 1;
}
