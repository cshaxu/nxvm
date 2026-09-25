#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/base/win32/sync.c"

static lib_win32_handle contended;
static lib_win32_long contender;
static void tracked_lock(base_sync_mutex *mutex)
{
    if (lib_win32_get_current_thread_id() == (lib_win32_dword)lib_win32_interlocked_compare_exchange(&contender, 0, 0)) {
        if (lib_win32_try_enter_critical_section(&mutex->gate)) return;
        lib_win32_set_event(contended); /* The native blocking lock is actually owned. */
    }
    base_sync_mutex_lock(mutex);
}
#define base_sync_mutex_lock tracked_lock
#include "lib/kvm-base/mailbox.c"
#undef base_sync_mutex_lock

static kvm_component_mailboxes mailbox;
static lib_u32 frame = 80u, copy, storage;
static lib_i32 operation;
static lib_win32_dword LIB_WIN32_WINAPI compete(void *unused)
{
    lib_u32 generation;
    const kvm_component_control stop = { .kind = KVM_COMPONENT_CONTROL_STOP };
    (void)unused;
    lib_win32_interlocked_exchange(&contender, (lib_win32_long)lib_win32_get_current_thread_id());
    if (operation == 0)
        lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame, sizeof(frame)) == LIB_STATUS_OK);
    else if (operation == 1) {
        lib_test_assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &copy, sizeof(copy)));
        lib_test_assert(copy == 80 && generation == 1);
    } else if (operation == 2)
        lib_test_assert(kvm_component_mailboxes_enqueue_control(&mailbox, &stop) == LIB_STATUS_OK);
    else {
        kvm_component_control control;
        lib_test_assert(!kvm_component_mailboxes_take_control(&mailbox, &control));
    }
    return 0;
}

int main(void)
{
    for (operation = 0; operation < 4; ++operation) {
        lib_win32_handle thread;
        kvm_component_control taken;
        const kvm_component_control title = { .kind = 42u };
        lib_test_assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
        lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame, sizeof(frame)) == LIB_STATUS_OK);
        contended = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        lib_test_assert(contended);
        base_sync_mutex *held = operation == 3 ? mailbox.control_lock : mailbox.frame_lock;
        base_sync_mutex_lock(held);
        thread = lib_win32_create_thread(LIB_NULL, 0, compete, LIB_NULL, 0, LIB_NULL);
        lib_test_assert(thread && lib_win32_wait_for_single_object(contended, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(lib_win32_wait_for_single_object(thread, 0) == LIB_WIN32_WAIT_TIMEOUT);
        /* Even STOP's pending frame-lock acquisition must not hold control. */
        if (operation == 3) {
            lib_u32 generation;
            lib_test_assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &copy, sizeof(copy)));
        } else {
            lib_test_assert(kvm_component_mailboxes_enqueue_control(&mailbox, &title) == LIB_STATUS_OK);
            lib_test_assert(kvm_component_mailboxes_take_control(&mailbox, &taken));
            lib_test_assert(taken.kind == 42u);
        }
        base_sync_mutex_unlock(held);
        lib_test_assert(lib_win32_wait_for_single_object(thread, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        if (operation == 2) {
            lib_test_assert(kvm_component_mailboxes_take_control(&mailbox, &taken));
            lib_test_assert(taken.kind == KVM_COMPONENT_CONTROL_STOP);
            lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame, sizeof(frame)) == LIB_STATUS_INVALID_STATE);
        }
        lib_win32_interlocked_exchange(&contender, 0);
        lib_win32_close_handle(thread);
        lib_win32_close_handle(contended);
        kvm_component_mailboxes_destroy(&mailbox);
    }
    return 0;
}
