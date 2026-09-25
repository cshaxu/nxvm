#include "lib/types/test.h"
#include "common/session/control.h"
#include "common/machine/input_queue.h"
#include "lib/base/sync_interface.h"

typedef struct probe {
    base_sync_mutex *mutex;
    base_sync_event *entered;
    base_sync_event *go;
    common_session_queue queue;
    lib_u32 count;
} probe;

static void producer(void *opaque, const base_sync_task *task)
{
    probe *p = opaque;
    lib_u32 i;
    (void)task;
    base_sync_event_signal(p->entered);
    lib_test_assert(base_sync_event_wait(p->go, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    for (i = 0; i < 1000u; ++i) {
        base_sync_mutex_lock(p->mutex);
        lib_test_assert(common_session_queue_push_frame_completed(&p->queue, p->count, 0, 7u));
        ++p->count;
        base_sync_mutex_unlock(p->mutex);
    }
}

int main(void)
{
    probe p = { 0 };
    base_sync_task *a = LIB_NULL, *b = LIB_NULL;
    common_session_event event = { 0 };
    common_machine_input_queue storage = { 0 }, *input = &storage;
    kvm_input_event key = { 0 }, copied = { 0 };
    lib_u32 i;
    lib_atomic_i32 atom;
    lib_test_assert(base_sync_mutex_create(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    base_sync_mutex_destroy(LIB_NULL);
    lib_test_assert(base_sync_mutex_create(&p.mutex) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &p.entered) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &p.go) == LIB_STATUS_OK);
    lib_test_assert(common_session_queue_initialize(&p.queue));
    lib_test_assert(!common_session_queue_take(&p.queue, &event, 0u));
    /* Force growth before a consumer exists; no fixed-capacity silent loss. */
    for (i = 0; i < 1000u; ++i)
        lib_test_assert(common_session_queue_push_frame_completed(&p.queue, i, 0, 9u));
    for (i = 0; i < 1000u; ++i) {
        lib_test_assert(common_session_queue_take(&p.queue, &event, 0u));
        lib_test_assert(event.value.frame.sequence == i && event.run_generation == 9u);
    }
    lib_test_assert(!common_session_queue_take(&p.queue, &event, 0u));
    base_sync_mutex_lock(p.mutex);
    lib_test_assert(base_sync_task_create(producer, &p, &a) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(p.entered, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    base_sync_event_reset(p.entered);
    lib_test_assert(base_sync_task_create(producer, &p, &b) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(p.entered, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    base_sync_event_signal(p.go);
    lib_test_assert(p.count == 0u);
    base_sync_mutex_unlock(p.mutex);
    for (i = 0; i < 2000u; ++i) {
        lib_test_assert(common_session_queue_take(&p.queue, &event, LIB_UINT32_MAX));
        lib_test_assert(event.kind == COMMON_SESSION_EVENT_FRAME_COMPLETED);
        lib_test_assert(event.value.frame.sequence == i && event.run_generation == 7u);
    }
    base_sync_task_destroy(a);
    base_sync_task_destroy(b);
    lib_test_assert(p.count == 2000u);
    lib_test_assert(!common_session_queue_take(&p.queue, &event, 0u));
    lib_test_assert(common_session_queue_push_console_failed(&p.queue));
    lib_test_assert(common_session_queue_take(&p.queue, &event, 0u));
    lib_test_assert(event.kind == COMMON_SESSION_EVENT_CONSOLE_FAILED);
    lib_test_assert(!common_session_queue_take(&p.queue, &event, 0u));
    common_session_queue_dispose(&p.queue);
    base_sync_event_destroy(p.go);
    base_sync_event_destroy(p.entered);
    base_sync_mutex_destroy(p.mutex);

    lib_test_assert(common_machine_input_queue_initialize(input) == LIB_STATUS_OK);
    key.type = KVM_EVENT_KEY;
    for (i = 0; i < 255u; ++i) {
        key.source_identity = i;
        lib_test_assert(common_machine_input_queue_push(input, &key));
    }
    lib_test_assert(!common_machine_input_queue_push(input, &key));
    for (i = 0; i < 255u; ++i) {
        lib_test_assert(common_machine_input_queue_pop(input, &copied));
        lib_test_assert(copied.source_identity == i);
    }
    lib_test_assert(!common_machine_input_queue_pending(input));
    lib_test_assert(common_machine_input_queue_push(input, &key));
    common_machine_input_queue_clear(input);
    lib_test_assert(!common_machine_input_queue_pop(input, &copied));
    common_machine_input_queue_dispose(input);

    lib_atomic_i32_initialize(&atom, 0);
    lib_test_assert(lib_atomic_i32_fetch_add_explicit(&atom, 3, LIB_MEMORY_ORDER_SEQ_CST) == 0);
    lib_test_assert(lib_atomic_i32_exchange_explicit(&atom, 7, LIB_MEMORY_ORDER_SEQ_CST) == 3);
    lib_test_assert(lib_atomic_i32_load_explicit(&atom, LIB_MEMORY_ORDER_SEQ_CST) == 7);
    return 0;
}
