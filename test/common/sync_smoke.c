#include "common/session/control.h"
#include "common/machine/input_queue.h"
#include "lib/host/sync_interface.h"
#include <assert.h>

typedef struct probe {
    host_sync_mutex *mutex;
    host_sync_event *entered;
    host_sync_event *go;
    common_session_queue *queue;
    lib_u32 count;
} probe;

static void producer(void *opaque, const host_sync_task *task)
{
    probe *p = opaque;
    lib_u32 i;
    (void)task;
    host_sync_event_signal(p->entered);
    assert(host_sync_event_wait(p->go, 5000u) == HOST_SYNC_WAIT_SIGNALED);
    for (i = 0; i < 1000u; ++i) {
        host_sync_mutex_lock(p->mutex);
        assert(common_session_queue_push_frame_completed(p->queue, p->count, 0, 7u));
        ++p->count;
        host_sync_mutex_unlock(p->mutex);
    }
}

int main(void)
{
    probe p = { 0 };
    host_sync_task *a = NULL, *b = NULL;
    common_session_event event = { 0 };
    common_machine_input_queue *input = NULL;
    kvm_input_event key = { 0 }, copied = { 0 };
    lib_u32 i;
    lib_atomic_i32 atom;
    assert(host_sync_mutex_create(NULL) == LIB_STATUS_INVALID_ARGUMENT);
    host_sync_mutex_destroy(NULL);
    assert(host_sync_mutex_create(&p.mutex) == LIB_STATUS_OK);
    assert(host_sync_event_create(&p.entered) == LIB_STATUS_OK);
    assert(host_sync_event_create(&p.go) == LIB_STATUS_OK);
    assert(common_session_queue_create(&p.queue));
    assert(!common_session_queue_take(p.queue, &event, 0u));
    /* Force growth before a consumer exists; no fixed-capacity silent loss. */
    for (i = 0; i < 1000u; ++i)
        assert(common_session_queue_push_frame_completed(p.queue, i, 0, 9u));
    for (i = 0; i < 1000u; ++i) {
        assert(common_session_queue_take(p.queue, &event, 0u));
        assert(event.value.frame.sequence == i && event.run_generation == 9u);
    }
    assert(!common_session_queue_take(p.queue, &event, 0u));
    host_sync_mutex_lock(p.mutex);
    assert(host_sync_task_create(producer, &p, &a) == LIB_STATUS_OK);
    assert(host_sync_event_wait(p.entered, 5000u) == HOST_SYNC_WAIT_SIGNALED);
    host_sync_event_reset(p.entered);
    assert(host_sync_task_create(producer, &p, &b) == LIB_STATUS_OK);
    assert(host_sync_event_wait(p.entered, 5000u) == HOST_SYNC_WAIT_SIGNALED);
    host_sync_event_signal(p.go);
    assert(p.count == 0u);
    host_sync_mutex_unlock(p.mutex);
    for (i = 0; i < 2000u; ++i) {
        assert(common_session_queue_take(p.queue, &event, 5000u));
        assert(event.kind == COMMON_SESSION_EVENT_FRAME_COMPLETED);
        assert(event.value.frame.sequence == i && event.run_generation == 7u);
    }
    host_sync_task_destroy(a);
    host_sync_task_destroy(b);
    assert(p.count == 2000u);
    assert(!common_session_queue_take(p.queue, &event, 0u));
    assert(common_session_queue_push_console_failed(p.queue));
    assert(common_session_queue_take(p.queue, &event, 0u));
    assert(event.kind == COMMON_SESSION_EVENT_CONSOLE_FAILED);
    assert(!common_session_queue_take(p.queue, &event, 0u));
    common_session_queue_destroy(p.queue);
    host_sync_event_destroy(p.go);
    host_sync_event_destroy(p.entered);
    host_sync_mutex_destroy(p.mutex);

    assert(common_machine_input_queue_create(&input) == LIB_STATUS_OK);
    key.type = KVM_EVENT_KEY;
    for (i = 0; i < 255u; ++i) {
        key.source_identity = i;
        assert(common_machine_input_queue_push(input, &key));
    }
    assert(!common_machine_input_queue_push(input, &key));
    for (i = 0; i < 255u; ++i) {
        assert(common_machine_input_queue_pop(input, &copied));
        assert(copied.source_identity == i);
    }
    assert(!common_machine_input_queue_pending(input));
    assert(common_machine_input_queue_push(input, &key));
    common_machine_input_queue_clear(input);
    assert(!common_machine_input_queue_pop(input, &copied));
    common_machine_input_queue_destroy(input);

    lib_atomic_i32_initialize(&atom, 0);
    assert(lib_atomic_i32_fetch_add_explicit(&atom, 3, LIB_MEMORY_ORDER_SEQ_CST) == 0);
    assert(lib_atomic_i32_exchange_explicit(&atom, 7, LIB_MEMORY_ORDER_SEQ_CST) == 3);
    assert(lib_atomic_i32_load_explicit(&atom, LIB_MEMORY_ORDER_SEQ_CST) == 7);
    return 0;
}
