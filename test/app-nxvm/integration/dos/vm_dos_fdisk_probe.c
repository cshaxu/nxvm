#include <windows.h>
#include "lib/types/types_interface.h"

#include "type.h"

#include "test/app-nxvm/unit/core/machine/support/vm_presentation_capture.h"
#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define VM_T287_FDISK_CELLS (80u * 25u)

static DWORD WINAPI vm_t287_fdisk_run(C_VOID *opaque)
{
    vm_machine_control_start(&((vm_machine *)opaque)->control);
    return 0u;
}

static C_INT vm_t287_fdisk_has_text(const vm_machine *session, const C_CHAR *text)
{
    core_machine_guest_display_frame frame;
    lib_size cell;
    lib_size character;
    lib_size length = lib_text_length(text);

    if (session == LIB_NULL || text == LIB_NULL || length == 0u ||
        test_vm_machine_capture_presentation(session, &frame) != TYPE_STATUS_OK) return 0;
    for (cell = 0u; cell + length <= VM_T287_FDISK_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (frame.characters[cell + character] != (lib_u8)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static C_INT vm_t287_fdisk_wait(const vm_machine *session, const C_CHAR *text,
    DWORD timeout)
{
    DWORD elapsed;

    for (elapsed = 0u; elapsed < timeout; elapsed += 10u) {
        if (vm_t287_fdisk_has_text(session, text)) return 1;
        if (elapsed >= 500u && !vm_machine_control_is_running(&session->control)) {
            return 0;
        }
        Sleep(10u);
    }
    return 0;
}

static C_INT vm_t287_fdisk_submit(const vm_machine *session, const lib_u8 *codes,
    lib_size count)
{
    lib_size index;

    if (session == LIB_NULL || codes == LIB_NULL) return 0;
    for (index = 0u; index < count; ++index) {
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
                codes[index]) != TYPE_STATUS_OK) return 0;
    }
    return 1;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_ini_session ini_session;
    const lib_u8 enter[] = {0x5au};
    const lib_u8 four_make[] = {0x25u};
    const lib_u8 four_break[] = {0xf0u, 0x25u};
    const lib_u8 fdisk[] = {0x2bu, 0xf0u, 0x2bu, 0x23u, 0xf0u, 0x23u,
        0x43u, 0xf0u, 0x43u, 0x1bu, 0xf0u, 0x1bu, 0x42u, 0xf0u, 0x42u, 0x5au};
    HANDLE thread = LIB_NULL;
    vm_machine *session = LIB_NULL;
    C_INT passed = 0;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != TYPE_STATUS_OK) return 77;
    session = ini_session.session;
    if ((thread = CreateThread(LIB_NULL, 0u,
            vm_t287_fdisk_run, session, 0u, LIB_NULL)) == LIB_NULL) goto done;
    if (!vm_t287_fdisk_wait(session, "Enter new date", 60000u) ||
        !vm_t287_fdisk_submit(session, enter, sizeof(enter)) ||
        !vm_t287_fdisk_wait(session, "Enter new time", 60000u) ||
        !vm_t287_fdisk_submit(session, enter, sizeof(enter)) ||
        !vm_t287_fdisk_wait(session, "A:\\>", 60000u) ||
        !vm_t287_fdisk_submit(session, fdisk, sizeof(fdisk)) ||
        !vm_t287_fdisk_wait(session, "FDISK Options", 60000u)) goto done;
    Sleep(3000u);
    if (!vm_t287_fdisk_submit(session, four_make, sizeof(four_make))) goto done;
    Sleep(100u);
    if (!vm_t287_fdisk_submit(session, four_break, sizeof(four_break))) goto done;
    Sleep(100u);
    if (!vm_t287_fdisk_submit(session, enter, sizeof(enter))) goto done;
    if (!vm_t287_fdisk_wait(session, "Display Partition Information", 60000u)) goto done;
    Sleep(1000u);
    if (!vm_t287_fdisk_has_text(session, "Display Partition Information") ||
        !vm_t287_fdisk_has_text(session, "Press Esc to continue")) goto done;
    passed = 1;

done:
    if (session != LIB_NULL) vm_machine_stop(session);
    if (thread != LIB_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    integration_ini_session_close(&ini_session);
    if (!passed) return 1;
    STD_PRINTF("M5:T287:S21:FDISK:OPTION4:EXTERNAL:OK\n");
    return 0;
}
