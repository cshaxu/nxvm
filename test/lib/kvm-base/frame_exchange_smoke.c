#include "lib/types/test.h"
#include "lib/kvm-base/frame_exchange_interface.h"
#include "lib/base/sync_interface.h"

static lib_u32 allocation, fail_at, live_allocations;
static lib_bool fail_mutex;
static lib_status create_mutex(base_sync_mutex **out_mutex)
{
    if (!fail_mutex) return base_sync_mutex_create(out_mutex);
    *out_mutex = LIB_NULL;
    return LIB_STATUS_IO_ERROR;
}
static void *allocate_frame(lib_size count, lib_size size)
{
    if (++allocation == fail_at) return LIB_NULL;
    void *memory = lib_allocate_zero(count, size);
    if (memory != LIB_NULL) ++live_allocations;
    return memory;
}
static void release_frame(void *memory)
{
    if (memory != LIB_NULL) { lib_test_assert(live_allocations > 0u); --live_allocations; }
    lib_release(memory);
}
#define lib_allocate_zero allocate_frame
#define lib_release release_frame
#define base_sync_mutex_create create_mutex
#include "lib/kvm-base/frame_exchange.c"
#undef base_sync_mutex_create
#undef lib_allocate_zero
#undef lib_release

typedef struct test_frame { lib_u32 serial, complement; } test_frame;

static void publish(kvm_frame_exchange *exchange, lib_u32 serial)
{
    test_frame *frame = kvm_frame_exchange_writer(exchange);
    lib_test_assert(frame != LIB_NULL);
    frame->serial = serial;
    frame->complement = ~serial;
    kvm_frame_exchange_publish(exchange);
}

static void check(const test_frame *frame, lib_u32 serial)
{
    lib_test_assert(frame != LIB_NULL && frame->serial == serial &&
        frame->complement == ~serial);
}

typedef struct latest_check { lib_u32 serial; lib_bool matched; } latest_check;

static void check_latest(void *opaque, const void *frame)
{
    latest_check *check_context = opaque;
    check((const test_frame *)frame, check_context->serial);
    check_context->matched = LIB_TRUE;
}

static lib_bool latest_is(kvm_frame_exchange *exchange, lib_u32 serial)
{
    latest_check check_context = { serial, LIB_FALSE };
    return kvm_frame_exchange_observe_latest(exchange, check_latest,
        &check_context) && check_context.matched;
}

static void check_latest_at_least(void *opaque, const void *frame)
{
    latest_check *check_context = opaque;
    const test_frame *latest = frame;
    lib_test_assert(latest->serial >= check_context->serial);
    check(latest, latest->serial);
    check_context->matched = LIB_TRUE;
}

static void exercise(void)
{
    kvm_frame_exchange *exchange = LIB_NULL;
    const test_frame *processing, *displayed;
    void *slots[4] = { 0 };
    lib_u32 count = 0u;
    lib_test_assert(kvm_frame_exchange_create(&exchange,
        sizeof(test_frame), LIB_NULL) == LIB_STATUS_OK);
    lib_u32 initial_allocations = allocation;
    lib_test_assert(kvm_frame_exchange_take(exchange, LIB_NULL) == LIB_NULL);
    lib_test_assert(!kvm_frame_exchange_observe_latest(exchange, check_latest, LIB_NULL));
    publish(exchange, 1u);
    displayed = kvm_frame_exchange_take(exchange, LIB_NULL);
    check(displayed, 1u);
    kvm_frame_exchange_finish(exchange, LIB_TRUE);
    {
        test_frame *writer = kvm_frame_exchange_writer(exchange);
        lib_test_assert(writer != displayed);
        writer->serial = 999u;
        check(displayed, 1u);
        kvm_frame_exchange_redraw(exchange);
        kvm_frame_exchange_redraw(exchange);
        lib_test_assert(kvm_frame_exchange_take(exchange, LIB_NULL) == displayed);
        /* Activation racing an in-flight output retains one pending redraw. */
        kvm_frame_exchange_redraw(exchange);
        kvm_frame_exchange_finish(exchange, LIB_TRUE);
        lib_test_assert(kvm_frame_exchange_take(exchange, LIB_NULL) == displayed);
        kvm_frame_exchange_finish(exchange, LIB_TRUE);
        lib_test_assert(kvm_frame_exchange_take(exchange, LIB_NULL) == LIB_NULL);
    }
    publish(exchange, 2u);
    const void *previous;
    processing = kvm_frame_exchange_take(exchange, &previous);
    lib_test_assert(previous == displayed);
    check(processing, 2u);
    lib_test_assert(kvm_frame_exchange_take(exchange, LIB_NULL) == LIB_NULL);
    /* A stalled consumer cannot prevent publication or mutate held inputs.
     * Record distinct writer addresses rather than merely checking values. */
    for (lib_u32 serial = 3u; serial < 10003u; ++serial) {
        void *writer = kvm_frame_exchange_writer(exchange);
        lib_u32 i;
        lib_test_assert(writer != processing && writer != displayed);
        for (i = 0u; i < count && slots[i] != writer; ++i) {}
        if (i == count) { lib_test_assert(count < 2u); slots[count++] = writer; }
        publish(exchange, serial);
        check(processing, 2u);
        check(displayed, 1u);
        lib_test_assert(latest_is(exchange, serial));
    }
    lib_test_assert(count == 2u);
    /* Failed old output must never replace the newer pending publication. */
    kvm_frame_exchange_redraw(exchange);
    kvm_frame_exchange_finish(exchange, LIB_FALSE);
    processing = kvm_frame_exchange_take(exchange, &previous);
    lib_test_assert(previous == displayed);
    check(displayed, 1u);
    check(processing, 10002u);
    kvm_frame_exchange_finish(exchange, LIB_FALSE);
    lib_test_assert(kvm_frame_exchange_take(exchange, LIB_NULL) == processing);
    kvm_frame_exchange_finish(exchange, LIB_TRUE);
    lib_test_assert(kvm_frame_exchange_take(exchange, LIB_NULL) == LIB_NULL);
    kvm_frame_exchange_detach(exchange);
    check(kvm_frame_exchange_take(exchange, LIB_NULL), 10002u);
    /* Invalidating a run cannot write into an in-flight immutable input or
     * requeue it when that consumer finishes. */
    kvm_frame_exchange_invalidate(exchange);
    kvm_frame_exchange_redraw(exchange);
    check(processing, 10002u);
    lib_test_assert(!kvm_frame_exchange_observe_latest(exchange, check_latest, LIB_NULL));
    kvm_frame_exchange_finish(exchange, LIB_FALSE);
    lib_test_assert(kvm_frame_exchange_take(exchange, LIB_NULL) == LIB_NULL);
    publish(exchange, 10003u);
    check(kvm_frame_exchange_take(exchange, LIB_NULL), 10003u);
    kvm_frame_exchange_finish(exchange, LIB_TRUE);
    kvm_frame_exchange_detach(exchange);
    lib_test_assert(allocation == initial_allocations);
    kvm_frame_exchange_destroy(exchange);
}

typedef struct concurrent_test {
    kvm_frame_exchange *exchange;
    base_sync_event *held, *produced;
} concurrent_test;

static void produce(void *context, const base_sync_task *task)
{
    concurrent_test *test = context;
    (void)task;
    lib_test_assert(base_sync_event_wait(test->held, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    for (lib_u32 serial = 3u; serial <= 100000u; ++serial)
        publish(test->exchange, serial);
    lib_test_assert(base_sync_event_signal(test->produced) == LIB_STATUS_OK);
}

static void concurrent_exchange(void)
{
    concurrent_test test = { 0 };
    base_sync_task *producer;
    const test_frame *displayed, *processing;
    lib_test_assert(kvm_frame_exchange_create(&test.exchange,
        sizeof(test_frame), LIB_NULL) == LIB_STATUS_OK);
    lib_u32 initial_allocations = allocation;
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &test.held) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &test.produced) == LIB_STATUS_OK);
    publish(test.exchange, 1u);
    displayed = kvm_frame_exchange_take(test.exchange, LIB_NULL);
    kvm_frame_exchange_finish(test.exchange, LIB_TRUE);
    publish(test.exchange, 2u);
    processing = kvm_frame_exchange_take(test.exchange, LIB_NULL);
    lib_test_assert(base_sync_task_create(produce, &test, &producer) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_signal(test.held) == LIB_STATUS_OK);
    /* Actual concurrent reads, while the producer repeatedly overwrites the
     * pending slot. No consumer progress is required to complete production. */
    base_sync_wait_result result;
    do {
        check(processing, 2u);
        check(displayed, 1u);
        result = base_sync_event_wait(test.produced, 0u);
    } while (result == BASE_SYNC_WAIT_TIMED_OUT);
    lib_test_assert(result == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(base_sync_task_destroy(producer) == LIB_STATUS_OK);
    kvm_frame_exchange_finish(test.exchange, LIB_TRUE);
    check(kvm_frame_exchange_take(test.exchange, LIB_NULL), 100000u);
    kvm_frame_exchange_finish(test.exchange, LIB_TRUE);
    base_sync_event_destroy(test.held);
    base_sync_event_destroy(test.produced);
    lib_test_assert(allocation == initial_allocations);
    kvm_frame_exchange_destroy(test.exchange);
}

/* Both sides progress: publication races take/finish, with a checkpoint every
 * 32 frames to guarantee consumer turnover even on a single-core scheduler.
 * These test barriers do not replace the stalled-consumer test above. */
static void produce_turnover(void *context, const base_sync_task *task)
{
    concurrent_test *test = context;
    (void)task;
    for (lib_u32 serial = 1u; serial <= 32000u; ++serial) {
        publish(test->exchange, serial);
        lib_test_assert(base_sync_event_signal(test->produced) == LIB_STATUS_OK);
        if (serial % 32u == 0u)
            lib_test_assert(base_sync_event_wait(test->held, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    }
}

static void concurrent_turnover(void)
{
    concurrent_test test = { 0 };
    base_sync_task *producer;
    const test_frame *displayed = LIB_NULL;
    lib_u32 last = 0u, displayed_serial = 0u, checkpoint = 0u, consumed = 0u;
    lib_bool previous_failed = LIB_FALSE;
    lib_test_assert(kvm_frame_exchange_create(&test.exchange,
        sizeof(test_frame), LIB_NULL) == LIB_STATUS_OK);
    lib_u32 initial_allocations = allocation;
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &test.held) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &test.produced) == LIB_STATUS_OK);
    lib_test_assert(base_sync_task_create(produce_turnover, &test, &producer) == LIB_STATUS_OK);
    while (last < 32000u) {
        const test_frame *processing = kvm_frame_exchange_take(test.exchange, LIB_NULL);
        if (processing == LIB_NULL) {
            lib_test_assert(base_sync_event_wait(test.produced, 5000u) == BASE_SYNC_WAIT_SIGNALED);
            continue;
        }
        lib_u32 serial = processing->serial;
        ++consumed;
        lib_bool success = previous_failed || serial == 32000u || consumed % 17u != 0u;
        lib_test_assert(serial > last || (previous_failed && serial == last));
        check(processing, serial);
        if (displayed != LIB_NULL) {
            lib_test_assert(displayed != processing);
            check(displayed, displayed_serial);
        }
        /* Observation is synchronous and bounded; it never exposes the
         * exchange lock or extends the frame lifetime. */
        latest_check latest = { serial, LIB_FALSE };
        lib_test_assert(kvm_frame_exchange_observe_latest(test.exchange,
            check_latest_at_least, &latest) && latest.matched);
        check(processing, serial);
        if (displayed != LIB_NULL) check(displayed, displayed_serial);
        kvm_frame_exchange_finish(test.exchange, success);
        if (success) {
            displayed = processing;
            displayed_serial = serial;
        }
        last = serial;
        previous_failed = !success;
        if (serial % 32u == 0u && serial != checkpoint) {
            checkpoint = serial;
            lib_test_assert(base_sync_event_signal(test.held) == LIB_STATUS_OK);
        }
    }
    lib_test_assert(consumed >= 1000u);
    lib_test_assert(base_sync_task_destroy(producer) == LIB_STATUS_OK);
    lib_test_assert(kvm_frame_exchange_take(test.exchange, LIB_NULL) == LIB_NULL);
    kvm_frame_exchange_detach(test.exchange);
    check(kvm_frame_exchange_take(test.exchange, LIB_NULL), 32000u);
    kvm_frame_exchange_finish(test.exchange, LIB_TRUE);
    base_sync_event_destroy(test.held);
    base_sync_event_destroy(test.produced);
    lib_test_assert(allocation == initial_allocations);
    kvm_frame_exchange_destroy(test.exchange);
}

int main(void)
{
    kvm_frame_exchange *exchange = (kvm_frame_exchange *)1;
    lib_test_assert(kvm_frame_exchange_create(LIB_NULL, 1u, LIB_NULL) ==
        LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_frame_exchange_create(&exchange, 0u, LIB_NULL) ==
        LIB_STATUS_INVALID_ARGUMENT && exchange == LIB_NULL);
    kvm_frame_exchange_destroy(LIB_NULL);
    for (fail_at = 1u; fail_at <= 5u; ++fail_at) {
        allocation = 0u;
        lib_test_assert(kvm_frame_exchange_create(&exchange, sizeof(test_frame), LIB_NULL) ==
            LIB_STATUS_NO_MEMORY);
        lib_test_assert(exchange == LIB_NULL && live_allocations == 0u);
    }
    fail_at = 0u;
    fail_mutex = LIB_TRUE;
    exchange = (kvm_frame_exchange *)1;
    lib_test_assert(kvm_frame_exchange_create(&exchange, sizeof(test_frame), LIB_NULL) ==
        LIB_STATUS_IO_ERROR);
    lib_test_assert(exchange == LIB_NULL && live_allocations == 0u);
    fail_mutex = LIB_FALSE;
    exercise();
    concurrent_exchange();
    concurrent_turnover();
    lib_test_assert(live_allocations == 0u);
    return 0;
}
