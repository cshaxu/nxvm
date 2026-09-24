#include <assert.h>
#include <windows.h>
#include "lib/base/win32/sync.c"

static HANDLE contended;
static LONG contender;
static void tracked_lock(base_sync_mutex *mutex)
{
    if (GetCurrentThreadId() == (DWORD)InterlockedCompareExchange(&contender, 0, 0)) {
        if (TryEnterCriticalSection(&mutex->gate)) return;
        SetEvent(contended); /* The native blocking lock is actually owned. */
    }
    base_sync_mutex_lock(mutex);
}
#define base_sync_mutex_lock tracked_lock
#include "lib/kvm-base/mailbox.c"
#undef base_sync_mutex_lock

static kvm_component_mailboxes mailbox;
static lib_u32 frame = 80u, copy, storage;
static lib_i32 operation;
static DWORD WINAPI compete(void *unused)
{
    lib_u32 generation;
    const kvm_component_control stop = { .kind = KVM_COMPONENT_CONTROL_STOP };
    (void)unused;
    InterlockedExchange(&contender, (LONG)GetCurrentThreadId());
    if (operation == 0)
        assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame, sizeof(frame)) == LIB_STATUS_OK);
    else if (operation == 1) {
        assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &copy, sizeof(copy)));
        assert(copy == 80 && generation == 1);
    } else if (operation == 2)
        assert(kvm_component_mailboxes_enqueue_control(&mailbox, &stop) == LIB_STATUS_OK);
    else {
        kvm_component_control control;
        assert(!kvm_component_mailboxes_take_control(&mailbox, &control));
    }
    return 0;
}

int main(void)
{
    for (operation = 0; operation < 4; ++operation) {
        HANDLE thread;
        kvm_component_control taken;
        const kvm_component_control title = { .kind = 42u };
        assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
        assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame, sizeof(frame)) == LIB_STATUS_OK);
        contended = CreateEventA(NULL, TRUE, FALSE, NULL);
        assert(contended);
        base_sync_mutex *held = operation == 3 ? mailbox.control_lock : mailbox.frame_lock;
        base_sync_mutex_lock(held);
        thread = CreateThread(NULL, 0, compete, NULL, 0, NULL);
        assert(thread && WaitForSingleObject(contended, 5000) == WAIT_OBJECT_0);
        assert(WaitForSingleObject(thread, 0) == WAIT_TIMEOUT);
        /* Even STOP's pending frame-lock acquisition must not hold control. */
        if (operation == 3) {
            lib_u32 generation;
            assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &copy, sizeof(copy)));
        } else {
            assert(kvm_component_mailboxes_enqueue_control(&mailbox, &title) == LIB_STATUS_OK);
            assert(kvm_component_mailboxes_take_control(&mailbox, &taken));
            assert(taken.kind == 42u);
        }
        base_sync_mutex_unlock(held);
        assert(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0);
        if (operation == 2) {
            assert(kvm_component_mailboxes_take_control(&mailbox, &taken));
            assert(taken.kind == KVM_COMPONENT_CONTROL_STOP);
            assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame, sizeof(frame)) == LIB_STATUS_INVALID_STATE);
        }
        InterlockedExchange(&contender, 0);
        CloseHandle(thread);
        CloseHandle(contended);
        kvm_component_mailboxes_destroy(&mailbox);
    }
    return 0;
}
