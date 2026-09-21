#include "common/machine/machine_interface.h"
#include "lib/base/sync_interface.h"
#include <assert.h>

/* Run the real worker synchronously; only its scheduling waits are scripted. */
static common_machine *active;
static base_sync_wait_result wait_result;
static unsigned idle_waits, paused_waits, resets, stops, cleanups, facts;
static unsigned action;
static lib_bool heartbeat;
static common_machine_executor_callback executor;
static void *executor_context;
static common_machine_state observed[8];
static lib_bool reject_destroy;
static lib_status frame_status;
static unsigned pending_kind;
static lib_bool terminate_before_submit;
static unsigned debug_wait_action, debug_calls, debug_cancels;
static lib_u8 *debug_source;
static lib_status debug_status;
static lib_bool debug_oversize;
static void request_lock(base_sync_mutex *mutex);
static void register_pending(void);
static lib_status destroy_task(base_sync_task *task)
{
    if (reject_destroy) return LIB_STATUS_IO_ERROR;
    return task == (base_sync_task *)1 ? LIB_STATUS_OK : base_sync_task_destroy(task);
}
static base_sync_wait_result paused_wait(base_sync_event *const *events,
    lib_u32 count, const base_sync_task *task, lib_u32 timeout, lib_u32 *index);
static base_sync_wait_result idle_wait(base_sync_event *event, lib_u32 timeout);
static lib_status no_thread(base_sync_task_entry entry, void *context,
    base_sync_task **out)
{ (void)entry; (void)context; *out = NULL; return LIB_STATUS_OK; }
#define base_sync_task_create no_thread
#define base_sync_task_destroy destroy_task
#define base_sync_event_wait idle_wait
#define base_sync_wait_any paused_wait
#define base_sync_mutex_lock request_lock
#include "common/machine/machine.c"
#undef base_sync_mutex_lock
#undef base_sync_wait_any
#undef base_sync_event_wait
#undef base_sync_task_create
#undef base_sync_task_destroy

static void request_lock(base_sync_mutex *mutex)
{
    if (terminate_before_submit && mutex == active->request_lock) {
        terminate_before_submit = LIB_FALSE;
        common_machine_finish_requests(active, COMMON_MACHINE_ERROR);
    }
    base_sync_mutex_lock(mutex);
}

static void register_pending(void)
{
    switch (pending_kind) {
    case 1:
        lib_atomic_i32_store_explicit(&active->state_read_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_store_explicit(&active->state_read_armed, 1, LIB_MEMORY_ORDER_SEQ_CST);
        active->state_read_continuing = LIB_TRUE;
        break;
    case 2:
        lib_atomic_i32_store_explicit(&active->state_write_waiting, 1, LIB_MEMORY_ORDER_SEQ_CST);
        break;
    case 3:
        lib_atomic_i32_store_explicit(&active->debug_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
        break;
    case 4:
        lib_atomic_i32_store_explicit(&active->media_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
        break;
    case 5:
        lib_atomic_i32_store_explicit(&active->state_write_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
        break;
    }
}

static base_sync_wait_result idle_wait(base_sync_event *event, lib_u32 timeout)
{
    if (event == active->debug_event) {
        assert(timeout == LIB_UINT32_MAX);
        if (debug_source != NULL) *debug_source = 0xffu; /* Already copied. */
        if (debug_wait_action == 1u) common_machine_debug_invalidate(active);
        if (debug_wait_action == 2u) common_machine_finish_requests(active, COMMON_MACHINE_ERROR);
        else if (debug_wait_action == 3u) return BASE_SYNC_WAIT_FAULT;
        else common_machine_service_debug(active);
        return base_sync_event_wait(event, 0u);
    }
    assert(event == active->command_event && timeout == LIB_UINT32_MAX);
    /* This event was consumed by the paused callback after shutdown signaled it.
     * An uncancellable infinite wait here would hang instead of returning. */
    assert(action != 5u || idle_waits == 0u);
    ++idle_waits;
    assert(idle_waits <= 3u);
    if (action == 4u) {
        register_pending();
        return BASE_SYNC_WAIT_FAULT;
    }
    if (idle_waits > 1u && !(action == 2u && resets == 1u))
        lib_atomic_i32_exchange_explicit(&active->terminate_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    return BASE_SYNC_WAIT_SIGNALED;
}

static base_sync_wait_result paused_wait(base_sync_event *const *events,
    lib_u32 count, const base_sync_task *task, lib_u32 timeout, lib_u32 *index)
{
    if (count == 1u) {
        assert(events[0] == active->command_event && timeout == LIB_UINT32_MAX);
        if (action == 5u && idle_waits != 0u) {
            assert(task == active->worker);
            assert(base_sync_event_wait(events[0], 0u) == BASE_SYNC_WAIT_TIMED_OUT);
            ++idle_waits;
            return base_sync_wait_any(events, count, task, 0u, index);
        }
        return idle_wait(events[0], timeout);
    }
    assert(count == 3u && events[0] == active->resume_event);
    assert(timeout == LIB_UINT32_MAX);
    ++paused_waits;
    if (action == 5u) {
        /* Debug woke the inner wait; shutdown then signals before its reset. */
        common_machine_debug_cancel(active);
        assert(common_machine_stop(active));
        lib_atomic_i32_store_explicit(&active->terminate_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
        assert(base_sync_event_signal(active->command_event) == LIB_STATUS_OK);
        assert(base_sync_task_request_cancel(active->worker) == LIB_STATUS_OK);
        *index = 1u;
        return BASE_SYNC_WAIT_SIGNALED;
    }
    register_pending();
    assert(paused_waits <= (action == 2u ? 2u : 1u));
    if (action == 3u) assert(common_machine_reset(active));
    if (wait_result != BASE_SYNC_WAIT_SIGNALED) return wait_result;
    if (action == 0u) assert(common_machine_resume(active));
    else if (action == 2u && resets == 1u) {
        assert(common_machine_reset(active));
        lib_atomic_i32_store_explicit(&active->state_read_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    }
    else assert(common_machine_stop(active));
    *index = 0u;
    return BASE_SYNC_WAIT_SIGNALED;
}

static lib_bool reset(void *context)
{
    (void)context;
    if (++resets == 2u) {
        assert(active->state_status == LIB_STATUS_IO_ERROR);
        assert(!lib_atomic_i32_load_explicit(&active->state_read_requested, LIB_MEMORY_ORDER_SEQ_CST));
        assert(base_sync_event_wait(active->state_event, 0u) == BASE_SYNC_WAIT_SIGNALED);
        base_sync_event_reset(active->state_event);
        active->state_status = LIB_STATUS_UNSUPPORTED;
    }
    return LIB_TRUE;
}
static void stop(void *context)
{ (void)context; ++stops; }
static void set_heartbeat(void *context, lib_bool enabled)
{ (void)context; heartbeat = enabled; }
static void callback(void *context, common_machine_executor_callback entry, void *opaque)
{
    (void)context; executor = entry; executor_context = opaque;
    if (entry == NULL) ++cleanups;
}
static lib_bool run(void *context)
{
    (void)context;
    assert(heartbeat && executor != NULL);
    if (frame_status != LIB_STATUS_OK) register_pending();
    assert(common_machine_pause(active));
    executor(executor_context);
    if (frame_status == LIB_STATUS_OK && wait_result == BASE_SYNC_WAIT_SIGNALED && action == 0u)
        assert(common_machine_stop(active));
    assert(stops != 0u);
    return LIB_TRUE; /* Unwinding the driver itself succeeds, even after fault. */
}
static void input(void *context, const kvm_input_event *event)
{ (void)context; (void)event; }
static lib_status frame(void *context, common_machine_frame *value)
{ (void)context; value->window.valid = 0u; return frame_status; }
static void state(void *context, common_machine_state value, lib_u32 generation)
{
    (void)context;
    assert(generation == (action == 4u ? 1u : resets) && facts < 8u);
    if (value == COMMON_MACHINE_STOPPED || value == COMMON_MACHINE_ERROR) {
        assert(!heartbeat && executor == NULL && cleanups == resets);
        if (pending_kind != 0u) {
            base_sync_event *event = pending_kind == 3u ? active->debug_event :
                pending_kind == 4u ? active->media_event : active->state_event;
            assert(base_sync_event_wait(event, 0u) == BASE_SYNC_WAIT_SIGNALED);
            assert(active->state_status == LIB_STATUS_IO_ERROR ||
                pending_kind == 3u || pending_kind == 4u);
            if (pending_kind == 3u) assert(active->debug_status == LIB_STATUS_IO_ERROR);
            if (pending_kind == 4u) assert(!active->media_succeeded);
            assert(!active->state_read_continuing);
            /* Consume completion; subsequent worker disposal must not send it again. */
            assert(base_sync_event_reset(event) == LIB_STATUS_OK);
        }
    }
    observed[facts++] = value;
}

static void completed_task(void *context, const base_sync_task *task)
{ (void)context; (void)task; }

static void check(base_sync_wait_result result, unsigned requested_action)
{
    common_machine_driver driver = {0};
    wait_result = result; action = requested_action;
    idle_waits = paused_waits = resets = stops = cleanups = facts = 0u;
    driver.reset = reset; driver.run = run; driver.request_stop = stop;
    driver.request_wake = stop; driver.set_heartbeat = set_heartbeat;
    driver.set_executor_callback = callback; driver.deliver_input = input;
    driver.copy_frame = frame;
    assert(common_machine_create(&active, &driver) == LIB_STATUS_OK);
    if (action == 5u)
        assert(base_sync_task_create(completed_task, NULL, &active->worker) == LIB_STATUS_OK);
    active->state_status = active->debug_status = LIB_STATUS_UNSUPPORTED;
    active->media_succeeded = LIB_TRUE;
    common_machine_set_state_sink(active, state, NULL);
    common_machine_begin_cold_run(active, LIB_FALSE);
    common_machine_worker(active, active->worker);
    if (action == 5u) {
        assert(idle_waits == 2u && paused_waits == 1u);
        assert(base_sync_task_destroy(active->worker) == LIB_STATUS_OK);
        active->worker = NULL;
    }
    if (action == 4u) {
        assert(facts == 1u && observed[0] == COMMON_MACHINE_ERROR);
        assert(resets == 0u && cleanups == 0u && paused_waits == 0u);
    } else if (frame_status != LIB_STATUS_OK) {
        assert(facts == 2u && observed[1] == COMMON_MACHINE_ERROR);
        assert(paused_waits == 0u && stops == 1u && cleanups == 1u);
        assert(active->published_frame_sequence == 0u);
    } else if (result != BASE_SYNC_WAIT_SIGNALED) {
        assert(observed[1] == COMMON_MACHINE_PAUSED);
        common_machine_state terminal = result == BASE_SYNC_WAIT_CANCELLED ?
            COMMON_MACHINE_STOPPED : COMMON_MACHINE_ERROR;
        assert(paused_waits == 1u && resets == 1u && cleanups == 1u);
        assert(facts == 3u && observed[2] == terminal);
        assert(common_machine_state_get(active) == terminal);
    } else if (action == 0u) {
        assert(facts == 4u && observed[2] == COMMON_MACHINE_RUNNING);
        assert(observed[3] == COMMON_MACHINE_STOPPED);
    } else if (action == 2u) {
        assert(resets == 2u && paused_waits == 2u && facts == 4u);
        assert(observed[2] == COMMON_MACHINE_RESET_COMPLETED);
        assert(observed[3] == COMMON_MACHINE_STOPPED);
    } else assert(facts == 3u && observed[2] == COMMON_MACHINE_STOPPED);
    if (pending_kind != 0u) {
        assert(base_sync_event_wait(active->state_event, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        assert(base_sync_event_wait(active->media_event, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        assert(base_sync_event_wait(active->debug_event, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        assert(!lib_atomic_i32_load_explicit(&active->state_read_requested, LIB_MEMORY_ORDER_SEQ_CST));
        assert(!lib_atomic_i32_load_explicit(&active->state_read_armed, LIB_MEMORY_ORDER_SEQ_CST));
        assert(!lib_atomic_i32_load_explicit(&active->state_read_ready, LIB_MEMORY_ORDER_SEQ_CST));
        assert(!lib_atomic_i32_load_explicit(&active->state_write_requested, LIB_MEMORY_ORDER_SEQ_CST));
        assert(!lib_atomic_i32_load_explicit(&active->state_write_waiting, LIB_MEMORY_ORDER_SEQ_CST));
        assert(!lib_atomic_i32_load_explicit(&active->debug_requested, LIB_MEMORY_ORDER_SEQ_CST));
        assert(!lib_atomic_i32_load_explicit(&active->media_requested, LIB_MEMORY_ORDER_SEQ_CST));
    } else {
        assert(active->state_status == LIB_STATUS_UNSUPPORTED);
        assert(active->debug_status == LIB_STATUS_UNSUPPORTED && active->media_succeeded);
    }
    /* A dead worker rejects all registrations, even with an allowed state. */
    assert(!common_machine_submit_request(active, &active->state_write_requested,
        active->state_event, 1u << common_machine_state_get(active)));
    /* Deterministically finish between the public precheck and registration. */
    lib_atomic_i32_store_explicit(&active->terminate_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&active->state, COMMON_MACHINE_RUNNING, LIB_MEMORY_ORDER_SEQ_CST);
    terminate_before_submit = LIB_TRUE;
    assert(!common_machine_submit_request(active, &active->state_read_requested,
        active->state_event, 1u << COMMON_MACHINE_RUNNING));
    assert(!terminate_before_submit && !lib_atomic_i32_load_explicit(
        &active->state_read_requested, LIB_MEMORY_ORDER_SEQ_CST));
    /* A failed join must retain the worker and every referenced machine field. */
    active->worker = (base_sync_task *)1;
    reject_destroy = LIB_TRUE;
    assert(common_machine_shutdown(active) == LIB_STATUS_IO_ERROR);
    assert(common_machine_destroy(active) == LIB_STATUS_IO_ERROR);
    assert(active->worker == (base_sync_task *)1 && active->frame_buffers[0]);
    assert(base_sync_event_signal(active->command_event) == LIB_STATUS_OK);
    reject_destroy = LIB_FALSE;
    assert(common_machine_shutdown(active) == LIB_STATUS_OK && !active->worker);
    assert(common_machine_destroy(active) == LIB_STATUS_OK);
}

static common_machine_frame candidate, captured;
static lib_status copied_frame(void *context, common_machine_frame *out)
{
    (void)context;
    *out = candidate;
    return LIB_STATUS_OK;
}

static void check_publication(void)
{
    common_machine_driver driver = {0};
    driver.reset = reset; driver.run = run; driver.request_stop = stop;
    driver.set_heartbeat = set_heartbeat; driver.set_executor_callback = callback;
    driver.deliver_input = input; driver.copy_frame = copied_frame;
    driver.request_wake = stop;
    assert(common_machine_create(&active, &driver) == LIB_STATUS_OK);
    common_machine_begin_cold_run(active, LIB_FALSE);
    assert(common_machine_publish(active) == LIB_STATUS_OK); /* Not ready. */
    assert(common_machine_published_frame_sequence(active) == 0);
    candidate.window.valid = candidate.window.graphics = 1;
    candidate.window.image.width = candidate.window.image.stride = 4;
    candidate.window.image.height = 4;
    assert(common_machine_publish(active) == LIB_STATUS_OK); /* First complete frame. */
    candidate.window.image.pixels[0] = 1;
    assert(common_machine_publish(active) == LIB_STATUS_OK); /* A, not consumed. */
    candidate.window.image.pixels[15] = 2;
    assert(common_machine_publish(active) == LIB_STATUS_OK); /* B retains A. */
    assert(common_machine_copy_published_frame(active, &captured, common_machine_run_generation(active)));
    assert(captured.sequence == 3 && captured.window.image.pixels[0] == 1 &&
        captured.window.image.pixels[15] == 2);
    candidate.window.valid = 0; /* Driver reports no subsequent display change. */
    assert(common_machine_publish(active) == LIB_STATUS_OK);
    assert(common_machine_published_frame_sequence(active) == 3);
    candidate = (common_machine_frame){0};
    candidate.window.valid = 1;
    candidate.window.text.base.text_columns = candidate.window.text.base.text_rows = 1;
    assert(common_machine_publish(active) == LIB_STATUS_OK);
    assert(common_machine_publish(active) == LIB_STATUS_OK); /* Unchanged text suppressed. */
    assert(common_machine_published_frame_sequence(active) == 4);
    candidate.window.text.base.cursor_column = 1;
    assert(common_machine_publish(active) == LIB_STATUS_OK);
    assert(common_machine_published_frame_sequence(active) == 5);
    for (unsigned field = 0; field < 5; ++field) {
        if (field == 0) candidate.window.text.base.cells[0].foreground = 1;
        if (field == 1) candidate.window.text.base.cells[0].background = 2;
        if (field == 2) candidate.window.text.base.cells[0].glyph_bank = 1;
        if (field == 3) candidate.window.text.base.text_palette[1] = 0x123456;
        if (field == 4) candidate.window.text.base.cells[0].glyph_index = 'X';
        assert(common_machine_publish(active) == LIB_STATUS_OK);
        assert(common_machine_published_frame_sequence(active) == 6 + field);
        assert(common_machine_publish(active) == LIB_STATUS_OK);
        assert(common_machine_published_frame_sequence(active) == 6 + field);
    }
    assert(common_machine_destroy(active) == LIB_STATUS_OK);
}

/* A byte-reversal protocol unrelated to registers, segments or instruction sets.
 * Scheduling is deterministic here; common_machine_smoke proves native thread
 * identity through the same public rendezvous with a neutral token protocol. */
static lib_status debug_bytes(void *context, const void *request, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    const lib_u8 *source = request;
    lib_u8 *destination = response;
    (void)context;
    ++debug_calls;
    assert(request == active->debug_request && response == active->debug_response);
    assert(debug_source == NULL || source[0] != *debug_source);
    if (size > capacity) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_size i = 0; i < size; ++i) destination[i] = source[size - 1u - i];
    *response_size = debug_oversize ? capacity + 1u : size;
    return debug_status;
}

static void cancel_debug(void *context)
{ (void)context; ++debug_cancels; }

static void check_debug(void)
{
    common_machine_driver driver = { .reset = reset, .run = run,
        .request_stop = stop, .request_wake = stop, .set_heartbeat = set_heartbeat,
        .set_executor_callback = callback, .deliver_input = input, .copy_frame = frame,
        .execute_debug = debug_bytes, .cancel_debug = cancel_debug };
    common_machine_debug_lease lease;
    lib_u8 request[COMMON_MACHINE_DEBUG_REQUEST_CAPACITY] = { 1, 2, 3 };
    lib_u8 response[COMMON_MACHINE_DEBUG_RESPONSE_CAPACITY];
    lib_size size;
    assert(common_machine_create(&active, &driver) == LIB_STATUS_OK);
    common_machine_begin_cold_run(active, LIB_FALSE);
    lib_atomic_i32_store_explicit(&active->state, COMMON_MACHINE_PAUSED, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&active->pause_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    assert(common_machine_debug_acquire(active, &lease) == LIB_STATUS_OK);
    debug_source = request;
    assert(common_machine_debug_execute_with_lease(active, &lease, request, 3,
        response, sizeof(response), &size) == LIB_STATUS_OK);
    assert(size == 3 && response[0] == 3 && response[2] == 1 && request[0] == 0xff);
    debug_source = NULL;
    assert(common_machine_debug_execute_with_lease(active, &lease, request, sizeof(request),
        response, sizeof(response), &size) == LIB_STATUS_OK && size == sizeof(request));
    common_machine_debug_cancel(active);
    assert(common_machine_debug_execute_with_lease(active, &lease, NULL, 0,
        NULL, 0, &size) == LIB_STATUS_OK && size == 0 && debug_cancels == 1);
    unsigned calls = debug_calls;
    const struct { const void *request; lib_size size; void *response; lib_size capacity;
        lib_status expected; } invalid[] = {
        {NULL, 1, response, 1, LIB_STATUS_INVALID_ARGUMENT},
        {request, 1, NULL, 1, LIB_STATUS_INVALID_ARGUMENT},
        {request, sizeof(request) + 1u, response, 1, LIB_STATUS_UNSUPPORTED},
        {request, 1, response, sizeof(response) + 1u, LIB_STATUS_UNSUPPORTED}
    };
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        size = 99; response[0] = 0x55;
        assert(common_machine_debug_execute_with_lease(active, &lease,
            invalid[i].request, invalid[i].size, invalid[i].response, invalid[i].capacity,
            &size) == invalid[i].expected);
        assert(size == 0 && response[0] == 0x55 && debug_calls == calls);
    }
    assert(common_machine_debug_execute_with_lease(active, &lease, request, 1,
        response, 1, NULL) == LIB_STATUS_INVALID_ARGUMENT && debug_calls == calls);
    for (unsigned scenario = 0; scenario < 6; ++scenario) {
        debug_status = scenario == 0 ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
        debug_oversize = scenario == 1;
        debug_wait_action = scenario >= 3 ? scenario - 2u : 0u;
        response[0] = 0x55; size = 99;
        lib_status expected = scenario == 0 ? LIB_STATUS_UNSUPPORTED :
            (scenario == 2 ? LIB_STATUS_INVALID_ARGUMENT :
            (scenario == 3 ? LIB_STATUS_INVALID_STATE : LIB_STATUS_IO_ERROR));
        assert(common_machine_debug_execute_with_lease(active, &lease, request, 3,
            response, scenario == 2 ? 2 : sizeof(response), &size) == expected);
        assert(size == 0 && response[0] == 0x55);
        if (scenario == 3) {
            calls = debug_calls;
            assert(common_machine_debug_execute_with_lease(active, &lease, request, 3,
                response, sizeof(response), &size) == LIB_STATUS_INVALID_STATE);
            assert(debug_calls == calls);
            assert(common_machine_debug_acquire(active, &lease) == LIB_STATUS_OK);
        }
        if (scenario == 4) {
            assert(active->debug_response_size == 0);
            lib_atomic_i32_store_explicit(&active->state, COMMON_MACHINE_PAUSED, LIB_MEMORY_ORDER_SEQ_CST);
        }
    }
    /* Failed wait retains the copied request, never a borrowed caller buffer.
     * Script executor unwind before freeing resources; no retry is attempted. */
    assert(lib_atomic_i32_load_explicit(&active->debug_requested, LIB_MEMORY_ORDER_SEQ_CST));
    common_machine_finish_requests(active, COMMON_MACHINE_STOPPED);
    assert(active->debug_response_size == 0);
    assert(common_machine_destroy(active) == LIB_STATUS_OK);
}

int main(void)
{
    common_machine_driver invalid_driver = { 0 };
    common_machine *output = (common_machine *)&invalid_driver;
    assert(common_machine_create(NULL, NULL) == LIB_STATUS_INVALID_ARGUMENT);
    assert(common_machine_create(&output, NULL) == LIB_STATUS_INVALID_ARGUMENT && output == NULL);
    output = (common_machine *)&invalid_driver;
    assert(common_machine_create(&output, &invalid_driver) == LIB_STATUS_INVALID_ARGUMENT && output == NULL);
    assert(sizeof(common_machine_frame) == 985112);
    /* Resource-only changes are publications, not just character changes. */
    static common_machine_frame before, after;
    before.window.valid = 1u;
    before.window.text.base.text_columns = 80u;
    before.window.text.base.text_rows = 25u;
    after = before;
    assert(!common_machine_text_frame_changed(&before, &after));
    after.characters.primary[65] = 0x263au;
    assert(common_machine_text_frame_changed(&before, &after));
    before = after;
    after.characters.secondary[65] = 0x2665u;
    assert(common_machine_text_frame_changed(&before, &after));
    before = after;
    after.window.text.font[65 * 16] = 0xffu;
    assert(common_machine_text_frame_changed(&before, &after));
    before = after;
    after.window.text.secondary_font[65 * 16] = 0x81u;
    assert(common_machine_text_frame_changed(&before, &after));
    check(BASE_SYNC_WAIT_FAULT, 0u);
    check(BASE_SYNC_WAIT_INVALID_ARGUMENT, 0u);
    check(BASE_SYNC_WAIT_TIMED_OUT, 0u);
    check(BASE_SYNC_WAIT_CANCELLED, 0u);
    check(BASE_SYNC_WAIT_FAULT, 3u); /* A reset arriving at the failed wait. */
    check(BASE_SYNC_WAIT_CANCELLED, 3u);
    check(BASE_SYNC_WAIT_SIGNALED, 0u); /* Resume. */
    check(BASE_SYNC_WAIT_SIGNALED, 1u); /* Stop. */
    check(BASE_SYNC_WAIT_SIGNALED, 2u); /* Reset, then stop. */
    check(BASE_SYNC_WAIT_SIGNALED, 5u); /* Debug-close/shutdown loses command wake. */
    frame_status = LIB_STATUS_UNSUPPORTED;
    check(BASE_SYNC_WAIT_SIGNALED, 0u);
    frame_status = LIB_STATUS_INVALID_ARGUMENT;
    check(BASE_SYNC_WAIT_SIGNALED, 0u);
    for (pending_kind = 1u; pending_kind <= 5u; ++pending_kind) {
        frame_status = LIB_STATUS_IO_ERROR;
        check(BASE_SYNC_WAIT_SIGNALED, 0u);
        frame_status = LIB_STATUS_OK;
        check(BASE_SYNC_WAIT_FAULT, 0u);
        check(BASE_SYNC_WAIT_CANCELLED, 0u);
        check(BASE_SYNC_WAIT_FAULT, 4u);
    }
    check_publication();
    check_debug();
    return 0;
}
