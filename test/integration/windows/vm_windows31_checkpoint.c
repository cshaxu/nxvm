#include "type.h"

#include <windows.h>

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "core/platform/presentation_mailbox_interface.h"
#include "test/integration/support/session_yaml.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/win32/win32.h"

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

static C_INT vm_t287_wait_for_hdc_command(const vm_session *session,
    DWORD timeout)
{
    DWORD elapsed;

    if (session == STD_NULL || session->core_machine == STD_NULL) return 0;
    for (elapsed = 0u; elapsed < timeout; elapsed += 10u) {
        if (session->core_machine->hdc.data.command_count != 0u) return 1;
        if (elapsed >= 500u && !vm_session_control_is_running(&session->control)) {
            return 0;
        }
        Sleep(10u);
    }
    return 0;
}

static C_VOID vm_t287_submit_key(const vm_session *session, type_unsigned_16 scan_code,
    type_unsigned_16 virtual_key)
{
    if (session == STD_NULL) return;
    vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
        session->platform_run_handle, scan_code, virtual_key, 1);
    Sleep(25u);
    vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
        session->platform_run_handle, scan_code, virtual_key, 0);
    Sleep(25u);
}

static C_VOID vm_t287_submit_colon(const vm_session *session)
{
    if (session == STD_NULL) return;
    vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
        session->platform_run_handle, 0x2au, VK_SHIFT, 1);
    Sleep(25u);
    vm_t287_submit_key(session, 0x27u, VK_OEM_1);
    vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
        session->platform_run_handle, 0x2au, VK_SHIFT, 0);
    Sleep(25u);
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
    integration_yaml_session yaml_session = {0};
    HANDLE thread = STD_NULL;
    vm_session *session = STD_NULL;
    type_unsigned_8 hdd_count = 0u;
    type_unsigned_8 hdd_bda[4] = {0};
    C_UINT ata_commands = 0u;
    C_INT c_present;
    const C_CHAR *drive_result;
    const C_CHAR *boot_text;
    const C_CHAR *stage = "create";

    if (argc != 3 || integration_yaml_session_open(argv[1], argv[2],
            &yaml_session) != TYPE_STATUS_OK) return 77;
    session = yaml_session.session;
    if (session == STD_NULL) goto fail;
    thread = CreateThread(STD_NULL, 0u, vm_t287_run_machine, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    stage = "date";
    boot_text = vm_t287_wait_for_text(session, "Enter new date", "A:\\>",
        VM_T287_BOOT_TIMEOUT_MILLISECONDS);
    if (boot_text == STD_NULL) goto fail;
    if (STD_STRCMP(boot_text, "A:\\>") != 0) {
        vm_t287_submit_key(session, 0x1cu, VK_RETURN);
    }
    stage = "time";
    if (STD_STRCMP(boot_text, "A:\\>") != 0) {
        boot_text = vm_t287_wait_for_text(session, "Enter new time", "A:\\>",
            VM_T287_BOOT_TIMEOUT_MILLISECONDS);
        if (boot_text == STD_NULL) goto fail;
        if (STD_STRCMP(boot_text, "A:\\>") != 0) {
            vm_t287_submit_key(session, 0x1cu, VK_RETURN);
        }
    }
    stage = "prompt";
    if (vm_t287_wait_for_text(session, "A:\\>", STD_NULL,
            VM_T287_BOOT_TIMEOUT_MILLISECONDS) == STD_NULL) {
        goto fail;
    }
    stage = "c-command";
    vm_t287_submit_key(session, 0x2eu, 'C');
    vm_t287_submit_colon(session);
    vm_t287_submit_key(session, 0x1cu, VK_RETURN);
    stage = "c-drive";
    drive_result = vm_t287_wait_for_text(session, "C:\\>",
        "Invalid drive specification", VM_T287_COMMAND_TIMEOUT_MILLISECONDS);
    c_present = drive_result != STD_NULL && !STD_STRCMP(drive_result, "C:\\>");
    if (!c_present) goto fail;
    stage = "c-dir";
    vm_t287_submit_key(session, 0x20u, 'D');
    vm_t287_submit_key(session, 0x17u, 'I');
    vm_t287_submit_key(session, 0x13u, 'R');
    vm_t287_submit_key(session, 0x1cu, VK_RETURN);
    if (!vm_t287_wait_for_hdc_command(session,
            VM_T287_COMMAND_TIMEOUT_MILLISECONDS)) goto fail;
    stage = "bda-hdd-count";
    if (core_machine_debug_read_memory(session->core_machine, 0x0474u, hdd_bda,
            sizeof(hdd_bda)) == TYPE_STATUS_OK) hdd_count = hdd_bda[1];
    ata_commands = session->core_machine->hdc.data.command_count;
    vm_session_control_stop(&session->control);
    WaitForSingleObject(thread, 2000u);
    vm_t287_report_frame(session);
    CloseHandle(thread);
    thread = STD_NULL;
    if (c_present && ata_commands != 0u) {
        STD_PRINTF("M5:T287:S2:WINDOWS31:CHECKPOINT:OK result=c-drive-present "
            "observed_bda_hdd_count=%u ata_commands=%u\n", hdd_count,
            ata_commands);
        integration_yaml_session_close(&yaml_session);
        return 0;
    }

fail:
    vm_t287_report(session, stage);
    if (session != STD_NULL) vm_session_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    integration_yaml_session_close(&yaml_session);
    return 1;
}
