#include <windows.h>

#include "type.h"

#include "core/platform/presentation_mailbox_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_T287_FDISK_CELLS (80u * 25u)

static DWORD WINAPI vm_t287_fdisk_run(C_VOID *opaque)
{
    vm_session_control_start(&((vm_session *)opaque)->control);
    return 0u;
}

static C_INT vm_t287_fdisk_has_text(const vm_session *session, const C_CHAR *text)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;
    STD_SIZE_T character;
    STD_SIZE_T length = STD_STRLEN(text);

    if (session == STD_NULL || text == STD_NULL || length == 0u ||
        core_platform_presentation_mailbox_capture(session->presentation_mailbox,
            &frame) != TYPE_STATUS_OK) return 0;
    for (cell = 0u; cell + length <= VM_T287_FDISK_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (frame.characters[cell + character] != (type_unsigned_8)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static C_INT vm_t287_fdisk_wait(const vm_session *session, const C_CHAR *text,
    DWORD timeout)
{
    DWORD elapsed;

    for (elapsed = 0u; elapsed < timeout; elapsed += 10u) {
        if (vm_t287_fdisk_has_text(session, text)) return 1;
        if (elapsed >= 500u && !vm_session_control_is_running(&session->control)) {
            return 0;
        }
        Sleep(10u);
    }
    return 0;
}

static C_INT vm_t287_fdisk_submit(const vm_session *session, const type_unsigned_8 *codes,
    STD_SIZE_T count)
{
    STD_SIZE_T index;

    if (session == STD_NULL || codes == STD_NULL) return 0;
    for (index = 0u; index < count; ++index) {
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
                codes[index]) != TYPE_STATUS_OK) return 0;
    }
    return 1;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    const vm_session_config config = {
        .floppy_image = { argc == 3 ? argv[1] : STD_NULL },
        .fixed_disk_image = { argc == 3 ? argv[2] : STD_NULL },
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const type_unsigned_8 enter[] = {0x5au};
    const type_unsigned_8 four_make[] = {0x25u};
    const type_unsigned_8 four_break[] = {0xf0u, 0x25u};
    const type_unsigned_8 fdisk[] = {0x2bu, 0xf0u, 0x2bu, 0x23u, 0xf0u, 0x23u,
        0x43u, 0xf0u, 0x43u, 0x1bu, 0xf0u, 0x1bu, 0x42u, 0xf0u, 0x42u, 0x5au};
    HANDLE thread = STD_NULL;
    vm_session *session = STD_NULL;
    C_INT passed = 0;

    if (argc != 3 || vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || (thread = CreateThread(STD_NULL, 0u,
            vm_t287_fdisk_run, session, 0u, STD_NULL)) == STD_NULL) goto done;
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
    if (session != STD_NULL) vm_session_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    vm_session_destroy(session);
    if (!passed) return 1;
    STD_PRINTF("M5:T287:S21:FDISK:OPTION4:EXTERNAL:OK\n");
    return 0;
}
