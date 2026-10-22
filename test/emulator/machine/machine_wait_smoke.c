#include "lib/types/test.h"
#include "emulator/machine/machine_interface.h"
#include "lib/base/sync_interface.h"

/* Run the real worker synchronously; only its scheduling waits are scripted. */
static emulator_machine *active;
static base_sync_wait_result wait_result;
static lib_u32 idle_waits, paused_waits, resets, stops, cleanups, facts;
static lib_u32 action;
static lib_bool heartbeat;
static emulator_machine_executor_callback executor;
static void *executor_context;
static emulator_machine_state observed[8];
static lib_bool reject_destroy;
static lib_status frame_status;
static lib_u32 pending_kind;
static lib_bool terminate_before_submit;
static lib_u32 debug_wait_action, debug_calls, debug_cancels;
static lib_u8 *debug_source;
static lib_status debug_status;
static lib_bool debug_oversize;
static base_sync_event *reject_signal, *reject_reset;
static lib_u32 joins;
static lib_status signal_event(base_sync_event *event)
{ return event == reject_signal ? LIB_STATUS_IO_ERROR : base_sync_event_signal(event); }
static lib_status reset_event(base_sync_event *event)
{ return event == reject_reset ? LIB_STATUS_IO_ERROR : base_sync_event_reset(event); }
static lib_status join_task(base_sync_task *task)
{ ++joins; return base_sync_task_join(task); }
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
{ (void)entry; (void)context; *out = LIB_NULL; return LIB_STATUS_OK; }
#define base_sync_task_create no_thread
#define base_sync_task_destroy destroy_task
#define base_sync_event_wait idle_wait
#define base_sync_wait_any paused_wait
#define base_sync_mutex_lock request_lock
#define base_sync_event_signal signal_event
#define base_sync_event_reset reset_event
#define base_sync_task_join join_task
#include "emulator/machine/machine.c"
#undef base_sync_task_join
#undef base_sync_event_reset
#undef base_sync_event_signal
#undef base_sync_mutex_lock
#undef base_sync_wait_any
#undef base_sync_event_wait
#undef base_sync_task_create
#undef base_sync_task_destroy

static void request_lock(base_sync_mutex *mutex)
{
    if (terminate_before_submit && mutex == active->request_lock) {
        terminate_before_submit = LIB_FALSE;
        emulator_machine_finish_requests(active, EMULATOR_MACHINE_ERROR);
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
        lib_test_assert(timeout == LIB_UINT32_MAX);
        if (debug_source != LIB_NULL) *debug_source = 0xffu; /* Already copied. */
        if (debug_wait_action == 1u) emulator_machine_debug_invalidate(active);
        if (debug_wait_action == 2u) emulator_machine_finish_requests(active, EMULATOR_MACHINE_ERROR);
        else if (debug_wait_action == 3u) return BASE_SYNC_WAIT_FAULT;
        else emulator_machine_service_debug(active);
        return base_sync_event_wait(event, 0u);
    }
    lib_test_assert(event == active->command_event && timeout == LIB_UINT32_MAX);
    /* This event was consumed by the paused callback after shutdown signaled it.
     * An uncancellable infinite wait here would hang instead of returning. */
    lib_test_assert(action != 5u || idle_waits == 0u);
    ++idle_waits;
    lib_test_assert(idle_waits <= 3u);
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
        if (reject_signal == events[0])
            return base_sync_wait_any(events, count, task, 0u, index);
        if (events[0] == active->debug_event)
            return idle_wait(events[0], timeout);
        lib_test_assert(events[0] == active->command_event && timeout == LIB_UINT32_MAX);
        if (action == 5u && idle_waits != 0u) {
            lib_test_assert(task == active->worker);
            lib_test_assert(base_sync_event_wait(events[0], 0u) == BASE_SYNC_WAIT_TIMED_OUT);
            ++idle_waits;
            return base_sync_wait_any(events, count, task, 0u, index);
        }
        return idle_wait(events[0], timeout);
    }
    lib_test_assert(count == 2u && events[0] == active->resume_event);
    lib_test_assert(emulator_machine_enqueue_input(active,
        &(kvm_input_event) { .type = KVM_EVENT_KEY }) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(timeout == LIB_UINT32_MAX);
    lib_test_assert(!heartbeat);
    if (action == 6u || action == 8u)
        lib_test_assert(emulator_machine_input_queue_pending(&active->input_queue));
    ++paused_waits;
    if (action == 6u) {
        lib_test_assert(paused_waits <= 2u);
        if (paused_waits == 1u) {
            lib_atomic_i32_store_explicit(&active->state_read_requested, 1,
                LIB_MEMORY_ORDER_SEQ_CST);
            *index = 1u;
        } else {
            lib_test_assert(active->state_status == LIB_STATUS_OK);
            lib_test_assert(!active->state_read_continuing);
            lib_test_assert(base_sync_event_wait(active->state_event, 0u) == BASE_SYNC_WAIT_SIGNALED);
            lib_test_assert(emulator_machine_stop(active));
            *index = 0u;
        }
        return BASE_SYNC_WAIT_SIGNALED;
    }
    if (action == 5u) {
        /* Debug woke the inner wait; shutdown then signals before its reset. */
        emulator_machine_debug_cancel(active);
        lib_test_assert(emulator_machine_stop(active));
        lib_atomic_i32_store_explicit(&active->terminate_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
        lib_test_assert(base_sync_event_signal(active->command_event) == LIB_STATUS_OK);
        lib_test_assert(base_sync_task_request_cancel(active->worker) == LIB_STATUS_OK);
        *index = 1u;
        return BASE_SYNC_WAIT_SIGNALED;
    }
    register_pending();
    lib_test_assert(paused_waits <= (action == 2u ? 2u : 1u));
    if (action == 3u) lib_test_assert(emulator_machine_reset(active));
    if (wait_result != BASE_SYNC_WAIT_SIGNALED) return wait_result;
    if (action == 0u || action == 7u) lib_test_assert(emulator_machine_resume(active));
    else if (action == 2u && resets == 1u) {
        lib_test_assert(emulator_machine_reset(active));
        lib_atomic_i32_store_explicit(&active->state_read_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    }
    else lib_test_assert(emulator_machine_stop(active));
    *index = 0u;
    return BASE_SYNC_WAIT_SIGNALED;
}

static lib_bool reset(void *context)
{
    (void)context;
    lib_test_assert(!heartbeat);
    if (++resets == 2u) {
        lib_test_assert(active->state_status == LIB_STATUS_IO_ERROR);
        lib_test_assert(!lib_atomic_i32_load_explicit(&active->state_read_requested, LIB_MEMORY_ORDER_SEQ_CST));
        lib_test_assert(base_sync_event_wait(active->state_event, 0u) == BASE_SYNC_WAIT_SIGNALED);
        base_sync_event_reset(active->state_event);
        active->state_status = LIB_STATUS_UNSUPPORTED;
    }
    return LIB_TRUE;
}
static void stop(void *context)
{ (void)context; ++stops; }
static void set_heartbeat(void *context, lib_bool enabled)
{ (void)context; heartbeat = enabled; }
static void callback(void *context, emulator_machine_executor_callback entry, void *opaque)
{
    (void)context; executor = entry; executor_context = opaque;
    if (entry == LIB_NULL) ++cleanups;
}
static lib_bool take_debug_stop(void *context)
{ (void)context; return action == 7u; }
static lib_status begin_state_read(void *context,
    const emulator_machine_state_writer *writer)
{
    (void)context; (void)writer;
    lib_test_assert(!heartbeat);
    return LIB_STATUS_OK;
}
static lib_bool take_state_read_result(void *context, lib_status *status)
{
    (void)context;
    if (!active->state_read_continuing) return LIB_FALSE;
    lib_test_assert(heartbeat);
    *status = LIB_STATUS_OK;
    return LIB_TRUE;
}
static lib_bool run(void *context)
{
    (void)context;
    lib_test_assert(heartbeat && executor != LIB_NULL);
    if (frame_status != LIB_STATUS_OK) register_pending();
    if (action == 6u || action == 8u)
        lib_test_assert(emulator_machine_enqueue_input(active,
            &(kvm_input_event) { .type = KVM_EVENT_KEY }) == LIB_STATUS_OK);
    if (action == 8u) {
        /* The driver captured before this callback; servicing completion must
         * freeze input before it can mutate the just-captured device state. */
        lib_atomic_i32_store_explicit(&active->state_read_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_store_explicit(&active->state_read_armed, 1, LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_store_explicit(&active->state_read_ready, 1, LIB_MEMORY_ORDER_SEQ_CST);
        active->state_status = LIB_STATUS_OK;
    } else if (action != 7u) lib_test_assert(emulator_machine_pause(active));
    executor(executor_context);
    if (action == 1u || action == 2u || action == 5u)
        lib_test_assert(!heartbeat); /* Stop from parking never re-enables it. */
    if (action == 6u) {
        lib_test_assert(heartbeat && active->state_read_continuing);
        lib_test_assert(emulator_machine_enqueue_input(active,
            &(kvm_input_event) { .type = KVM_EVENT_KEY }) == LIB_STATUS_INVALID_STATE);
        lib_test_assert(facts == 2u); /* Private continuation emits no RUNNING. */
        executor(executor_context);
        lib_test_assert(!heartbeat && facts == 2u); /* No duplicate PAUSED. */
    }
    if (frame_status == LIB_STATUS_OK && wait_result == BASE_SYNC_WAIT_SIGNALED &&
        (action == 0u || action == 7u)) {
        lib_test_assert(heartbeat);
        lib_test_assert(emulator_machine_stop(active));
    }
    lib_test_assert(stops != 0u);
    return LIB_TRUE; /* Unwinding the driver itself succeeds, even after fault. */
}
static void input(void *context, const kvm_input_event *event)
{ (void)context; (void)event; lib_test_assert(!"paused/capture input must remain queued"); }
static lib_status frame(void *context, kvm_window_frame_storage *value, kvm_console_text_frame *console, lib_bool *produced)
{ (void)context; (void)console; (void)value; *produced = LIB_FALSE; return frame_status; }
static void state(void *context, emulator_machine_state value, lib_u32 generation)
{
    (void)context;
    lib_test_assert(generation == (action == 4u ? 1u : resets) && facts < 8u);
    if (value == EMULATOR_MACHINE_PAUSED || value == EMULATOR_MACHINE_RESET_COMPLETED)
        lib_test_assert(!heartbeat);
    if (value == EMULATOR_MACHINE_RUNNING) lib_test_assert(heartbeat);
    if (value == EMULATOR_MACHINE_STOPPED || value == EMULATOR_MACHINE_ERROR) {
        lib_test_assert(!heartbeat && executor == LIB_NULL && cleanups == resets);
        if (pending_kind != 0u) {
            base_sync_event *event = pending_kind == 3u ? active->debug_event :
                pending_kind == 4u ? active->media_event : active->state_event;
            lib_test_assert(base_sync_event_wait(event, 0u) == BASE_SYNC_WAIT_SIGNALED);
            lib_test_assert(active->state_status == LIB_STATUS_IO_ERROR ||
                pending_kind == 3u || pending_kind == 4u);
            if (pending_kind == 3u) lib_test_assert(active->debug_status == LIB_STATUS_IO_ERROR);
            if (pending_kind == 4u) lib_test_assert(!active->media_succeeded);
            lib_test_assert(!active->state_read_continuing);
            /* Consume completion; subsequent worker disposal must not send it again. */
            lib_test_assert(base_sync_event_reset(event) == LIB_STATUS_OK);
        }
    }
    observed[facts++] = value;
}

static void completed_task(void *context, const base_sync_task *task)
{ (void)context; (void)task; }

static void cancellation_task(void *context, const base_sync_task *task)
{
    lib_test_assert(base_sync_task_wait_cancel(task, LIB_UINT32_MAX) == BASE_SYNC_WAIT_CANCELLED);
    *(lib_bool *)context = LIB_TRUE;
}

static lib_u32 media_calls;
static lib_bool replace_media(void *context, const char *path, lib_storage_medium_mode mode)
{ (void)context; (void)path; (void)mode; ++media_calls; return LIB_TRUE; }

static void check_request_failures(void)
{
    emulator_machine_driver driver = { .reset = reset, .run = run,
        .request_stop = stop, .request_wake = stop, .set_heartbeat = set_heartbeat,
        .set_executor_callback = callback, .deliver_input = input, .copy_frame = frame,
        .set_removable_media = replace_media };
    lib_test_assert(emulator_machine_create(&active, &driver) == LIB_STATUS_OK);
    lib_atomic_i32 *requests[] = { &active->debug_requested, &active->media_requested,
        &active->state_read_requested, &active->state_write_requested };
    base_sync_event *events[] = { active->debug_event, active->media_event,
        active->state_event, active->state_event };
    for (lib_u32 i = 0; i < 4u; ++i) {
        /* A previous completion must never satisfy a failed new request. */
        lib_test_assert(base_sync_event_signal(events[i]) == LIB_STATUS_OK);
        reject_reset = events[i];
        lib_test_assert(emulator_machine_submit_request(active, requests[i], events[i],
            1u << EMULATOR_MACHINE_STOPPED) == LIB_STATUS_IO_ERROR);
        lib_test_assert(!emulator_machine_take_request(active, requests[i]));
        reject_reset = LIB_NULL;
        reject_signal = active->command_event;
        lib_test_assert(emulator_machine_submit_request(active, requests[i], events[i],
            1u << EMULATOR_MACHINE_STOPPED) == LIB_STATUS_IO_ERROR);
        lib_test_assert(base_sync_event_wait(events[i], 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        lib_test_assert(!emulator_machine_take_request(active, requests[i]));
        reject_signal = LIB_NULL;
        lib_test_assert(emulator_machine_submit_request(active, requests[i], events[i],
            1u << EMULATOR_MACHINE_STOPPED) == LIB_STATUS_OK);
        lib_test_assert(emulator_machine_take_request(active, requests[i]));
        lib_test_assert(!emulator_machine_take_request(active, requests[i]));
    }
    lib_test_assert(emulator_machine_destroy(active) == LIB_STATUS_OK);
    for (lib_u32 i = 0; i < 3u; ++i) {
        lib_bool unwound = LIB_FALSE;
        lib_test_assert(emulator_machine_create(&active, &driver) == LIB_STATUS_OK);
        lib_test_assert(base_sync_task_create(cancellation_task, &unwound, &active->worker) == LIB_STATUS_OK);
        base_sync_event *completed = i == 0u ? active->debug_event :
            i == 1u ? active->media_event : active->state_event;
        reject_signal = completed;
        lib_u32 before = joins;
        if (i == 1u) {
            media_calls = 0u;
            lib_atomic_i32_store_explicit(&active->media_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
            emulator_machine_service_media(active);
            emulator_machine_service_media(active);
            lib_test_assert(media_calls == 1u);
        } else emulator_machine_complete_request(active, completed);
        lib_test_assert(emulator_machine_state_get(active) == EMULATOR_MACHINE_ERROR);
        lib_test_assert(base_sync_task_cancelled(active->worker));
        lib_test_assert(emulator_machine_wait_request(active, completed) == LIB_STATUS_IO_ERROR);
        lib_test_assert(joins == before + 1u && unwound);
        lib_test_assert(emulator_machine_submit_request(active, &active->debug_requested,
            completed, 1u << EMULATOR_MACHINE_ERROR) == LIB_STATUS_INVALID_STATE);
        emulator_machine_finish_requests(active, EMULATOR_MACHINE_STOPPED);
        lib_test_assert(emulator_machine_state_get(active) == EMULATOR_MACHINE_ERROR);
        reject_signal = LIB_NULL;
        lib_test_assert(emulator_machine_destroy(active) == LIB_STATUS_OK);
    }
    /* A dispatch reset failure must not execute a pending start or spin on
     * the still-signaled event. */
    lib_test_assert(emulator_machine_create(&active, &driver) == LIB_STATUS_OK);
    action = 1u; idle_waits = resets = 0u;
    emulator_machine_begin_cold_run(active, LIB_FALSE);
    reject_reset = active->command_event;
    emulator_machine_worker(active, LIB_NULL);
    lib_test_assert(emulator_machine_state_get(active) == EMULATOR_MACHINE_ERROR && resets == 0u);
    reject_reset = LIB_NULL;
    lib_test_assert(emulator_machine_destroy(active) == LIB_STATUS_OK);
}

static void check(base_sync_wait_result result, lib_u32 requested_action)
{
    emulator_machine_driver driver = {0};
    wait_result = result; action = requested_action;
    idle_waits = paused_waits = resets = stops = cleanups = facts = 0u;
    driver.reset = reset; driver.run = run; driver.request_stop = stop;
    driver.request_wake = stop; driver.set_heartbeat = set_heartbeat;
    driver.set_executor_callback = callback; driver.deliver_input = input;
    driver.copy_frame = frame;
    driver.take_debug_stop = take_debug_stop;
    if (action == 6u) {
        driver.begin_state_read = begin_state_read;
        driver.take_state_read_result = take_state_read_result;
    }
    lib_test_assert(emulator_machine_create(&active, &driver) == LIB_STATUS_OK);
    if (action == 5u)
        lib_test_assert(base_sync_task_create(completed_task, LIB_NULL, &active->worker) == LIB_STATUS_OK);
    active->state_status = active->debug_status = LIB_STATUS_UNSUPPORTED;
    active->media_succeeded = LIB_TRUE;
    emulator_machine_set_state_sink(active, state, LIB_NULL);
    emulator_machine_begin_cold_run(active, LIB_FALSE);
    emulator_machine_worker(active, active->worker);
    if (action == 5u) {
        lib_test_assert(idle_waits == 2u && paused_waits == 1u);
        lib_test_assert(base_sync_task_destroy(active->worker) == LIB_STATUS_OK);
        active->worker = LIB_NULL;
    }
    if (action == 4u) {
        lib_test_assert(facts == 1u && observed[0] == EMULATOR_MACHINE_ERROR);
        lib_test_assert(resets == 0u && cleanups == 0u && paused_waits == 0u);
    } else if (frame_status != LIB_STATUS_OK) {
        lib_test_assert(facts == 2u && observed[1] == EMULATOR_MACHINE_ERROR);
        lib_test_assert(paused_waits == 0u && stops == 1u && cleanups == 1u);
        lib_test_assert(active->published_frame_sequence == 0u);
    } else if (result != BASE_SYNC_WAIT_SIGNALED) {
        lib_test_assert(observed[1] == EMULATOR_MACHINE_PAUSED);
        emulator_machine_state terminal = result == BASE_SYNC_WAIT_CANCELLED ?
            EMULATOR_MACHINE_STOPPED : EMULATOR_MACHINE_ERROR;
        lib_test_assert(paused_waits == 1u && resets == 1u && cleanups == 1u);
        lib_test_assert(facts == 3u && observed[2] == terminal);
        lib_test_assert(emulator_machine_state_get(active) == terminal);
    } else if (action == 0u || action == 7u) {
        lib_test_assert(facts == 4u && observed[2] == EMULATOR_MACHINE_RUNNING);
        lib_test_assert(observed[3] == EMULATOR_MACHINE_STOPPED);
    } else if (action == 2u) {
        lib_test_assert(resets == 2u && paused_waits == 2u && facts == 4u);
        lib_test_assert(observed[2] == EMULATOR_MACHINE_RESET_COMPLETED);
        lib_test_assert(observed[3] == EMULATOR_MACHINE_STOPPED);
    } else lib_test_assert(facts == 3u && observed[2] == EMULATOR_MACHINE_STOPPED);
    if (pending_kind != 0u) {
        lib_test_assert(base_sync_event_wait(active->state_event, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        lib_test_assert(base_sync_event_wait(active->media_event, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        lib_test_assert(base_sync_event_wait(active->debug_event, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        lib_test_assert(!lib_atomic_i32_load_explicit(&active->state_read_requested, LIB_MEMORY_ORDER_SEQ_CST));
        lib_test_assert(!lib_atomic_i32_load_explicit(&active->state_read_armed, LIB_MEMORY_ORDER_SEQ_CST));
        lib_test_assert(!lib_atomic_i32_load_explicit(&active->state_read_ready, LIB_MEMORY_ORDER_SEQ_CST));
        lib_test_assert(!lib_atomic_i32_load_explicit(&active->state_write_requested, LIB_MEMORY_ORDER_SEQ_CST));
        lib_test_assert(!lib_atomic_i32_load_explicit(&active->state_write_waiting, LIB_MEMORY_ORDER_SEQ_CST));
        lib_test_assert(!lib_atomic_i32_load_explicit(&active->debug_requested, LIB_MEMORY_ORDER_SEQ_CST));
        lib_test_assert(!lib_atomic_i32_load_explicit(&active->media_requested, LIB_MEMORY_ORDER_SEQ_CST));
    } else {
        lib_test_assert(active->state_status == ((action == 6u || action == 8u)
            ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED));
        lib_test_assert(active->debug_status == LIB_STATUS_UNSUPPORTED && active->media_succeeded);
    }
    /* A dead worker rejects all registrations, even with an allowed state. */
    lib_test_assert(emulator_machine_submit_request(active, &active->state_write_requested,
        active->state_event, 1u << emulator_machine_state_get(active)) == LIB_STATUS_INVALID_STATE);
    /* Deterministically finish between the public precheck and registration. */
    lib_atomic_i32_store_explicit(&active->terminate_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&active->state, EMULATOR_MACHINE_RUNNING, LIB_MEMORY_ORDER_SEQ_CST);
    terminate_before_submit = LIB_TRUE;
    lib_test_assert(emulator_machine_submit_request(active, &active->state_read_requested,
        active->state_event, 1u << EMULATOR_MACHINE_RUNNING) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(!terminate_before_submit && !lib_atomic_i32_load_explicit(
        &active->state_read_requested, LIB_MEMORY_ORDER_SEQ_CST));
    /* A failed join must retain the worker and every referenced machine field. */
    active->worker = (base_sync_task *)1;
    reject_destroy = LIB_TRUE;
    lib_test_assert(emulator_machine_shutdown(active) == LIB_STATUS_IO_ERROR);
    lib_test_assert(emulator_machine_destroy(active) == LIB_STATUS_IO_ERROR);
    lib_test_assert(active->worker == (base_sync_task *)1 && active->frames);
    lib_test_assert(base_sync_event_signal(active->command_event) == LIB_STATUS_OK);
    reject_destroy = LIB_FALSE;
    lib_test_assert(emulator_machine_shutdown(active) == LIB_STATUS_OK && !active->worker);
    lib_test_assert(emulator_machine_destroy(active) == LIB_STATUS_OK);
}

static kvm_window_frame_storage candidate, captured;
static lib_bool candidate_ready;
static kvm_console_character_map candidate_map;
static kvm_console_text_frame *produced_console;
static lib_status produce_status(void *context, kvm_console_text_frame *console)
{
    lib_test_assert(context == active &&
        console == kvm_frame_exchange_writer(active->console_frames));
    lib_test_assert(kvm_console_frame_prepare(console, 80u,25u,80u) == LIB_STATUS_OK);
    console->base.cells[0].glyph_index = 'S';
    produced_console = console;
    return LIB_STATUS_OK;
}
static lib_status copied_frame(void *context, kvm_window_frame_storage *out,
    kvm_console_text_frame *console, lib_bool *produced)
{
    (void)context;
    lib_test_assert(console == kvm_frame_exchange_writer(active->console_frames));
    produced_console = console;
    *produced = candidate_ready;
    if (!*produced) return LIB_STATUS_OK;
    lib_status status = kvm_window_frame_validate(&candidate.frame);
    if (status != LIB_STATUS_OK) return status;
    lib_test_assert(kvm_window_frame_copy(out, &candidate.frame));
    if (!out->frame.graphics) {
        lib_test_assert(kvm_console_frame_prepare(console, candidate.data.text->base.text_columns,
            candidate.data.text->base.text_rows, candidate.data.text->base.stride) == LIB_STATUS_OK);
        kvm_text_cell *cells = console->base.cells;
        lib_size capacity = console->base.capacity;
        console->base = candidate.data.text->base;
        console->base.cells = cells; console->base.capacity = capacity;
        lib_memory_copy(cells, candidate.data.text->base.cells, capacity * sizeof(*cells));
        *console->characters = candidate_map;
    }
    return LIB_STATUS_OK;
}

static void check_publication(void)
{
    emulator_machine_driver driver = {0};
    driver.reset = reset; driver.run = run; driver.request_stop = stop;
    driver.set_heartbeat = set_heartbeat; driver.set_executor_callback = callback;
    driver.deliver_input = input; driver.copy_frame = copied_frame;
    driver.request_wake = stop;
    lib_test_assert(emulator_machine_create(&active, &driver) == LIB_STATUS_OK);
    emulator_machine_set_console_status_source(active, produce_status, active);
    emulator_machine_begin_cold_run(active, LIB_FALSE);
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK); /* Not ready. */
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 0);
    candidate_ready = LIB_TRUE;
    lib_test_assert(kvm_window_frame_prepare_graphics(&candidate, 4u, 4u, 4u) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK); /* First complete frame. */
    candidate.data.graphics->pixels[0] = 1;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK); /* A, not consumed. */
    candidate.data.graphics->pixels[15] = 2;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK); /* B retains A. */
    lib_test_assert(emulator_machine_copy_published_frame(active, &captured, emulator_machine_run_generation(active)));
    lib_test_assert(captured.frame.sequence == 3 && captured.data.graphics->pixels[0] == 1 &&
        captured.data.graphics->pixels[15] == 2);
    candidate_ready = LIB_FALSE; /* Driver reports no subsequent display change. */
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 3);
    kvm_window_frame_release(&candidate);
    candidate_ready = LIB_TRUE;
    lib_test_assert(kvm_window_frame_prepare_text(&candidate, 80u, 50u, 80u) == LIB_STATUS_OK);
    candidate.data.text->base.text_columns = candidate.data.text->base.text_rows = 1;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK); /* Produced, even if identical. */
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 5);
    candidate.data.text->base.cursor_column = 1;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 6);
    for (lib_u32 field = 0; field < 5; ++field) {
        if (field == 0) candidate.data.text->base.cells[0].foreground = 1;
        if (field == 1) candidate.data.text->base.cells[0].background = 2;
        if (field == 2) candidate.data.text->base.cells[0].glyph_bank = 1;
        if (field == 3) candidate.data.text->base.text_palette[1] = 0x123456;
        if (field == 4) candidate.data.text->base.cells[0].glyph_index = 'X';
        lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
        lib_test_assert(emulator_machine_published_frame_sequence(active) == 7 + 2 * field);
        lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
        lib_test_assert(emulator_machine_published_frame_sequence(active) == 8 + 2 * field);
    }
    candidate.data.text->base.text_columns = 80;
    candidate.data.text->base.text_rows = 50;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    candidate.data.text->base.cells[3999].glyph_index = 'Z';
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_copy_published_frame(active, &captured,
        emulator_machine_run_generation(active)));
    lib_test_assert(captured.frame.sequence == 18 &&
        captured.data.text->base.cells[3999].glyph_index == 'Z');
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 19);
    active->published_frame_sequence = LIB_UINT32_MAX - 1u;
    candidate.data.text->base.cells[0].glyph_index = 'Y';
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_published_frame_sequence(active) == LIB_UINT32_MAX);
    /* Restart invalidates publication, never the immutable input held by a
     * consumer. The old generation must not become visible to new observers. */
    const kvm_window_frame *held = kvm_frame_exchange_take(active->frames, LIB_NULL);
    lib_test_assert(held != LIB_NULL && held->sequence == LIB_UINT32_MAX);
    emulator_machine_begin_cold_run(active, LIB_FALSE);
    lib_test_assert(held->text != LIB_NULL && held->sequence == LIB_UINT32_MAX);
    kvm_frame_exchange_finish(active->frames, LIB_FALSE);
    lib_test_assert(kvm_frame_exchange_take(active->frames, LIB_NULL) == LIB_NULL);
    lib_test_assert(!emulator_machine_copy_published_frame(active, &captured, 0u));
    lib_test_assert(!emulator_machine_copy_published_frame(active, &captured,
        emulator_machine_run_generation(active)));
    lib_test_assert(captured.frame.sequence == 18u &&
        captured.data.text->base.cells[3999].glyph_index == 'Z');
    candidate_ready = LIB_FALSE;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_published_frame_sequence(active) == LIB_UINT32_MAX);
    candidate_ready = LIB_TRUE;
    lib_test_assert(kvm_window_frame_prepare_text(&candidate, 80u, 50u, 80u) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_copy_published_frame(active, &captured,
        emulator_machine_run_generation(active)));
    lib_test_assert(captured.frame.sequence == 1u); /* Restart retains the counter; wrap skips zero. */
    const kvm_console_text_frame *console = kvm_frame_exchange_take(active->console_frames, LIB_NULL);
    lib_test_assert(console == produced_console);
    lib_u32 sequence = 0u;
    lib_bool graphics = LIB_TRUE;
    lib_test_assert(emulator_machine_read_presentation(active,
        emulator_machine_run_generation(active), &sequence, &graphics));
    lib_test_assert(sequence == 1u && !graphics &&
        console->base.cells[3999].glyph_index == 'Z');
    lib_test_assert(!emulator_machine_read_presentation(active, 0u,
        &sequence, &graphics));
    lib_test_assert(sequence == 1u && console->base.cells[3999].glyph_index == 'Z');
    /* Invalid construction cannot retire a valid publication or advance serial. */
    candidate.data.text->base.text_columns = 81u;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_machine_read_presentation(active,
        emulator_machine_run_generation(active), &sequence, &graphics));
    lib_test_assert(sequence == 1u && console->base.text_columns == 80u);
    candidate.data.text->base.text_columns = 80u;
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 1u);
    /* Character-map-only changes reach the direct Console slot. A stalled
     * consumer retains its input while the producer keeps publishing. */
    candidate_map.primary[65] = 0x263au;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 2u);
    candidate_map.secondary[65] = 0x2665u;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 3u);
    lib_test_assert(console->characters->primary[65] == 0u);
    kvm_frame_exchange_finish(active->console_frames, LIB_TRUE);
    console = kvm_frame_exchange_take(active->console_frames, LIB_NULL);
    lib_test_assert(console == produced_console && console->characters->primary[65] == 0x263au &&
        console->characters->secondary[65] == 0x2665u);
    kvm_frame_exchange_finish(active->console_frames, LIB_TRUE);
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK); /* Produced unchanged text still advances. */
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 4u);
    candidate_map.primary[65] = 0xd800u;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_machine_published_frame_sequence(active) == 4u);
    candidate_map.primary[65] = 0x263au;
    kvm_window_frame_release(&candidate);
    candidate_ready = LIB_TRUE;
    lib_test_assert(kvm_window_frame_prepare_graphics(&candidate, 1u, 1u, 1u) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    console = kvm_frame_exchange_take(active->console_frames, LIB_NULL);
    lib_test_assert(console == produced_console && console->base.cells[0].glyph_index == 'S');
    kvm_frame_exchange_finish(active->console_frames, LIB_TRUE);
    candidate_ready = LIB_FALSE;
    lib_test_assert(emulator_machine_publish(active) == LIB_STATUS_OK);
    lib_test_assert(kvm_frame_exchange_take(active->console_frames, LIB_NULL) == LIB_NULL);
    lib_test_assert(emulator_machine_destroy(active) == LIB_STATUS_OK);
    kvm_window_frame_release(&candidate);
    kvm_window_frame_release(&captured);
}

/* A byte-reversal protocol unrelated to registers, segments or instruction sets.
 * Scheduling is deterministic here; emulator_machine_smoke proves native thread
 * identity through the same public rendezvous with a neutral token protocol. */
static lib_status debug_bytes(void *context, const void *request, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    const lib_u8 *source = request;
    lib_u8 *destination = response;
    (void)context;
    ++debug_calls;
    lib_test_assert(request == active->debug_request && response == active->debug_response);
    lib_test_assert(debug_source == LIB_NULL || source[0] != *debug_source);
    if (size > capacity) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_size i = 0; i < size; ++i) destination[i] = source[size - 1u - i];
    *response_size = debug_oversize ? capacity + 1u : size;
    return debug_status;
}

static void cancel_debug(void *context)
{ (void)context; ++debug_cancels; }

static void check_debug(void)
{
    emulator_machine_driver driver = { .reset = reset, .run = run,
        .request_stop = stop, .request_wake = stop, .set_heartbeat = set_heartbeat,
        .set_executor_callback = callback, .deliver_input = input, .copy_frame = frame,
        .execute_debug = debug_bytes, .cancel_debug = cancel_debug };
    emulator_machine_debug_lease lease;
    lib_u8 request[EMULATOR_MACHINE_DEBUG_REQUEST_CAPACITY] = { 1, 2, 3 };
    lib_u8 response[EMULATOR_MACHINE_DEBUG_RESPONSE_CAPACITY];
    lib_size size;
    lib_test_assert(emulator_machine_create(&active, &driver) == LIB_STATUS_OK);
    emulator_machine_begin_cold_run(active, LIB_FALSE);
    lib_atomic_i32_store_explicit(&active->state, EMULATOR_MACHINE_PAUSED, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&active->pause_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_test_assert(emulator_machine_debug_acquire(active, &lease) == LIB_STATUS_OK);
    debug_source = request;
    lib_test_assert(emulator_machine_debug_execute_with_lease(active, &lease, request, 3,
        response, sizeof(response), &size) == LIB_STATUS_OK);
    lib_test_assert(size == 3 && response[0] == 3 && response[2] == 1 && request[0] == 0xff);
    debug_source = LIB_NULL;
    lib_test_assert(emulator_machine_debug_execute_with_lease(active, &lease, request, sizeof(request),
        response, sizeof(response), &size) == LIB_STATUS_OK && size == sizeof(request));
    emulator_machine_debug_cancel(active);
    lib_test_assert(emulator_machine_debug_execute_with_lease(active, &lease, LIB_NULL, 0,
        LIB_NULL, 0, &size) == LIB_STATUS_OK && size == 0 && debug_cancels == 1);
    lib_u32 calls = debug_calls;
    const struct { const void *request; lib_size size; void *response; lib_size capacity;
        lib_status expected; } invalid[] = {
        {LIB_NULL, 1, response, 1, LIB_STATUS_INVALID_ARGUMENT},
        {request, 1, LIB_NULL, 1, LIB_STATUS_INVALID_ARGUMENT},
        {request, sizeof(request) + 1u, response, 1, LIB_STATUS_UNSUPPORTED},
        {request, 1, response, sizeof(response) + 1u, LIB_STATUS_UNSUPPORTED}
    };
    for (lib_u32 i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        size = 99; response[0] = 0x55;
        lib_test_assert(emulator_machine_debug_execute_with_lease(active, &lease,
            invalid[i].request, invalid[i].size, invalid[i].response, invalid[i].capacity,
            &size) == invalid[i].expected);
        lib_test_assert(size == 0 && response[0] == 0x55 && debug_calls == calls);
    }
    lib_test_assert(emulator_machine_debug_execute_with_lease(active, &lease, request, 1,
        response, 1, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT && debug_calls == calls);
    for (lib_u32 scenario = 0; scenario < 6; ++scenario) {
        debug_status = scenario == 0 ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
        debug_oversize = scenario == 1;
        debug_wait_action = scenario >= 3 ? scenario - 2u : 0u;
        response[0] = 0x55; size = 99;
        lib_status expected = scenario == 0 ? LIB_STATUS_UNSUPPORTED :
            (scenario == 2 ? LIB_STATUS_INVALID_ARGUMENT :
            (scenario == 3 ? LIB_STATUS_INVALID_STATE : LIB_STATUS_IO_ERROR));
        lib_test_assert(emulator_machine_debug_execute_with_lease(active, &lease, request, 3,
            response, scenario == 2 ? 2 : sizeof(response), &size) == expected);
        lib_test_assert(size == 0 && response[0] == 0x55);
        if (scenario == 3) {
            calls = debug_calls;
            lib_test_assert(emulator_machine_debug_execute_with_lease(active, &lease, request, 3,
                response, sizeof(response), &size) == LIB_STATUS_INVALID_STATE);
            lib_test_assert(debug_calls == calls);
            lib_test_assert(emulator_machine_debug_acquire(active, &lease) == LIB_STATUS_OK);
        }
        if (scenario == 4) {
            lib_test_assert(active->debug_response_size == 0);
            lib_atomic_i32_store_explicit(&active->state, EMULATOR_MACHINE_PAUSED, LIB_MEMORY_ORDER_SEQ_CST);
        }
    }
    /* Failed wait retains the copied request, never a borrowed caller buffer.
     * Script executor unwind before freeing resources; no retry is attempted. */
    lib_test_assert(lib_atomic_i32_load_explicit(&active->debug_requested, LIB_MEMORY_ORDER_SEQ_CST));
    emulator_machine_finish_requests(active, EMULATOR_MACHINE_STOPPED);
    lib_test_assert(active->debug_response_size == 0);
    lib_test_assert(emulator_machine_destroy(active) == LIB_STATUS_OK);
}

int main(void)
{
    emulator_machine_driver invalid_driver = { 0 };
    emulator_machine *output = (emulator_machine *)&invalid_driver;
    lib_test_assert(emulator_machine_create(LIB_NULL, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_machine_create(&output, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT && output == LIB_NULL);
    output = (emulator_machine *)&invalid_driver;
    lib_test_assert(emulator_machine_create(&output, &invalid_driver) == LIB_STATUS_INVALID_ARGUMENT && output == LIB_NULL);
    lib_test_assert(lib_offsetof(kvm_window_frame_storage, frame) == 0u);
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
    check(BASE_SYNC_WAIT_SIGNALED, 6u); /* Paused capture continues to a safe point. */
    check(BASE_SYNC_WAIT_SIGNALED, 7u); /* Driver debug stop, then resume. */
    check(BASE_SYNC_WAIT_SIGNALED, 8u); /* Capture completion precedes input. */
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
    check_request_failures();
    return 0;
}
