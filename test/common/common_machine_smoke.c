#include "common/machine/machine_interface.h"
#include "common/debug/debug_interface.h"

#include <windows.h>
#include <assert.h>
#include <string.h>

typedef struct machine_fake {
    HANDLE stopped;
    HANDLE state_stopped;
    HANDLE reset_completed;
    HANDLE wake;
    HANDLE running;
    HANDLE frame;
    HANDLE input;
    common_machine_executor_callback callback;
    void *callback_context;
    LONG resets;
    LONG runs;
    LONG inputs;
    LONG debug_calls;
    DWORD executor_thread;
    HANDLE callback_entered;
    HANDLE callback_release;
    LONG notifications;
} machine_fake;

static lib_bool fake_reset(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    ResetEvent(fake->stopped);
    ResetEvent(fake->wake);
    InterlockedIncrement(&fake->resets);
    return LIB_TRUE;
}

static lib_bool fake_run(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    HANDLE events[2] = { fake->stopped, fake->wake };
    InterlockedIncrement(&fake->runs);
    fake->executor_thread = GetCurrentThreadId();
    if (fake->callback != NULL) fake->callback(fake->callback_context);
    for (;;) {
        DWORD result = WaitForMultipleObjects(2u, events, FALSE, 5000u);
        if (result == WAIT_OBJECT_0) return LIB_TRUE;
        if (result != WAIT_OBJECT_0 + 1u) return LIB_FALSE;
        ResetEvent(fake->wake);
        if (fake->callback != NULL) fake->callback(fake->callback_context);
    }
}

static void fake_request_stop(void *opaque)
{ SetEvent(((machine_fake *)opaque)->stopped); }
static void fake_request_wake(void *opaque)
{ SetEvent(((machine_fake *)opaque)->wake); }
static lib_bool fake_set_media(void *opaque, const char *path)
{ (void)opaque; (void)path; return LIB_TRUE; }
static void fake_heartbeat(void *opaque, lib_bool enabled)
{ (void)opaque; (void)enabled; }
static void fake_set_callback(void *opaque, common_machine_executor_callback callback,
    void *context)
{
    machine_fake *fake = (machine_fake *)opaque;
    fake->callback = callback;
    fake->callback_context = context;
}
static void fake_deliver_input(void *opaque, const kvm_input_event *event)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)event;
    InterlockedIncrement(&fake->inputs);
    SetEvent(fake->input);
}
static lib_bool fake_copy_frame(void *opaque, kvm_frame *frame)
{
    (void)opaque;
    memset(frame, 0, sizeof(*frame));
    frame->valid = 1u;
    frame->text_columns = KVM_TEXT_COLUMNS;
    frame->text_rows = KVM_TEXT_ROWS;
    return LIB_TRUE;
}
static lib_status fake_execute_debug(void *opaque,
    const common_machine_debug_request *request,
    common_machine_debug_result *result)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (request == NULL || result == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    assert(GetCurrentThreadId() == fake->executor_thread);
    InterlockedIncrement(&fake->debug_calls);
    *result = (common_machine_debug_result) { .value = request->address };
    return LIB_STATUS_OK;
}
static void note_state(void *opaque, common_machine_state state,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)generation;
    InterlockedIncrement(&fake->notifications);
    if (state == COMMON_MACHINE_STOPPED && fake->callback_entered != NULL) {
        SetEvent(fake->callback_entered);
        assert(WaitForSingleObject(fake->callback_release, 5000u) == WAIT_OBJECT_0);
    }
    if (state == COMMON_MACHINE_RUNNING) SetEvent(fake->running);
    if (state == COMMON_MACHINE_STOPPED) SetEvent(fake->state_stopped);
    if (state == COMMON_MACHINE_RESET_COMPLETED) SetEvent(fake->reset_completed);
}
static void note_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)sequence; (void)graphics; (void)generation;
    InterlockedIncrement(&fake->notifications);
    SetEvent(fake->frame);
}

static DWORD WINAPI shutdown_machine(void *opaque)
{
    common_machine_shutdown(opaque);
    return 0;
}

/* A blocked final callback must prevent shutdown returning. A thread handle
 * is the completion barrier; no timing sleep guesses at worker quiescence. */
static void shutdown_active(common_machine *machine, machine_fake *fake)
{
    HANDLE thread;
    LONG notifications;
    fake->callback_entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->callback_release = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(fake->callback_entered && fake->callback_release);
    thread = CreateThread(NULL, 0u, shutdown_machine, machine, 0u, NULL);
    assert(thread != NULL);
    assert(WaitForSingleObject(fake->callback_entered, 5000u) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(thread, 0u) == WAIT_TIMEOUT);
    SetEvent(fake->callback_release);
    assert(WaitForSingleObject(thread, 5000u) == WAIT_OBJECT_0);
    assert(fake->callback == NULL && fake->callback_context == NULL);
    notifications = fake->notifications;
    assert(!common_machine_start(machine));
    assert(!common_machine_reset(machine));
    assert(!common_machine_set_removable_media(machine, NULL));
    common_machine_shutdown(machine);
    common_machine_destroy(machine);
    assert(fake->notifications == notifications);
    CloseHandle(thread);
    CloseHandle(fake->callback_entered); CloseHandle(fake->callback_release);
    fake->callback_entered = fake->callback_release = NULL;
}

int main(void)
{
    machine_fake fake = { 0 };
    common_machine_driver driver = { 0 };
    common_machine *machine = NULL;
    kvm_input_event input = { 0 };
    kvm_frame frame = { 0 };
    lib_u32 generation = 0u;
    common_machine_debug_lease lease = { 0 };
    common_machine_debug_result debug_result = { 0 };
    common_debug *debug = NULL;
    common_debug_result debug_command_result = { 0 };

    fake.stopped = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.state_stopped = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.reset_completed = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.wake = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.running = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.frame = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.input = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(fake.stopped != NULL && fake.state_stopped != NULL &&
        fake.reset_completed != NULL && fake.wake != NULL && fake.running != NULL &&
        fake.frame != NULL && fake.input != NULL);
    driver.context = &fake;
    driver.reset = fake_reset;
    driver.run = fake_run;
    driver.request_stop = fake_request_stop;
    driver.request_wake = fake_request_wake;
    driver.set_heartbeat = fake_heartbeat;
    driver.set_executor_callback = fake_set_callback;
    driver.deliver_input = fake_deliver_input;
    driver.copy_frame = fake_copy_frame;
    driver.set_removable_media = fake_set_media;
    driver.execute_debug = fake_execute_debug;
    assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    frame.valid = 1u;
    frame.sequence = 77u;
    assert(!common_machine_copy_published_frame(machine, &frame,
        common_machine_run_generation(machine)));
    assert(frame.valid == 1u && frame.sequence == 77u);
    common_machine_set_state_sink(machine, note_state, &fake);
    common_machine_set_frame_sink(machine, note_frame, &fake);
    assert(common_debug_create(&debug) == LIB_STATUS_OK);
    assert(common_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 0);
    assert(common_debug_submit_line(debug, "?", &debug_command_result) == LIB_STATUS_OK);
    assert(debug_command_result.keep_active && fake.debug_calls == 0);
    assert(common_debug_submit_line(debug, "h 1 2", &debug_command_result) == LIB_STATUS_OK);
    assert(strstr(debug_command_result.text, "0003") != NULL && fake.debug_calls == 0);
    assert(common_debug_submit_line(debug, "r", &debug_command_result) == LIB_STATUS_OK);
    assert(strstr(debug_command_result.text, "must be paused") != NULL && fake.debug_calls == 0);
    assert(common_machine_start(machine));
    assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(fake.frame, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.resets, 0, 0) == 1);
    generation = common_machine_run_generation(machine);
    assert(common_machine_copy_published_frame(machine, &frame, generation));
    assert(frame.valid == 1u && generation == common_machine_run_generation(machine));
    {
        lib_u32 sequence = frame.sequence;
        assert(!common_machine_copy_published_frame(machine, &frame, generation + 1u));
        assert(frame.valid == 1u && frame.sequence == sequence);
    }
    input.type = KVM_EVENT_KEY;
    input.data.key.pressed = 1u;
    assert(common_machine_enqueue_input(machine, &input));
    assert(WaitForSingleObject(fake.input, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.inputs, 0, 0) == 1);
    assert(common_debug_submit_line(debug, "d", &debug_command_result) == LIB_STATUS_OK);
    assert(strstr(debug_command_result.text, "must be paused") != NULL);
    assert(common_debug_submit_line(debug, "q", &debug_command_result) == LIB_STATUS_OK);
    assert(!debug_command_result.keep_active);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_RUNNING);
    assert(common_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 0);
    ResetEvent(fake.running);
    assert(common_machine_reset(machine));
    assert(WaitForSingleObject(fake.reset_completed, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.resets, 0, 0) == 2);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease,
        &(common_machine_debug_request) {
            .operation = COMMON_MACHINE_DEBUG_READ_REGISTER,
            .address = 0x1234u }, &debug_result) == LIB_STATUS_OK);
    assert(debug_result.value == 0x1234u &&
        InterlockedCompareExchange(&fake.debug_calls, 0, 0) == 1);
    assert(common_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 1);
    assert(common_debug_submit_line(debug, "?", &debug_command_result) ==
        LIB_STATUS_OK);
    assert(strstr(debug_command_result.text, "assemble") != NULL);
    common_debug_close(debug);
    common_debug_destroy(debug);
    assert(common_machine_resume(machine));
    assert(common_machine_debug_execute_with_lease(machine, &lease,
        &(common_machine_debug_request) { 0 }, &debug_result) ==
        LIB_STATUS_INVALID_STATE);
    assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
    assert(common_machine_stop(machine));
    assert(WaitForSingleObject(fake.state_stopped, 5000u) == WAIT_OBJECT_0);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_STOPPED);
    {
        lib_u32 sequence = frame.sequence;
        assert(!common_machine_copy_published_frame(machine, &frame, generation));
        assert(frame.valid == 1u && frame.sequence == sequence);
    }
    common_machine_shutdown(machine);
    common_machine_shutdown(machine);
    common_machine_destroy(machine);
    /* A never-started worker uses the same terminal path. */
    assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    common_machine_shutdown(machine);
    assert(!common_machine_start(machine));
    common_machine_destroy(machine);
    for (int paused = 0; paused != 2; ++paused) {
        ResetEvent(fake.running); ResetEvent(fake.reset_completed);
        assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
        common_machine_set_state_sink(machine, note_state, &fake);
        common_machine_set_frame_sink(machine, note_frame, &fake);
        assert(paused ? common_machine_reset(machine) : common_machine_start(machine));
        assert(WaitForSingleObject(paused ? fake.reset_completed : fake.running,
            5000u) == WAIT_OBJECT_0);
        shutdown_active(machine, &fake);
    }
    common_machine_shutdown(NULL);
    CloseHandle(fake.input); CloseHandle(fake.frame); CloseHandle(fake.running);
    CloseHandle(fake.wake); CloseHandle(fake.reset_completed);
    CloseHandle(fake.state_stopped); CloseHandle(fake.stopped);
    return 0;
}
