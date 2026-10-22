#include "lib/types/types_interface.h"
#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/kvm-console/console.h"
#include "lib/base/sync_interface.h"

#include "lib/types/win32/console.h"

typedef struct kvm_console_win32_state {
    lib_win32_handle worker;
    base_sync_event *stop;
} kvm_console_win32_state;

static lib_win32_dword LIB_WIN32_WINAPI kvm_console_worker(void *opaque)
{
    kvm_console *console = (kvm_console *)opaque;

    while (lib_atomic_i32_load_explicit(&console->base.stopping,
        LIB_MEMORY_ORDER_ACQUIRE) == 0) {
        base_sync_event *events[2];
        kvm_component_control control;
        const kvm_console_text_frame *frame;
        const void *previous;
        lib_u32 event_index;
        kvm_console_win32_state *state = console->worker_state;
        base_sync_wait_result wake;

        if (state == LIB_NULL || state->stop == LIB_NULL) break;
        events[0] = console->base.mailboxes.wake;
        events[1] = state->stop;
        wake = base_sync_wait_any(events, 2u, LIB_NULL, LIB_UINT32_MAX,
            &event_index);
        if (wake != BASE_SYNC_WAIT_SIGNALED) {
            kvm_component_fail(&console->base, LIB_STATUS_IO_ERROR);
            break;
        }
        if (event_index == 1u) goto retired;
        if (lib_atomic_i32_load_explicit(&console->base.stopping,
                LIB_MEMORY_ORDER_ACQUIRE) != 0) break;
        while (kvm_component_mailboxes_take_control(&console->base.mailboxes, &control)) {
            if (control.kind == KVM_COMPONENT_CONTROL_STOP) {
                goto retired;
            }
            /* This consumer has no ordinary controls. The transport accepts
             * opaque kinds, but only the consumer can validate their meaning. */
            kvm_component_fail(&console->base, LIB_STATUS_INVALID_ARGUMENT);
            goto retired;
        }
        frame = kvm_component_mailboxes_take_frame(&console->base.mailboxes, &previous);
        if (frame != LIB_NULL) {
            lib_status status = kvm_console_publish_text_frame(console, frame, previous);
            kvm_component_mailboxes_finish_frame(&console->base.mailboxes,
                status == LIB_STATUS_OK);
            if (status != LIB_STATUS_OK) {
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
    lib_status status;

    if (console == LIB_NULL || console->logical_console == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    state = lib_allocate_zero(1u, sizeof(*state));
    if (state == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    console->worker_state = state;
    if (base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &state->stop) != LIB_STATUS_OK) {
        console->worker_state = LIB_NULL;
        lib_release(state);
        return LIB_STATUS_NO_MEMORY;
    }
    state->worker = lib_win32_create_thread(LIB_NULL, 0u, kvm_console_worker, console, 0u, LIB_NULL);
    if (state->worker == LIB_NULL) {
        console->worker_state = LIB_NULL;
        base_sync_event_destroy(state->stop);
        lib_release(state);
        return LIB_STATUS_NO_MEMORY;
    }
    /* No Console input can arrive before create returns.  Starting first lets
     * a failed sink attachment use the same terminal wake and join path,
     * rather than leaving an inaccessible worker that cannot be released. */
    status = lib_console_set_event_sink(console->logical_console,
        kvm_console_receive_event, console);
    if (status == LIB_STATUS_OK) return LIB_STATUS_OK;
    (void)kvm_console_worker_join(console);
    return status;
}

lib_status kvm_console_worker_join(kvm_console *console)
{
    kvm_console_win32_state *state;
    lib_status status;

    if (console == LIB_NULL || (state = (kvm_console_win32_state *)
            console->worker_state) == LIB_NULL) return LIB_STATUS_OK;
    /* STOP closes normal admission. This separate terminal wake guarantees the
     * worker can observe it even when ordinary mailbox notification failed. */
    status = base_sync_event_signal(state->stop);
    if (lib_win32_wait_for_single_object(state->worker,
            status == LIB_STATUS_OK ? LIB_WIN32_INFINITE : 0u) != LIB_WIN32_WAIT_OBJECT_0)
        lib_process_exit_immediate(LIB_STATUS_INTERNAL_ERROR);
    if (!lib_win32_close_handle(state->worker)) status = LIB_STATUS_IO_ERROR;
    console->worker_state = LIB_NULL;
    base_sync_event_destroy(state->stop);
    lib_release(state);
    return status;
}
