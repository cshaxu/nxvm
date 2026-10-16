#include "../../../x86-nxvm/machine/support/guest_input.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <ctype.h>
#include <stdio.h>

#include <windows.h>
#undef exception_code

#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/fault.h"
#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "test/x86-nxvm/machine/support/nxvm_presentation_capture.h"
#include "test/x86-nxvm/setup/session_ini.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)

static lib_i32 vm_dos_keyboard_submit_key(x86_nxvm_machine *session,
    lib_u16 scan_code, lib_u16 virtual_key, lib_i32 pressed)
{
    core_machine_guest_input_event event = { 0 };
    lib_status status;

    if (session == LIB_NULL) return 0;
    event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    event.data.key.scan_code = scan_code;
    event.data.key.virtual_key = virtual_key;
    event.data.key.pressed = pressed != 0;
    status = vm_test_submit_host_input(session, &event);
    if (status != LIB_STATUS_OK) {
        printf("keyboard input rejected: scan=%04x key=%04x pressed=%u status=%u\n",
            scan_code, virtual_key, (unsigned)(pressed != 0), (unsigned)status);
    }
    return status == LIB_STATUS_OK;
}

static lib_i32 vm_dos_keyboard_submit_return(x86_nxvm_machine *session)
{
    if (!vm_dos_keyboard_submit_key(session, 0x1cu, VK_RETURN, 1)) return 0;
    Sleep(25u);
    return vm_dos_keyboard_submit_key(session, 0x1cu, VK_RETURN, 0);
}

static lib_i32 vm_dos_keyboard_has_text(const x86_nxvm_machine *session,
    const char *text)
{
    core_machine_guest_display_frame frame;
    lib_size cell;
    lib_size character;
    lib_size length = lib_text_length(text);

    if (test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK) return 0;
    for (cell = 0u; cell + length <= TEXT_VIDEO_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (frame.characters[cell + character] != (lib_u8)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static lib_i32 vm_dos_keyboard_has_prompt(const x86_nxvm_machine *session)
{
    core_machine_guest_display_frame frame;
    lib_size cell;

    if (test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK) return 0;
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        if (isalpha(frame.characters[cell]) &&
            frame.characters[cell + 1u] == ':' &&
            frame.characters[cell + 2u] == '\\' &&
            frame.characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

static lib_i32 vm_dos_keyboard_has_date_prompt(const x86_nxvm_machine *session)
{
    return vm_dos_keyboard_has_text(session, "Enter new date (mm-dd-yy):");
}

static lib_i32 vm_dos_keyboard_has_time_prompt(const x86_nxvm_machine *session)
{
    return vm_dos_keyboard_has_text(session, "Enter new time:");
}

static lib_i32 vm_dos_keyboard_has_edit_menu(const x86_nxvm_machine *session)
{
    return vm_dos_keyboard_has_text(session, "File  Edit  Search") &&
        vm_dos_keyboard_has_text(session, "Options") &&
        vm_dos_keyboard_has_text(session, "Help");
}

static lib_i32 vm_dos_keyboard_verify_text_frame(x86_nxvm_machine *session)
{
    x86_video_snapshot frame;
    lib_u8 text[TEXT_VIDEO_CELLS * 2u];
    lib_size cell;
    lib_size title_cell = TEXT_VIDEO_CELLS;

    if (session == LIB_NULL ||
        x86_core_machine_debug_read_memory(session->x86_core_machine, TEXT_VIDEO_BASE,
            text, sizeof(text)) != LIB_STATUS_OK) {
        printf("edit display: text memory unavailable\n");
        return 0;
    }
    /* Paused inspection compares frozen Core values. The published UI frame
     * may precede pause by one cadence; observers cannot enter its producer. */
    if (x86_nxvm_board_capture_display_snapshot(session->board, &frame) != LIB_STATUS_OK) {
        printf("edit display: paused frame unavailable\n");
        return 0;
    }
    if (frame.kind != X86_VIDEO_KIND_TEXT || frame.columns != 80u ||
        frame.rows != 25u) {
        printf("edit display: kind=%u columns=%u rows=%u\n", frame.kind,
            frame.columns, frame.rows);
        return 0;
    }
    for (cell = 0u; cell < TEXT_VIDEO_CELLS; ++cell) {
        if (frame.characters[cell] != text[cell * 2u] ||
            frame.attributes[cell] != text[cell * 2u + 1u]) {
            printf("edit display mismatch: cell=%u raw=%02x/%02x "
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
        printf("NXVM:EDIT-ATTR title=%02x body=%02x\n",
            frame.attributes[title_cell], frame.attributes[80u]);
    }
    return 1;
}

static void vm_dos_keyboard_report_failure(const x86_nxvm_machine *session,
    const x86_core_cpu_state *state)
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
    (void)x86_core_machine_debug_read_memory(session->x86_core_machine, 0x041au,
        &head, sizeof(head));
    (void)x86_core_machine_debug_read_memory(session->x86_core_machine, 0x041cu,
        &tail, sizeof(tail));
    (void)x86_core_machine_debug_read_memory(session->x86_core_machine, 0x0449u,
        &video_mode, sizeof(video_mode));
    (void)x86_core_machine_debug_read_memory(session->x86_core_machine,
        state->cs_base + state->eip, instructions, sizeof(instructions));
    (void)x86_nxvm_board_keyboard_get_native_scan_set(session->board,
        &scan_set);
    printf("keyboard smoke timed out: BDA head=%04x tail=%04x\n", head, tail);
    if (test_nxvm_machine_capture_presentation(session, &frame) == LIB_STATUS_OK) {
        for (cell = 0u; cell < 25u; ++cell) {
            for (index = 0u; index < 80u; ++index) {
                lib_u8 character = frame.characters[cell * 80u + index];
                printf("%c", character == 0u ? ' ' : character);
            }
            printf("\n");
        }
    } else {
        printf("keyboard display unavailable\n");
    }
    printf("edit state: mode=%02x BDA head=%04x tail=%04x halt=%u bytes="
        "%02x %02x %02x %02x %02x %02x %02x %02x\n", video_mode, head, tail,
        state->halted, instructions[0], instructions[1], instructions[2],
        instructions[3], instructions[4], instructions[5], instructions[6],
        instructions[7]);
    printf("keyboard controller: scan_set=%u\n", scan_set);
}

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;
    x86_nxvm_machine *session = LIB_NULL;
    DWORD elapsed;
    const lib_u8 scan_codes[] = { 0x12u, 0x20u, 0x17u, 0x14u, 0x1cu };
    const lib_u8 virtual_keys[] = { 'E', 'D', 'I', 'T', VK_RETURN };
    lib_size index;
    DWORD prompt_timeout = argc == 4 ? 20000u : 3000u;
    DWORD edit_timeout = argc == 4 ? 20000u : 5000u;
    lib_i32 display_ok = 0;
    const char *stage = "start";

    if ((argc != 3 && argc != 4) || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    if (integration_ini_session_start(&ini_session) != LIB_STATUS_OK) goto fail;
    stage = "prompt";
    for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
        if (vm_dos_keyboard_has_prompt(session) ||
            vm_dos_keyboard_has_date_prompt(session)) break;
        Sleep(10u);
    }
    if (elapsed == prompt_timeout) {
        x86_core_cpu_state state;

        if (integration_ini_session_pause(&ini_session, 500u) == LIB_STATUS_OK &&
            x86_core_machine_debug_read_cpu(session->x86_core_machine, &state) ==
                LIB_STATUS_OK) {
            printf("prompt pause: %04x:%08x flags=%08x\n", state.cs,
                state.eip, state.eflags);
            vm_dos_keyboard_report_failure(session, &state);
        }
        goto fail;
    }
    if (vm_dos_keyboard_has_date_prompt(session)) {
        stage = "date-input";
        if (!vm_dos_keyboard_submit_return(session)) goto fail;
        for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
            if (vm_dos_keyboard_has_time_prompt(session)) break;
            Sleep(10u);
        }
        stage = "time-input";
        if (elapsed == prompt_timeout || !vm_dos_keyboard_submit_return(session)) goto fail;
        for (elapsed = 0u; elapsed < prompt_timeout; elapsed += 10u) {
            if (vm_dos_keyboard_has_prompt(session)) break;
            Sleep(10u);
        }
        if (elapsed == prompt_timeout) {
            x86_core_cpu_state state;

            if (integration_ini_session_pause(&ini_session, 500u) == LIB_STATUS_OK &&
                x86_core_machine_debug_read_cpu(session->x86_core_machine, &state) ==
                    LIB_STATUS_OK) {
                printf("date input pause: %04x:%08x flags=%08x\n", state.cs,
                    state.eip, state.eflags);
                vm_dos_keyboard_report_failure(session, &state);
            }
            goto fail;
        }
    }
    stage = "command-input";
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
        x86_nxvm_machine_fault_outcome outcome;
        x86_core_cpu_state state;

        if (integration_ini_session_pause(&ini_session, 500u) == LIB_STATUS_OK &&
            x86_core_machine_debug_read_cpu(session->x86_core_machine, &state) ==
                LIB_STATUS_OK) {
            printf("edit pause: %04x:%08x flags=%08x\n", state.cs,
                state.eip, state.eflags);
            vm_dos_keyboard_report_failure(session, &state);
        }

        if (x86_nxvm_machine_fault_get(session, &outcome) == 0 && outcome.valid) {
            printf("edit fault: detail=%08x pc=%08x mask=%08x code=%08x\n",
                outcome.run.detail, outcome.run.linear_pc,
                outcome.diagnostic.first_fault.exception_mask,
                outcome.diagnostic.first_fault.exception_code);
        } else {
            printf("edit run state: %s\n",
                emulator_machine_state_get(session->executor) == EMULATOR_MACHINE_RUNNING ?
                    "running" : "stopped");
        }
    }
    if (elapsed != edit_timeout) {
        if (integration_ini_session_pause(&ini_session, 500u) == LIB_STATUS_OK) {
            display_ok = vm_dos_keyboard_verify_text_frame(session);
        } else {
            printf("edit display: pause unavailable\n");
        }
    }
    x86_nxvm_machine_stop(session);
    integration_ini_session_close(&ini_session);
    if (elapsed == edit_timeout || !display_ok) return 1;
    printf("NXVM:EDIT:DOS:OK\n");
    return 0;

fail:
    printf("keyboard smoke failed: stage=%s active=%u state=%u\n", stage,
        session == LIB_NULL ? 0u : (unsigned)session->active,
        ini_session.emulator_machine == LIB_NULL ? 0u :
        (unsigned)emulator_machine_state_get(ini_session.emulator_machine));
    if (session != LIB_NULL) x86_nxvm_machine_stop(session);
    integration_ini_session_close(&ini_session);
    return 1;
}
