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
static base_sync_wait_result paused_wait(base_sync_event *const *events,
    lib_u32 count, const base_sync_task *task, lib_u32 timeout, lib_u32 *index);
static base_sync_wait_result idle_wait(base_sync_event *event, lib_u32 timeout);
static lib_status no_thread(base_sync_task_entry entry, void *context,
    base_sync_task **out)
{ (void)entry; (void)context; *out = NULL; return LIB_STATUS_OK; }
static lib_bool not_cancelled(const base_sync_task *task)
{ (void)task; return LIB_FALSE; }
#define base_sync_task_create no_thread
#define base_sync_task_cancelled not_cancelled
#define base_sync_event_wait idle_wait
#define base_sync_wait_any paused_wait
#include "common/machine/machine.c"
#undef base_sync_wait_any
#undef base_sync_event_wait
#undef base_sync_task_cancelled
#undef base_sync_task_create

static base_sync_wait_result idle_wait(base_sync_event *event, lib_u32 timeout)
{
    assert(event == active->command_event && timeout == LIB_UINT32_MAX);
    ++idle_waits;
    assert(idle_waits <= 3u);
    if (idle_waits > 1u && !(action == 2u && resets == 1u))
        lib_atomic_i32_exchange_explicit(&active->terminate_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    return BASE_SYNC_WAIT_SIGNALED;
}

static base_sync_wait_result paused_wait(base_sync_event *const *events,
    lib_u32 count, const base_sync_task *task, lib_u32 timeout, lib_u32 *index)
{
    (void)task;
    assert(count == 3u && events[0] == active->resume_event);
    assert(timeout == LIB_UINT32_MAX);
    ++paused_waits;
    assert(paused_waits <= (action == 2u ? 2u : 1u));
    if (action == 3u) assert(common_machine_reset(active));
    if (wait_result != BASE_SYNC_WAIT_SIGNALED) return wait_result;
    if (action == 0u) assert(common_machine_resume(active));
    else if (action == 2u && resets == 1u) assert(common_machine_reset(active));
    else assert(common_machine_stop(active));
    *index = 0u;
    return BASE_SYNC_WAIT_SIGNALED;
}

static lib_bool reset(void *context)
{ (void)context; ++resets; return LIB_TRUE; }
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
    assert(common_machine_pause(active));
    executor(executor_context);
    if (wait_result == BASE_SYNC_WAIT_SIGNALED && action == 0u)
        assert(common_machine_stop(active));
    assert(stops != 0u);
    return LIB_TRUE; /* Unwinding the driver itself succeeds, even after fault. */
}
static void input(void *context, const kvm_input_event *event)
{ (void)context; (void)event; }
static lib_bool frame(void *context, kvm_frame *value)
{ (void)context; (void)value; return LIB_FALSE; }
static void state(void *context, common_machine_state value, lib_u32 generation)
{
    (void)context;
    assert(generation == resets && facts < 8u);
    if (value == COMMON_MACHINE_STOPPED || value == COMMON_MACHINE_ERROR)
        assert(!heartbeat && executor == NULL && cleanups == resets);
    observed[facts++] = value;
}

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
    common_machine_set_state_sink(active, state, NULL);
    common_machine_begin_cold_run(active, LIB_FALSE);
    common_machine_worker(active, NULL);
    assert(observed[0] == COMMON_MACHINE_RUNNING);
    assert(observed[1] == COMMON_MACHINE_PAUSED);
    if (result != BASE_SYNC_WAIT_SIGNALED) {
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
    common_machine_destroy(active);
}

int main(void)
{
    check(BASE_SYNC_WAIT_FAULT, 0u);
    check(BASE_SYNC_WAIT_INVALID_ARGUMENT, 0u);
    check(BASE_SYNC_WAIT_TIMED_OUT, 0u);
    check(BASE_SYNC_WAIT_CANCELLED, 0u);
    check(BASE_SYNC_WAIT_FAULT, 3u); /* A reset arriving at the failed wait. */
    check(BASE_SYNC_WAIT_CANCELLED, 3u);
    check(BASE_SYNC_WAIT_SIGNALED, 0u); /* Resume. */
    check(BASE_SYNC_WAIT_SIGNALED, 1u); /* Stop. */
    check(BASE_SYNC_WAIT_SIGNALED, 2u); /* Reset, then stop. */
    return 0;
}
