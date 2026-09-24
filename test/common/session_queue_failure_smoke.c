#include "common/session/control.h"
#include <assert.h>

static lib_i32 fail_allocation, fail_mutex, fail_event;
static lib_u32 blocks, mutexes, events;
static void *allocate(lib_size count, lib_size size)
{
    void *p = fail_allocation ? NULL : lib_allocate_zero(count, size);
    if (p != NULL) ++blocks;
    return p;
}
static void release(void *p)
{
    if (p != NULL) { assert(blocks != 0u); --blocks; }
    lib_release(p);
}
static lib_status create_mutex(base_sync_mutex **out)
{
    if (fail_mutex) { *out = NULL; return LIB_STATUS_NO_MEMORY; }
    lib_status status = base_sync_mutex_create(out);
    if (status == LIB_STATUS_OK) ++mutexes;
    return status;
}
static void destroy_mutex(base_sync_mutex *p)
{
    if (p != NULL) { assert(mutexes != 0u); --mutexes; }
    base_sync_mutex_destroy(p);
}
static lib_status create_event(base_sync_event_mode mode, base_sync_event **out)
{
    if (fail_event) { *out = NULL; return LIB_STATUS_NO_MEMORY; }
    lib_status status = base_sync_event_create(mode, out);
    if (status == LIB_STATUS_OK) ++events;
    return status;
}
static void destroy_event(base_sync_event *p)
{
    if (p != NULL) { assert(events != 0u); --events; }
    base_sync_event_destroy(p);
}
#define base_sync_mutex_create create_mutex
#define base_sync_mutex_destroy destroy_mutex
#define base_sync_event_create create_event
#define base_sync_event_destroy destroy_event
#define lib_release release
#define lib_allocate_zero allocate
#include "common/session/control.c"

int main(void)
{
    common_session_queue storage = { 0 }, *queue = &storage;
    common_session_event event;
    kvm_input_event key = { 0 };
    lib_u32 i, round;
    assert(!common_session_queue_initialize(NULL));
    common_session_queue_dispose(NULL);
    for (i = 0u; i < 3u; ++i) {
        fail_mutex = i == 0u;
        fail_allocation = i == 1u;
        fail_event = i == 2u;
        assert(!common_session_queue_initialize(queue));
        assert(queue->lock == NULL && queue->events == NULL && queue->available == NULL);
        assert(blocks == 0u && mutexes == 0u && events == 0u);
        common_session_queue_dispose(queue);
    }
    fail_mutex = fail_allocation = fail_event = 0;
    assert(common_session_queue_initialize(queue));
    assert(blocks == 1u && mutexes == 1u && events == 1u);
    for (round = 0u; round < 2u; ++round) {
        for (i = 0u; i < 64u; ++i)
            assert(common_session_queue_push_frame_completed(queue, i, 0, 1u));
        fail_allocation = 1;
        /* Reverse arrival order still retains the original class priority. */
        assert(!common_session_queue_push_runtime_completed(queue,
            COMMON_SESSION_MACHINE_PAUSED, 20u + round));
        assert(!common_session_queue_push_console_failed(queue));
        key.source_identity = 100u + round;
        assert(!common_session_queue_push_kvm_for_run(queue, &key, 30u + round));
        key.source_identity = 999u;
        assert(!common_session_queue_push_kvm_for_run(queue, &key, 999u));
        for (i = 0u; i < 64u; ++i) {
            assert(common_session_queue_take(queue, &event, 0u));
            assert(event.kind == COMMON_SESSION_EVENT_FRAME_COMPLETED);
            assert(event.value.frame.sequence == i);
        }
        assert(common_session_queue_take(queue, &event, 0u));
        assert(event.kind == COMMON_SESSION_EVENT_KVM_DELIVERY_FAILED);
        assert(event.run_generation == 30u + round);
        assert(event.value.delivery_failure.source_identity == 100u + round);
        assert(event.value.delivery_failure.status == LIB_STATUS_NO_MEMORY);
        assert(common_session_queue_take(queue, &event, 0u));
        assert(event.kind == COMMON_SESSION_EVENT_QUEUE_DELIVERY_FAILED);
        assert(event.run_generation == 20u + round);
        assert(event.value.queue_delivery_status == LIB_STATUS_NO_MEMORY);
        assert(!common_session_queue_take(queue, &event, 0u));
        fail_allocation = 0;
    }
    assert(common_session_queue_push_console_failed(queue));
    assert(common_session_queue_take(queue, &event, 0u));
    assert(event.kind == COMMON_SESSION_EVENT_CONSOLE_FAILED);
    common_session_queue_dispose(queue);
    assert(blocks == 0u && mutexes == 0u && events == 0u);
    assert(common_session_queue_initialize(queue));
    assert(!common_session_queue_take(queue, &event, 0u));
    common_session_queue_dispose(queue);
    assert(blocks == 0u && mutexes == 0u && events == 0u);
    return 0;
}
