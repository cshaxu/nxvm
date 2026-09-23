#include "lib/types/types_interface.h"
#include "type.h"

#include <windows.h>

#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/fault.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "test/app-nxvm/unit/core/machine/support/vm_presentation_capture.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)

static C_INT vm_dos_keyboard_submit_key(vm_machine *session,
    lib_u16 scan_code, lib_u16 virtual_key, C_INT pressed)
{
    core_machine_guest_input_event event = { 0 };

    if (session == LIB_NULL) return 0;
    event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    event.data.key.scan_code = scan_code;
    event.data.key.virtual_key = virtual_key;
    event.data.key.pressed = pressed != 0;
    return vm_machine_submit_host_input(session, &event) == TYPE_STATUS_OK;
}

static C_INT vm_dos_keyboard_submit_return(vm_machine *session)
{
    if (!vm_dos_keyboard_submit_key(session, 0x1cu, VK_RETURN, 1)) return 0;
    Sleep(25u);
    return vm_dos_keyboard_submit_key(session, 0x1cu, VK_RETURN, 0);
}

static C_INT vm_dos_keyboard_has_text(const vm_machine *session,
    const C_CHAR *text)
{
    core_machine_guest_display_frame frame;
    lib_size cell;
    lib_size character;
    lib_size length = lib_text_length(text);

    (C_VOID)test_vm_machine_capture_presentation(session, &frame);
    for (cell = 0u; cell + length <= TEXT_VIDEO_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (frame.characters[cell + character] != (C_UCHAR)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static C_INT vm_dos_keyboard_has_prompt(const vm_machine *session)
{
    core_machine_guest_display_frame frame;
    lib_size cell;

    (C_VOID)test_vm_machine_capture_presentation(session, &frame);
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        if (STD_ISALPHA(frame.characters[cell]) &&
            frame.characters[cell + 1u] == ':' &&
            frame.characters[cell + 2u] == '\\' &&
            frame.characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

static C_INT vm_dos_keyboard_has_date_prompt(const vm_machine *session)
{
    return vm_dos_keyboard_has_text(session, "Enter new date (mm-dd-yy):");
}

static C_INT vm_dos_keyboard_has_time_prompt(const vm_machine *session)
{
    return vm_dos_keyboard_has_text(session, "Enter new time:");
}

static C_INT vm_dos_keyboard_has_edit_menu(const vm_machine *session)
{
    return vm_dos_keyboard_has_text(session, "File  Edit  Search") &&
        vm_dos_keyboard_has_text(session, "Options") &&
        vm_dos_keyboard_has_text(session, "Help");
}

static C_INT vm_dos_keyboard_verify_text_frame(const vm_machine *session)
{
    core_machine_guest_display_frame frame;
    lib_u8 text[TEXT_VIDEO_CELLS * 2u];
    lib_size cell;
    lib_size title_cell = TEXT_VIDEO_CELLS;

    if (session == LIB_NULL ||
        core_machine_debug_read_memory(session->core_machine, TEXT_VIDEO_BASE,
            text, sizeof(text)) != TYPE_STATUS_OK) {
        STD_PRINTF("edit display: text memory unavailable\n");
        return 0;
    }
    (C_VOID)test_vm_machine_capture_presentation(session, &frame);
    if (frame.kind != CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT || frame.columns != 80u ||
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
            lib_memory_compare(&frame.characters[cell], "UNTITLED", 8u) == 0) {
            title_cell = cell;
        }
    }
    if (title_cell != TEXT_VIDEO_CELLS) {
        STD_PRINTF("M5:T287:S18:EDIT-ATTR title=%02x body=%02x\n",
            frame.attributes[title_cell], frame.attributes[80u]);
    }
    return 1;
}

static C_VOID vm_dos_keyboard_report_failure(const vm_machine *session,
    const core_machine_cpu_state *state)
{
    core_machine_guest_display_frame frame;
    lib_u16 head = 0u;
    lib_u16 tail = 0u;
    lib_u8 video_mode = 0u;
    lib_u8 scan_set = 0u;
    lib_u8 instructions[8] = { 0u };
    lib_size cell;
    lib_size index;

    if (session == LIB_NULL || state == LIB_NULL) return;
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
    (C_VOID)test_vm_machine_capture_presentation(session, &frame);
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
    integration_ini_session ini_session;
    vm_machine *session = LIB_NULL;
    DWORD elapsed;
    const C_UCHAR scan_codes[] = { 0x12u, 0x20u, 0x17u, 0x14u, 0x1cu };
    const C_UCHAR virtual_keys[] = { 'E', 'D', 'I', 'T', VK_RETURN };
    lib_size index;
    DWORD prompt_timeout = argc == 4 ? 20000u : 3000u;
    DWORD edit_timeout = argc == 4 ? 20000u : 5000u;
    C_INT display_ok = 0;

    if ((argc != 3 && argc != 4) || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != TYPE_STATUS_OK) return 77;
    session = ini_session.session;
    if (integration_ini_session_start(&ini_session) != TYPE_STATUS_OK) goto fail;
    for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
        if (vm_dos_keyboard_has_prompt(session) ||
            vm_dos_keyboard_has_date_prompt(session)) break;
        Sleep(10u);
    }
    if (elapsed == prompt_timeout) {
        core_machine_cpu_state state;

        if (integration_ini_session_pause(&ini_session, 500u) == TYPE_STATUS_OK &&
            core_machine_debug_read_cpu(session->core_machine, &state) ==
                TYPE_STATUS_OK) {
            STD_PRINTF("prompt pause: %04x:%08x flags=%08x\n", state.cs,
                state.eip, state.eflags);
            vm_dos_keyboard_report_failure(session, &state);
        }
        goto fail;
    }
    if (vm_dos_keyboard_has_date_prompt(session)) {
        if (!vm_dos_keyboard_submit_return(session)) goto fail;
        for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
            if (vm_dos_keyboard_has_time_prompt(session)) break;
            Sleep(10u);
        }
        if (elapsed == prompt_timeout || !vm_dos_keyboard_submit_return(session)) goto fail;
        for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
            if (vm_dos_keyboard_has_prompt(session)) break;
            Sleep(10u);
        }
        if (elapsed == prompt_timeout) {
            core_machine_cpu_state state;

            if (integration_ini_session_pause(&ini_session, 500u) == TYPE_STATUS_OK &&
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
        if (!vm_dos_keyboard_submit_key(session, scan_codes[index],
                virtual_keys[index], 1)) goto fail;
        Sleep(25u);
        if (!vm_dos_keyboard_submit_key(session, scan_codes[index],
                virtual_keys[index], 0)) goto fail;
        Sleep(25u);
    }
    for (elapsed = 0u; elapsed < edit_timeout; elapsed += 10u) {
        if (vm_dos_keyboard_has_edit_menu(session)) break;
        Sleep(10u);
    }
    if (elapsed == edit_timeout) {
        vm_machine_fault_outcome outcome;
        core_machine_cpu_state state;

        if (integration_ini_session_pause(&ini_session, 500u) == TYPE_STATUS_OK &&
            core_machine_debug_read_cpu(session->core_machine, &state) ==
                TYPE_STATUS_OK) {
            STD_PRINTF("edit pause: %04x:%08x flags=%08x\n", state.cs,
                state.eip, state.eflags);
            vm_dos_keyboard_report_failure(session, &state);
        }

        if (vm_machine_fault_get(session, &outcome) == 0 && outcome.valid) {
            STD_PRINTF("edit fault: detail=%08x pc=%08x mask=%08x code=%08x\n",
                outcome.run.detail, outcome.run.linear_pc,
                outcome.diagnostic.first_fault.exception_mask,
                outcome.diagnostic.first_fault.exception_code);
        } else {
            STD_PRINTF("edit run state: %s\n",
                common_machine_state_get(session->executor) == COMMON_MACHINE_RUNNING ?
                    "running" : "stopped");
        }
    }
    if (elapsed != edit_timeout) {
        if (integration_ini_session_pause(&ini_session, 500u) == TYPE_STATUS_OK) {
            display_ok = vm_dos_keyboard_verify_text_frame(session);
        } else {
            STD_PRINTF("edit display: pause unavailable\n");
        }
    }
    vm_machine_stop(session);
    integration_ini_session_close(&ini_session);
    if (elapsed == edit_timeout || !display_ok) return 1;
    STD_PRINTF("M5:T216:S5:EDIT:DOS:OK\n");
    return 0;

fail:
    if (session != LIB_NULL) vm_machine_stop(session);
    integration_ini_session_close(&ini_session);
    return 1;
}
