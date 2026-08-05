#include "type.h"

#include <windows.h>

#include "core/platform/display_frame.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "tests/support/vm_session_fixture.h"
#include "vm/platform/presentation_mailbox.h"

#define VM_RUNNER_DISPLAY_CADENCE_TIMEOUT_MILLISECONDS 1000u

static DWORD WINAPI vm_runner_display_cadence_run(C_VOID *opaque)
{
    vm_session_control_start(vm_session_fixture_control((vm_session *)opaque));
    return 0u;
}

static C_INT vm_runner_display_cadence_contains(const core_platform_display_frame *frame,
    const C_CHAR *text)
{
    STD_SIZE_T cell;
    STD_SIZE_T index;

    if (frame == STD_NULL || text == STD_NULL) return 0;
    for (cell = 0u; cell < CORE_PLATFORM_DISPLAY_MAX_COLUMNS *
            CORE_PLATFORM_DISPLAY_MAX_ROWS; ++cell) {
        for (index = 0u; text[index] != '\0' && cell + index <
                CORE_PLATFORM_DISPLAY_MAX_COLUMNS * CORE_PLATFORM_DISPLAY_MAX_ROWS;
             ++index) {
            if (frame->characters[cell + index] != (uint8_t)text[index]) break;
        }
        if (text[index] == '\0') return 1;
    }
    return 0;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    core_platform_display_frame frame;
    HANDLE thread = STD_NULL;
    DWORD elapsed;
    C_INT seen = 0;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    if (!vm_session_fixture_is_active(session)) return 1;
    vm_session_control_reset(vm_session_fixture_control(session));
    thread = CreateThread(STD_NULL, 0u, vm_runner_display_cadence_run, session,
        0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    for (elapsed = 0u; elapsed < VM_RUNNER_DISPLAY_CADENCE_TIMEOUT_MILLISECONDS;
         ++elapsed) {
        vm_platform_presentation_mailbox_capture(vm_session_fixture_presentation_mailbox(session), &frame);
        if (vm_runner_display_cadence_contains(&frame, "Invalid boot disk")) {
            seen = 1;
            break;
        }
        Sleep(1u);
    }
    vm_session_control_stop(vm_session_fixture_control(session));
    if (WaitForSingleObject(thread, 2000u) != WAIT_OBJECT_0) goto fail;
    CloseHandle(thread);
    vm_session_destroy(session);
    if (!seen) return 1;
    puts("M5:T212:S2:RUNNER-CADENCE:OK");
    return 0;

fail:
    vm_session_control_stop(vm_session_fixture_control(session));
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    vm_session_destroy(session);
    return 1;
}
