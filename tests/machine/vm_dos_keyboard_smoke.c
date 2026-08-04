#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/machine/fdd.h"
#include "vm/platform/win32/win32.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)

static DWORD WINAPI run_machine(C_VOID *opaque)
{
    vm_session_control_start(((vm_session *)opaque)->control);
    return 0u;
}

static C_INT vm_dos_keyboard_has_text(const vm_session *session,
    const C_CHAR *text)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;
    STD_SIZE_T character;
    STD_SIZE_T length = STD_STRLEN(text);

    vm_platform_presentation_mailbox_capture(&session->presentation_mailbox, &frame);
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

    vm_platform_presentation_mailbox_capture(&session->presentation_mailbox, &frame);
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        if (STD_ISALPHA(frame.characters[cell]) &&
            frame.characters[cell + 1u] == ':' &&
            frame.characters[cell + 2u] == '\\' &&
            frame.characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_session *session = STD_NULL;
    HANDLE thread = STD_NULL;
    DWORD elapsed;
    const C_UCHAR scan_codes[] = { 0x2fu, 0x12u, 0x13u, 0x1cu };
    const C_UCHAR virtual_keys[] = { 'V', 'E', 'R', VK_RETURN };
    STD_SIZE_T index;

    if (argc != 2 || vm_session_create(STD_NULL, &session) != NTVDM64_STATUS_OK ||
        vm_machine_fdd_insert_for(&session->fdd, argv[1]) != 0) goto fail;
    thread = CreateThread(STD_NULL, 0u, run_machine, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    for (elapsed = 0u; elapsed < 3000u; elapsed += 10u) {
        if (vm_dos_keyboard_has_prompt(session)) break;
        Sleep(10u);
    }
    if (elapsed == 3000u) goto fail;
    for (index = 0u; index < sizeof(scan_codes); ++index) {
        vm_platform_win32_keyboard_make_key_for(&session->platform_run_context,
            scan_codes[index], virtual_keys[index]);
    }
    for (elapsed = 0u; elapsed < 1000u; elapsed += 10u) {
        if (vm_dos_keyboard_has_text(session, "ver")) break;
        Sleep(10u);
    }
    vm_session_stop(session);
    WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    vm_session_destroy(session);
    if (elapsed == 1000u) return 1;
    STD_PRINTF("M5:T151:S2:DOS-KEYBOARD:OK\n");
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
