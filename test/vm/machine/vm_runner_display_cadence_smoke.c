#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_RUNNER_DISPLAY_CADENCE_RUN_MILLISECONDS 100u
#define VM_RUNNER_DISPLAY_CADENCE_MAX_FRAMES 12u

static DWORD WINAPI vm_runner_display_cadence_run(C_VOID *opaque)
{
    vm_session_control_start(&((vm_session *)opaque)->control);
    return 0u;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xb8u, 0x00u, 0xb8u, /* mov ax, 0xb800 */
        0x8eu, 0xc0u,       /* mov es, ax */
        0xbfu, 0x00u, 0x00u, /* mov di, 0 */
        0xb0u, 0x41u,       /* mov al, 'A' */
        0xaau,              /* stosb */
        0xfeu, 0xc0u,      /* inc al */
        0xebu, 0xfbu        /* jmp stosb */
    };
    vm_session *session = STD_NULL;
    HANDLE thread = STD_NULL;
    type_unsigned_64 generation;
    C_INT failed = 0;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_session_control_reset(&session->control) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine, 0xffff0u, program,
            sizeof(program)) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    thread = CreateThread(STD_NULL, 0u, vm_runner_display_cadence_run, session,
        0u, STD_NULL);
    if (thread == STD_NULL) {
        failed = 1;
        goto done;
    }
    Sleep(VM_RUNNER_DISPLAY_CADENCE_RUN_MILLISECONDS);
    vm_session_control_request_pause(&session->control, VM_SESSION_PAUSE_EXPLICIT);
    if (!vm_session_control_wait_for_pause(&session->control, 2000u)) {
        failed = 1;
        goto done;
    }
    generation = session->display_generation;
    failed = generation < 2u || generation > VM_RUNNER_DISPLAY_CADENCE_MAX_FRAMES;

done:
    vm_session_control_stop(&session->control);
    if (thread != STD_NULL) {
        if (WaitForSingleObject(thread, 2000u) != WAIT_OBJECT_0) failed = 1;
        CloseHandle(thread);
    }
    if (session != STD_NULL) vm_session_destroy(session);
    if (failed) return 1;
    puts("M5:T212:S2:RUNNER-CADENCE:OK");
    puts("M5:T458:S1:RUNNER-PRESENTATION-CADENCE:OK");
    return 0;
}
