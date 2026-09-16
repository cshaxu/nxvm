#include "lib/types/types_interface.h"
#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/kvm-console/console.h"
#include "lib/base/sync_interface.h"

#include "lib/types/win32/console.h"

typedef struct kvm_console_win32_state {
    lib_win32_handle worker;
} kvm_console_win32_state;

lib_u8 kvm_console_mouse_buttons(lib_u32 buttons)
{
    lib_u8 result = (buttons & LIB_WIN32_FROM_LEFT_1ST_BUTTON_PRESSED) != 0u ?
        KVM_MOUSE_BUTTON_LEFT : 0u;
    if ((buttons & LIB_WIN32_RIGHTMOST_BUTTON_PRESSED) != 0u)
        result |= KVM_MOUSE_BUTTON_RIGHT;
    return result;
}

static lib_win32_dword LIB_WIN32_WINAPI kvm_console_worker(void *opaque)
{
    kvm_console *console = (kvm_console *)opaque;
    lib_u32 generation = 0u;

    while (lib_atomic_i32_load_explicit(&console->base.stopping,
        LIB_MEMORY_ORDER_ACQUIRE) == 0) {
        kvm_component_control control;
        kvm_frame frame;
        base_sync_wait_result wake = base_sync_event_wait(
            console->base.mailboxes.wake, LIB_UINT32_MAX);
        if (wake != BASE_SYNC_WAIT_SIGNALED) {
            kvm_component_fail(&console->base, LIB_STATUS_IO_ERROR);
            break;
        }
        if (lib_atomic_i32_load_explicit(&console->base.stopping,
                LIB_MEMORY_ORDER_ACQUIRE) != 0) break;
        while (kvm_component_mailboxes_take_control(&console->base.mailboxes, &control)) {
            if (control.kind == KVM_COMPONENT_CONTROL_STOP) {
                goto retired;
            }
            /* kvm-console has no title or mouse surface. Unsupported Window
             * control entries are intentionally consumed as no-ops. */
        }
        if (kvm_component_mailboxes_capture_frame(&console->base.mailboxes,
                &generation, &frame)) {
            lib_status status = kvm_console_publish_text_frame(console, &frame);
            if (status == LIB_STATUS_OK)
                kvm_component_mailboxes_acknowledge_frame(&console->base.mailboxes,
                    generation);
            if (status != LIB_STATUS_OK && status != LIB_STATUS_NOT_CURRENT) {
                kvm_component_fail(&console->base, status);
                break;
            }
        }
    }
retired:
    /* Detach waits for any in-flight callback on every exit, including a
     * failed wake. Retirement is the final input fact from this source. */
    lib_atomic_i32_store_explicit(&console->base.stopping, 1, LIB_MEMORY_ORDER_RELEASE);
    (void)lib_console_set_event_sink(console->logical_console, LIB_NULL, LIB_NULL);
    kvm_component_retire(&console->base, LIB_STATUS_OK);
    return 0u;
}

lib_status kvm_console_worker_start(kvm_console *console)
{
    kvm_console_win32_state *state;

    if (console == LIB_NULL || console->logical_console == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    state = lib_allocate_zero(1u, sizeof(*state));
    if (state == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    console->worker_state = state;
    /* Install before starting: an immediate worker failure must not be
     * followed by reattaching the retired source from this thread. */
    if (lib_console_set_event_sink(console->logical_console,
            kvm_console_receive_event, console) != LIB_STATUS_OK) {
        console->worker_state = LIB_NULL;
        lib_release(state);
        return LIB_STATUS_INVALID_STATE;
    }
    state->worker = lib_win32_create_thread(LIB_NULL, 0u, kvm_console_worker, console, 0u, LIB_NULL);
    if (state->worker == LIB_NULL) {
        if (lib_console_set_event_sink(console->logical_console, LIB_NULL, LIB_NULL) != LIB_STATUS_OK)
            return LIB_STATUS_IO_ERROR;
        console->worker_state = LIB_NULL;
        lib_release(state);
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

lib_status kvm_console_worker_join(kvm_console *console, lib_u32 timeout_ms)
{
    kvm_console_win32_state *state;

    if (console == LIB_NULL || (state = (kvm_console_win32_state *)
            console->worker_state) == LIB_NULL) return LIB_STATUS_OK;
    /* kvm_component_destroy has queued STOP; wait for the Console worker to
     * consume it before detaching the event sink or releasing state. */
    if (lib_win32_wait_for_single_object(state->worker, timeout_ms) != LIB_WIN32_WAIT_OBJECT_0)
        return LIB_STATUS_IO_ERROR;
    if (!lib_win32_close_handle(state->worker)) return LIB_STATUS_IO_ERROR;
    console->worker_state = LIB_NULL;
    lib_release(state);
    return LIB_STATUS_OK;
}
