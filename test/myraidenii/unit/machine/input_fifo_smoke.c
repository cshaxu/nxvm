#include "lib/types/test.h"
#include "lib/base/sync_interface.h"
#include "myraidenii/machine/driver_interface.h"

typedef struct fixture {
    base_sync_event *gate, *wake, *running, *paused, *finished;
    emulator_machine_executor_callback callback;
    void *callback_context;
    lib_atomic_i32 stopping;
    char selected_order[5];
    lib_u32 selected_count;
    lib_u32 delivered;
} fixture;

static lib_bool reset(void *opaque) { (void)opaque; return LIB_TRUE; }
static void wake(void *opaque)
{ fixture *f = opaque; lib_test_assert(base_sync_event_signal(f->wake) == LIB_STATUS_OK); }
static void stop(void *opaque)
{
    fixture *f = opaque;
    lib_atomic_i32_store_explicit(&f->stopping, 1, LIB_MEMORY_ORDER_SEQ_CST);
    wake(opaque);
}
static void heartbeat(void *opaque, lib_bool enabled) { (void)opaque; (void)enabled; }
static void callback(void *opaque, emulator_machine_executor_callback entry, void *context)
{ fixture *f = opaque; f->callback = entry; f->callback_context = context; }
static lib_bool run(void *opaque)
{
    fixture *f = opaque;
    if (base_sync_event_wait(f->gate, 5000u) != BASE_SYNC_WAIT_SIGNALED) return LIB_FALSE;
    while (lib_atomic_i32_load_explicit(&f->stopping, LIB_MEMORY_ORDER_SEQ_CST) == 0) {
        if (base_sync_event_wait(f->wake, 5000u) != BASE_SYNC_WAIT_SIGNALED) return LIB_FALSE;
        if (f->callback != LIB_NULL) f->callback(f->callback_context);
    }
    return LIB_TRUE;
}
static void input(void *opaque, const kvm_input_event *event)
{
    fixture *f = opaque;
    if (event->type == KVM_EVENT_KEY && event->data.key.key == 'Z') {
        lib_test_assert(f->selected_count < 4u);
        f->selected_order[f->selected_count++] = event->data.key.pressed ? '+' : '-';
    }
    if (++f->delivered == 20u)
        lib_test_assert(base_sync_event_signal(f->finished) == LIB_STATUS_OK);
}
static lib_status frame(void *opaque, kvm_window_frame_storage *out,
    kvm_console_text_frame *console, lib_bool *produced)
{ (void)opaque; (void)out; (void)console; *produced = LIB_FALSE; return LIB_STATUS_OK; }
static void state(void *opaque, emulator_machine_state current, lib_u32 generation)
{
    fixture *f = opaque;
    (void)generation;
    if (current == EMULATOR_MACHINE_RUNNING)
        lib_test_assert(base_sync_event_signal(f->running) == LIB_STATUS_OK);
    if (current == EMULATOR_MACHINE_PAUSED)
        lib_test_assert(base_sync_event_signal(f->paused) == LIB_STATUS_OK);
}

int main(void)
{
    fixture f = {0};
    emulator_machine *machine = LIB_NULL;
    emulator_machine_driver driver = { .context = &f, .reset = reset, .run = run,
        .request_stop = stop, .request_wake = wake, .set_heartbeat = heartbeat,
        .set_executor_callback = callback, .deliver_input = input, .copy_frame = frame };
    kvm_input_event event = { .type = KVM_EVENT_KEY };
    lib_atomic_i32_initialize(&f.stopping, 0);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &f.gate) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &f.wake) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &f.running) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &f.paused) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &f.finished) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_create(&machine, &driver) == LIB_STATUS_OK);
    emulator_machine_set_state_sink(machine, state, &f);
    lib_test_assert(emulator_machine_start(machine));
    lib_test_assert(base_sync_event_wait(f.running, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    event.data.key.key = 'Z'; event.data.key.pressed = LIB_TRUE;
    lib_test_assert(emulator_machine_enqueue_input(machine, &event));
    lib_test_assert(emulator_machine_enqueue_input(machine, &event));
    lib_test_assert(emulator_machine_pause(machine));
    lib_test_assert(base_sync_event_signal(f.gate) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(f.paused, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(emulator_machine_resume(machine));
    lib_test_assert(base_sync_event_wait(f.running, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(myraidenii_machine_release_inputs(machine));
    lib_test_assert(emulator_machine_enqueue_input(machine, &event));
    lib_test_assert(base_sync_event_wait(f.finished, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(f.selected_count == 4u && lib_text_compare(f.selected_order, "++-+") == 0);
    lib_test_assert(emulator_machine_stop(machine));
    lib_test_assert(emulator_machine_shutdown(machine) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_destroy(machine) == LIB_STATUS_OK);
    base_sync_event_destroy(f.finished); base_sync_event_destroy(f.paused);
    base_sync_event_destroy(f.running); base_sync_event_destroy(f.wake);
    base_sync_event_destroy(f.gate);
    return 0;
}
