#include "type.h"

#include <windows.h>

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/fault.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/machine/fdd.h"
#include "vm/platform/win32/win32.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)

static DWORD WINAPI run_machine(C_VOID *opaque)
{
    vm_session_control_start(&((vm_session *)opaque)->control);
    return 0u;
}

static C_INT vm_dos_keyboard_has_text(const vm_session *session,
    const C_CHAR *text)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;
    STD_SIZE_T character;
    STD_SIZE_T length = STD_STRLEN(text);

    (C_VOID)core_platform_presentation_mailbox_capture(
        session->presentation_mailbox, &frame);
    for (cell = 0u; cell + length <= TEXT_VIDEO_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (frame.characters[cell + character] != (C_UCHAR)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static C_INT vm_dos_keyboard_has_prompt(const vm_session *session)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;

    (C_VOID)core_platform_presentation_mailbox_capture(
        session->presentation_mailbox, &frame);
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        if (STD_ISALPHA(frame.characters[cell]) &&
            frame.characters[cell + 1u] == ':' &&
            frame.characters[cell + 2u] == '\\' &&
            frame.characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

static C_INT vm_dos_keyboard_has_date_prompt(const vm_session *session)
{
    return vm_dos_keyboard_has_text(session, "Enter new date (mm-dd-yy):");
}

static C_INT vm_dos_keyboard_has_time_prompt(const vm_session *session)
{
    return vm_dos_keyboard_has_text(session, "Enter new time:");
}

static C_INT vm_dos_keyboard_has_edit_menu(const vm_session *session)
{
    return vm_dos_keyboard_has_text(session, "File  Edit  Search") &&
        vm_dos_keyboard_has_text(session, "Options") &&
        vm_dos_keyboard_has_text(session, "Help");
}

static C_INT vm_dos_keyboard_verify_text_frame(const vm_session *session)
{
    core_platform_display_frame frame;
    type_unsigned_8 text[TEXT_VIDEO_CELLS * 2u];
    STD_SIZE_T cell;
    STD_SIZE_T title_cell = TEXT_VIDEO_CELLS;

    if (session == STD_NULL ||
        core_machine_debug_read_memory(session->core_machine, TEXT_VIDEO_BASE,
            text, sizeof(text)) != TYPE_STATUS_OK) {
        STD_PRINTF("edit display: text memory unavailable\n");
        return 0;
    }
    (C_VOID)core_platform_presentation_mailbox_capture(
        session->presentation_mailbox, &frame);
    if (frame.kind != CORE_PLATFORM_DISPLAY_KIND_TEXT || frame.columns != 80u ||
        frame.rows != 25u) {
        STD_PRINTF("edit display: kind=%u columns=%u rows=%u\n", frame.kind,
            frame.columns, frame.rows);
        return 0;
    }
    for (cell = 0u; cell < TEXT_VIDEO_CELLS; ++cell) {
        if (frame.characters[cell] != text[cell * 2u] ||
            frame.attributes[cell] != text[cell * 2u + 1u]) {
            STD_PRINTF("edit display mismatch: cell=%u raw=%02x/%02x "
                "frame=%02x/%02x\n", (unsigned)cell, text[cell * 2u],
                text[cell * 2u + 1u], frame.characters[cell],
                frame.attributes[cell]);
            return 0;
        }
        if (cell + 8u <= TEXT_VIDEO_CELLS &&
            STD_MEMCMP(&frame.characters[cell], "UNTITLED", 8u) == 0) {
            title_cell = cell;
        }
    }
    if (title_cell != TEXT_VIDEO_CELLS) {
        STD_PRINTF("M5:T287:S18:EDIT-ATTR title=%02x body=%02x\n",
            frame.attributes[title_cell], frame.attributes[80u]);
    }
    return 1;
}

static C_VOID vm_dos_keyboard_report_failure(const vm_session *session,
    const core_machine_cpu_state *state)
{
    core_platform_display_frame frame;
    type_unsigned_16 head = 0u;
    type_unsigned_16 tail = 0u;
    type_unsigned_8 video_mode = 0u;
    type_unsigned_8 scan_set = 0u;
    type_unsigned_8 instructions[8] = { 0u };
    STD_SIZE_T cell;
    STD_SIZE_T index;

    if (session == STD_NULL || state == STD_NULL) return;
    (C_VOID)core_machine_debug_read_memory(session->core_machine, 0x041au,
        &head, sizeof(head));
    (C_VOID)core_machine_debug_read_memory(session->core_machine, 0x041cu,
        &tail, sizeof(tail));
    (C_VOID)core_machine_debug_read_memory(session->core_machine, 0x0449u,
        &video_mode, sizeof(video_mode));
    (C_VOID)core_machine_debug_read_memory(session->core_machine,
        state->cs_base + state->eip, instructions, sizeof(instructions));
    (C_VOID)core_machine_keyboard_get_native_scan_set(session->core_machine,
        &scan_set);
    (C_VOID)core_platform_presentation_mailbox_capture(
        session->presentation_mailbox, &frame);
    STD_PRINTF("keyboard smoke timed out: BDA head=%04x tail=%04x\n", head, tail);
    for (cell = 0u; cell < 25u; ++cell) {
        for (index = 0u; index < 80u; ++index) {
            C_UCHAR character = frame.characters[cell * 80u + index];
            STD_PRINTF("%c", character == 0u ? ' ' : character);
        }
        STD_PRINTF("\n");
    }
    STD_PRINTF("edit state: mode=%02x BDA head=%04x tail=%04x halt=%u bytes="
        "%02x %02x %02x %02x %02x %02x %02x %02x\n", video_mode, head, tail,
        state->halted, instructions[0], instructions[1], instructions[2],
        instructions[3], instructions[4], instructions[5], instructions[6],
        instructions[7]);
    STD_PRINTF("keyboard controller: scan_set=%u\n", scan_set);
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_session *session = STD_NULL;
    HANDLE thread = STD_NULL;
    DWORD elapsed;
    const C_UCHAR scan_codes[] = { 0x12u, 0x20u, 0x17u, 0x14u, 0x1cu };
    const C_UCHAR virtual_keys[] = { 'E', 'D', 'I', 'T', VK_RETURN };
    STD_SIZE_T index;
    DWORD prompt_timeout = argc == 3 ? 20000u : 3000u;
    DWORD edit_timeout = argc == 3 ? 20000u : 5000u;
    C_INT display_ok = 0;

    if ((argc != 2 && argc != 3) ||
        vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_machine_fdd_insert_for(&session->fdd, argv[1]) != 0) goto fail;
    thread = CreateThread(STD_NULL, 0u, run_machine, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
        if (vm_dos_keyboard_has_prompt(session) ||
            vm_dos_keyboard_has_date_prompt(session)) break;
        Sleep(10u);
    }
    if (elapsed == prompt_timeout) {
        core_machine_cpu_state state;

        vm_session_control_request_pause(&session->control,
            VM_SESSION_PAUSE_EXPLICIT);
        if (vm_session_control_wait_for_pause(&session->control, 500u) &&
            core_machine_debug_read_cpu(session->core_machine, &state) ==
                TYPE_STATUS_OK) {
            STD_PRINTF("prompt pause: %04x:%08x flags=%08x\n", state.cs,
                state.eip, state.eflags);
            vm_dos_keyboard_report_failure(session, &state);
        }
        goto fail;
    }
    if (vm_dos_keyboard_has_date_prompt(session)) {
        if (core_machine_keyboard_receive_native_byte(session->core_machine, 0x5au) !=
            TYPE_STATUS_OK) goto fail;
        for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
            if (vm_dos_keyboard_has_time_prompt(session)) break;
            Sleep(10u);
        }
        if (elapsed == prompt_timeout ||
            core_machine_keyboard_receive_native_byte(session->core_machine, 0x5au) !=
                TYPE_STATUS_OK) goto fail;
        for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
            if (vm_dos_keyboard_has_prompt(session)) break;
            Sleep(10u);
        }
        if (elapsed == prompt_timeout) {
            core_machine_cpu_state state;

            vm_session_control_request_pause(&session->control,
                VM_SESSION_PAUSE_EXPLICIT);
            if (vm_session_control_wait_for_pause(&session->control, 500u) &&
                core_machine_debug_read_cpu(session->core_machine, &state) ==
                    TYPE_STATUS_OK) {
                STD_PRINTF("date input pause: %04x:%08x flags=%08x\n", state.cs,
                    state.eip, state.eflags);
                vm_dos_keyboard_report_failure(session, &state);
            }
            goto fail;
        }
    }
    for (index = 0u; index < sizeof(scan_codes); ++index) {
        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            session->platform_run_handle, scan_codes[index], virtual_keys[index], 1);
        Sleep(25u);
        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            session->platform_run_handle, scan_codes[index], virtual_keys[index], 0);
        Sleep(25u);
    }
    for (elapsed = 0u; elapsed < edit_timeout; elapsed += 10u) {
        if (vm_dos_keyboard_has_edit_menu(session)) break;
        Sleep(10u);
    }
    if (elapsed == edit_timeout) {
        vm_session_fault_outcome outcome;
        core_machine_cpu_state state;

        vm_session_control_request_pause(&session->control,
            VM_SESSION_PAUSE_EXPLICIT);
        if (vm_session_control_wait_for_pause(&session->control, 500u) &&
            core_machine_debug_read_cpu(session->core_machine, &state) ==
                TYPE_STATUS_OK) {
            STD_PRINTF("edit pause: %04x:%08x flags=%08x\n", state.cs,
                state.eip, state.eflags);
            vm_dos_keyboard_report_failure(session, &state);
        }

        if (vm_session_fault_get(session, &outcome) == 0 && outcome.valid) {
            STD_PRINTF("edit fault: detail=%08x pc=%08x mask=%08x code=%08x\n",
                outcome.run.detail, outcome.run.linear_pc,
                outcome.diagnostic.first_fault.exception_mask,
                outcome.diagnostic.first_fault.exception_code);
        } else {
            STD_PRINTF("edit run state: %s\n",
                vm_session_control_is_running(&session->control) ? "running" : "stopped");
        }
    }
    if (elapsed != edit_timeout) {
        vm_session_control_request_pause(&session->control,
            VM_SESSION_PAUSE_EXPLICIT);
        if (vm_session_control_wait_for_pause(&session->control, 500u)) {
            display_ok = vm_dos_keyboard_verify_text_frame(session);
        } else {
            STD_PRINTF("edit display: pause unavailable\n");
        }
    }
    vm_session_stop(session);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_session_destroy(session);
    if (elapsed == edit_timeout || !display_ok) return 1;
    STD_PRINTF("M5:T216:S5:EDIT:DOS:OK\n");
    return 0;

fail:
    if (session != STD_NULL) vm_session_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    vm_session_destroy(session);
    return 1;
}
