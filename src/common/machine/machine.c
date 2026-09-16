#include "common/machine/machine_interface.h"
#include "common/machine/input_queue.h"
#include "lib/base/sync_interface.h"


struct common_machine {
    common_machine_driver driver;
    common_machine_input_queue *input_queue;
    kvm_frame *frame_buffers[2];
    base_sync_mutex *frame_lock;
    int published_frame_index;
    lib_u32 published_frame_sequence;
    lib_u32 published_frame_run_generation;
    base_sync_event *command_event;
    base_sync_event *ready_event;
    base_sync_event *resume_event;
    base_sync_event *input_event;
    base_sync_event *media_event;
    base_sync_event *debug_event;
    common_machine_debug_request debug_request;
    common_machine_debug_result debug_result;
    common_machine_debug_lease debug_lease;
    lib_status debug_status;
    lib_atomic_i32 debug_requested;
    lib_atomic_i32 debug_cancel_requested;
    base_sync_task *worker;
    lib_atomic_i32 state;
    lib_atomic_i32 run_generation;
    lib_atomic_i32 debug_generation;
    lib_atomic_i32 pause_requested;
    lib_atomic_i32 stop_requested;
    lib_atomic_i32 start_requested;
    lib_atomic_i32 reset_requested;
    lib_atomic_i32 reset_active;
    lib_atomic_i32 terminate_requested;
    lib_atomic_i32 media_requested;
    common_machine_state_sink state_sink;
    void *state_context;
    common_machine_frame_sink frame_sink;
    void *frame_context;
    lib_bool media_succeeded;
    char media_path[COMMON_MACHINE_PATH_CAPACITY];
};

static void common_machine_notify_state(common_machine *machine,
    common_machine_state completed)
{
    if (machine != NULL && machine->state_sink != NULL)
        machine->state_sink(machine->state_context, completed,
            common_machine_run_generation(machine));
}

static void common_machine_debug_invalidate(common_machine *machine)
{
    if (machine != NULL) (void)lib_atomic_i32_fetch_add_explicit(&machine->debug_generation, 1, LIB_MEMORY_ORDER_SEQ_CST);
}

static void common_machine_invalidate_published_frame(common_machine *machine)
{
    if (machine == NULL) return;
    base_sync_mutex_lock(machine->frame_lock);
    lib_memory_set(machine->frame_buffers[0], 0, sizeof(*machine->frame_buffers[0]));
    lib_memory_set(machine->frame_buffers[1], 0, sizeof(*machine->frame_buffers[1]));
    machine->published_frame_index = 0;
    machine->published_frame_run_generation = 0u;
    base_sync_mutex_unlock(machine->frame_lock);
}

static lib_bool common_machine_text_frame_changed(const kvm_frame *previous,
    const kvm_frame *candidate)
{
    if (previous == NULL || previous->valid == 0u || previous->graphics != 0u)
        return LIB_TRUE;
    return previous->text_columns != candidate->text_columns ||
        previous->text_rows != candidate->text_rows ||
        previous->cursor_column != candidate->cursor_column ||
        previous->cursor_row != candidate->cursor_row ||
        previous->cursor_top != candidate->cursor_top ||
        previous->cursor_bottom != candidate->cursor_bottom ||
        previous->cursor_visible != candidate->cursor_visible ||
        previous->cursor_phase != candidate->cursor_phase ||
        previous->font_height != candidate->font_height ||
        previous->attribute_font_select != candidate->attribute_font_select ||
        lib_memory_compare(previous->text, candidate->text, sizeof(candidate->text)) != 0 ||
        lib_memory_compare(previous->attributes, candidate->attributes,
            sizeof(candidate->attributes)) != 0 ||
        lib_memory_compare(previous->text_palette, candidate->text_palette,
            sizeof(candidate->text_palette)) != 0 ||
        lib_memory_compare(previous->font, candidate->font, sizeof(candidate->font)) != 0 ||
        lib_memory_compare(previous->secondary_font, candidate->secondary_font,
            sizeof(candidate->secondary_font)) != 0;
}

static void common_machine_publish(common_machine *machine)
{
    kvm_frame *frame;
    int staging_index;
    common_machine_frame_sink sink = NULL;
    void *sink_context = NULL;
    lib_u32 sequence = 0u;
    lib_u32 generation = 0u;
    lib_bool graphics = LIB_FALSE;
    kvm_frame *published_frame = NULL;

    if (machine == NULL || machine->driver.copy_frame == NULL) return;
    base_sync_mutex_lock(machine->frame_lock);
    staging_index = machine->published_frame_index == 0 ? 1 : 0;
    frame = machine->frame_buffers[staging_index];
    if (frame == NULL || !machine->driver.copy_frame(machine->driver.context, frame)) {
        base_sync_mutex_unlock(machine->frame_lock);
        return;
    }
    if (frame->valid == 0u || (frame->graphics == 0u &&
        !common_machine_text_frame_changed(
            machine->frame_buffers[machine->published_frame_index], frame))) {
        base_sync_mutex_unlock(machine->frame_lock);
        return;
    }
    frame->sequence = ++machine->published_frame_sequence;
    generation = common_machine_run_generation(machine);
    machine->published_frame_run_generation = generation;
    machine->published_frame_index = staging_index;
    sink = machine->frame_sink;
    sink_context = machine->frame_context;
    sequence = frame->sequence;
    graphics = frame->graphics != 0u;
    published_frame = frame;
    base_sync_mutex_unlock(machine->frame_lock);
    if (machine->driver.frame_published != NULL)
        machine->driver.frame_published(machine->driver.context, published_frame);
    if (sink != NULL)
        sink(sink_context, sequence, graphics, generation);
}

static void common_machine_drain_input(common_machine *machine)
{
    kvm_input_event event;
    if (machine == NULL || machine->driver.deliver_input == NULL) return;
    if (common_machine_input_queue_pop(machine->input_queue, &event)) {
        machine->driver.deliver_input(machine->driver.context, &event);
        if (common_machine_input_queue_pending(machine->input_queue) &&
            machine->driver.request_wake != NULL) {
            /* The driver uses this wake edge to schedule another safe host
             * callback; it is not a lifecycle stop request. */
            machine->driver.request_wake(machine->driver.context);
        }
    }
}

static void common_machine_service_media(common_machine *machine)
{
    if (machine == NULL || lib_atomic_i32_exchange_explicit(&machine->media_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) == 0)
        return;
    machine->media_succeeded = machine->driver.set_removable_media != NULL &&
        machine->driver.set_removable_media(machine->driver.context,
            machine->media_path[0] == '\0' ? NULL : machine->media_path);
    base_sync_event_signal(machine->media_event);
}

/* The control thread submits one synchronous operation at a time. Only the
 * existing executor calls the driver, including while parked in PAUSED. */
static void common_machine_service_debug(common_machine *machine)
{
    if (lib_atomic_i32_exchange_explicit(&machine->debug_cancel_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0 &&
        machine->driver.cancel_debug != NULL)
        machine->driver.cancel_debug(machine->driver.context);
    if (lib_atomic_i32_exchange_explicit(&machine->debug_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) == 0) return;
    machine->debug_status = LIB_STATUS_INVALID_STATE;
    lib_memory_set(&machine->debug_result, 0, sizeof(machine->debug_result));
    if (common_machine_state_get(machine) == COMMON_MACHINE_PAUSED &&
        lib_atomic_i32_load_explicit(&machine->pause_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0 &&
        machine->debug_lease.generation == (lib_u64)(lib_u32)
            lib_atomic_i32_load_explicit(&machine->debug_generation, LIB_MEMORY_ORDER_SEQ_CST))
        machine->debug_status = machine->driver.execute_debug(
            machine->driver.context, &machine->debug_request, &machine->debug_result);
    base_sync_event_signal(machine->debug_event);
}

static void common_machine_executor_event(void *opaque)
{
    common_machine *machine = (common_machine *)opaque;
    lib_bool debug_stop = machine->driver.take_debug_stop != NULL &&
        machine->driver.take_debug_stop(machine->driver.context);
    if ((lib_atomic_i32_exchange_explicit(&machine->debug_cancel_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0 ||
         lib_atomic_i32_load_explicit(&machine->pause_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0) &&
        machine->driver.cancel_debug != NULL) {
        machine->driver.cancel_debug(machine->driver.context);
        debug_stop = LIB_FALSE;
    }
    if (debug_stop) lib_atomic_i32_exchange_explicit(&machine->pause_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    common_machine_drain_input(machine);
    common_machine_publish(machine);
    if (lib_atomic_i32_load_explicit(&machine->pause_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0 &&
        lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0) {
        lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_PAUSED, LIB_MEMORY_ORDER_SEQ_CST);
        if (lib_atomic_i32_exchange_explicit(&machine->reset_active, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0)
            common_machine_notify_state(machine, COMMON_MACHINE_RESET_COMPLETED);
        else
            common_machine_notify_state(machine, COMMON_MACHINE_PAUSED);
        while (lib_atomic_i32_load_explicit(&machine->pause_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0 &&
            lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0) {
            base_sync_event *events[3] = { machine->resume_event,
                machine->command_event, machine->input_event };
            lib_u32 index = UINT32_MAX;
            base_sync_wait_result result = base_sync_wait_any(events, 3u,
                machine->worker, UINT32_MAX, &index);
            if (result != BASE_SYNC_WAIT_SIGNALED) {
                if (result != BASE_SYNC_WAIT_CANCELLED)
                    lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_ERROR, LIB_MEMORY_ORDER_SEQ_CST);
                lib_atomic_i32_exchange_explicit(&machine->reset_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
                lib_atomic_i32_exchange_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
                machine->driver.request_stop(machine->driver.context);
                break;
            }
            if (index == 0u)
                base_sync_event_reset(machine->resume_event);
            else if (index == 1u) {
                base_sync_event_reset(machine->command_event);
                common_machine_service_media(machine);
                common_machine_service_debug(machine);
            } else {
                base_sync_event_reset(machine->input_event);
                common_machine_drain_input(machine);
            }
        }
        if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0) {
            lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_RUNNING, LIB_MEMORY_ORDER_SEQ_CST);
            common_machine_notify_state(machine, COMMON_MACHINE_RUNNING);
        }
    }
}

static void common_machine_begin_cold_run(common_machine *machine,
    lib_bool pause_after_start)
{
    common_machine_debug_invalidate(machine);
    common_machine_input_queue_clear(machine->input_queue);
    common_machine_invalidate_published_frame(machine);
    lib_atomic_i32_exchange_explicit(&machine->pause_requested, pause_after_start != 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_STARTING, LIB_MEMORY_ORDER_SEQ_CST);
    (void)lib_atomic_i32_fetch_add_explicit(&machine->run_generation, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->start_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
}

static lib_bool common_machine_schedule_cold_run(common_machine *machine,
    lib_bool pause_after_start)
{
    if (machine == NULL || machine->worker == NULL || lib_atomic_i32_load_explicit(&machine->state, LIB_MEMORY_ORDER_SEQ_CST) !=
        COMMON_MACHINE_STOPPED) return LIB_FALSE;
    base_sync_event_reset(machine->ready_event);
    common_machine_begin_cold_run(machine, pause_after_start);
    base_sync_event_signal(machine->command_event);
    return LIB_TRUE;
}

static void common_machine_worker(void *opaque, const base_sync_task *task)
{
    common_machine *machine = (common_machine *)opaque;
    for (;;) {
        lib_bool succeeded;
        if (base_sync_event_wait(machine->command_event, UINT32_MAX) !=
            BASE_SYNC_WAIT_SIGNALED || base_sync_task_cancelled(task)) break;
        base_sync_event_reset(machine->command_event);
        if (lib_atomic_i32_load_explicit(&machine->terminate_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0)
            break;
        common_machine_service_debug(machine);
        if (lib_atomic_i32_load_explicit(&machine->media_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
            common_machine_service_media(machine);
            continue;
        }
        if (lib_atomic_i32_exchange_explicit(&machine->start_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) == 0) continue;
        succeeded = machine->driver.reset(machine->driver.context);
        if (!succeeded) {
            lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_ERROR, LIB_MEMORY_ORDER_SEQ_CST);
            common_machine_notify_state(machine, COMMON_MACHINE_ERROR);
            base_sync_event_signal(machine->ready_event);
            continue;
        }
        if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
            lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_STOPPED, LIB_MEMORY_ORDER_SEQ_CST);
            common_machine_notify_state(machine, COMMON_MACHINE_STOPPED);
            base_sync_event_signal(machine->ready_event);
            continue;
        }
        machine->driver.set_executor_callback(machine->driver.context,
            common_machine_executor_event, machine);
        machine->driver.set_heartbeat(machine->driver.context, LIB_TRUE);
        lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_RUNNING, LIB_MEMORY_ORDER_SEQ_CST);
        if (lib_atomic_i32_load_explicit(&machine->reset_active, LIB_MEMORY_ORDER_SEQ_CST) == 0)
            common_machine_notify_state(machine, COMMON_MACHINE_RUNNING);
        base_sync_event_signal(machine->ready_event);
        do {
            succeeded = machine->driver.run(machine->driver.context);
        } while (succeeded &&
            lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0 &&
            lib_atomic_i32_load_explicit(&machine->terminate_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0);
        machine->driver.set_heartbeat(machine->driver.context, LIB_FALSE);
        machine->driver.set_executor_callback(machine->driver.context, NULL, NULL);
        if (machine->driver.cancel_debug != NULL)
            machine->driver.cancel_debug(machine->driver.context);
        common_machine_debug_invalidate(machine);
        /* A successful driver unwind must not erase a failed executor wait. */
        succeeded = succeeded && common_machine_state_get(machine) != COMMON_MACHINE_ERROR;
        if (succeeded && lib_atomic_i32_exchange_explicit(&machine->reset_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
            lib_atomic_i32_exchange_explicit(&machine->stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
            common_machine_begin_cold_run(machine, LIB_TRUE);
            base_sync_event_signal(machine->command_event);
            continue;
        }
        lib_atomic_i32_exchange_explicit(&machine->state, succeeded ? COMMON_MACHINE_STOPPED :
            COMMON_MACHINE_ERROR, LIB_MEMORY_ORDER_SEQ_CST);
        common_machine_notify_state(machine, succeeded ? COMMON_MACHINE_STOPPED :
            COMMON_MACHINE_ERROR);
        base_sync_event_signal(machine->ready_event);
    }
}

lib_status common_machine_create(common_machine **out_machine,
    const common_machine_driver *driver)
{
    common_machine *machine;
    lib_status status;
    if (out_machine == NULL || driver == NULL || driver->reset == NULL ||
        driver->run == NULL || driver->request_stop == NULL ||
        driver->request_wake == NULL ||
        driver->set_heartbeat == NULL || driver->set_executor_callback == NULL ||
        driver->deliver_input == NULL || driver->copy_frame == NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = NULL;
    machine = lib_allocate_zero(1u, sizeof(*machine));
    if (machine == NULL) return LIB_STATUS_NO_MEMORY;
    lib_atomic_i32_initialize(&machine->debug_requested, 0);
    lib_atomic_i32_initialize(&machine->debug_cancel_requested, 0);
    lib_atomic_i32_initialize(&machine->state, COMMON_MACHINE_STOPPED);
    lib_atomic_i32_initialize(&machine->run_generation, 0);
    lib_atomic_i32_initialize(&machine->debug_generation, 1);
    lib_atomic_i32_initialize(&machine->pause_requested, 0);
    lib_atomic_i32_initialize(&machine->stop_requested, 0);
    lib_atomic_i32_initialize(&machine->start_requested, 0);
    lib_atomic_i32_initialize(&machine->reset_requested, 0);
    lib_atomic_i32_initialize(&machine->reset_active, 0);
    lib_atomic_i32_initialize(&machine->terminate_requested, 0);
    lib_atomic_i32_initialize(&machine->media_requested, 0);
    machine->driver = *driver;
    status = base_sync_mutex_create(&machine->frame_lock);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->command_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->ready_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->resume_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->input_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->media_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->debug_event);
    if (status == LIB_STATUS_OK) status = common_machine_input_queue_create(&machine->input_queue);
    if (status == LIB_STATUS_OK) {
        machine->frame_buffers[0] = lib_allocate_zero(1u, sizeof(*machine->frame_buffers[0]));
        machine->frame_buffers[1] = lib_allocate_zero(1u, sizeof(*machine->frame_buffers[1]));
        if (machine->frame_buffers[0] == NULL || machine->frame_buffers[1] == NULL)
            status = LIB_STATUS_NO_MEMORY;
    }
    if (status == LIB_STATUS_OK)
        status = base_sync_task_create(common_machine_worker, machine, &machine->worker);
    if (status != LIB_STATUS_OK) {
        common_machine_destroy(machine);
        return status;
    }
    *out_machine = machine;
    return LIB_STATUS_OK;
}

void common_machine_set_state_sink(common_machine *machine,
    common_machine_state_sink sink, void *context)
{
    if (machine == NULL) return;
    machine->state_sink = sink;
    machine->state_context = context;
}

void common_machine_set_frame_sink(common_machine *machine,
    common_machine_frame_sink sink, void *context)
{
    if (machine == NULL) return;
    machine->frame_sink = sink;
    machine->frame_context = context;
}

lib_bool common_machine_start(common_machine *machine)
{
    return common_machine_schedule_cold_run(machine, LIB_FALSE);
}

void common_machine_debug_cancel(common_machine *machine)
{
    if (machine == NULL) return;
    lib_atomic_i32_exchange_explicit(&machine->debug_cancel_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    base_sync_event_signal(machine->command_event);
    machine->driver.request_wake(machine->driver.context);
}

lib_bool common_machine_pause(common_machine *machine)
{
    if (machine == NULL || lib_atomic_i32_load_explicit(&machine->state, LIB_MEMORY_ORDER_SEQ_CST) !=
        COMMON_MACHINE_RUNNING) return LIB_FALSE;
    lib_atomic_i32_exchange_explicit(&machine->pause_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    return LIB_TRUE;
}

lib_bool common_machine_resume(common_machine *machine)
{
    if (machine == NULL || lib_atomic_i32_load_explicit(&machine->state, LIB_MEMORY_ORDER_SEQ_CST) !=
        COMMON_MACHINE_PAUSED) return LIB_FALSE;
    common_machine_debug_invalidate(machine);
    lib_atomic_i32_exchange_explicit(&machine->pause_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    base_sync_event_signal(machine->resume_event);
    return LIB_TRUE;
}

lib_bool common_machine_stop(common_machine *machine)
{
    lib_i32 state;
    if (machine == NULL) return LIB_FALSE;
    state = lib_atomic_i32_load_explicit(&machine->state, LIB_MEMORY_ORDER_SEQ_CST);
    if (state == COMMON_MACHINE_STOPPED) return LIB_TRUE;
    if (state == COMMON_MACHINE_ERROR) return LIB_FALSE;
    common_machine_debug_invalidate(machine);
    base_sync_event_reset(machine->ready_event);
    lib_atomic_i32_exchange_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->pause_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    base_sync_event_signal(machine->resume_event);
    machine->driver.request_stop(machine->driver.context);
    return LIB_TRUE;
}

lib_bool common_machine_reset(common_machine *machine)
{
    lib_i32 state;
    if (machine == NULL) return LIB_FALSE;
    state = lib_atomic_i32_load_explicit(&machine->state, LIB_MEMORY_ORDER_SEQ_CST);
    if (state == COMMON_MACHINE_STOPPED) {
        lib_atomic_i32_exchange_explicit(&machine->reset_active, 1, LIB_MEMORY_ORDER_SEQ_CST);
        if (common_machine_schedule_cold_run(machine, LIB_TRUE)) return LIB_TRUE;
        lib_atomic_i32_exchange_explicit(&machine->reset_active, 0, LIB_MEMORY_ORDER_SEQ_CST);
        return LIB_FALSE;
    }
    if (state != COMMON_MACHINE_RUNNING && state != COMMON_MACHINE_PAUSED)
        return LIB_FALSE;
    common_machine_debug_invalidate(machine);
    lib_atomic_i32_exchange_explicit(&machine->reset_active, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->reset_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->pause_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    base_sync_event_signal(machine->resume_event);
    machine->driver.request_stop(machine->driver.context);
    return LIB_TRUE;
}

lib_bool common_machine_set_removable_media(common_machine *machine,
    const char *path)
{
    lib_size length;
    lib_i32 state;
    if (machine == NULL || machine->worker == NULL || machine->driver.set_removable_media == NULL)
        return LIB_FALSE;
    state = lib_atomic_i32_load_explicit(&machine->state, LIB_MEMORY_ORDER_SEQ_CST);
    if (state != COMMON_MACHINE_STOPPED && state != COMMON_MACHINE_PAUSED)
        return LIB_FALSE;
    if (path == NULL) machine->media_path[0] = '\0';
    else {
        length = lib_text_length(path);
        if (length >= sizeof(machine->media_path)) return LIB_FALSE;
        lib_memory_copy(machine->media_path, path, length + 1u);
    }
    base_sync_event_reset(machine->media_event);
    lib_atomic_i32_exchange_explicit(&machine->media_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    base_sync_event_signal(machine->command_event);
    return base_sync_event_wait(machine->media_event, UINT32_MAX) ==
        BASE_SYNC_WAIT_SIGNALED && machine->media_succeeded;
}

common_machine_state common_machine_state_get(const common_machine *machine)
{
    return machine == NULL ? COMMON_MACHINE_ERROR :
        (common_machine_state)lib_atomic_i32_load_explicit(&machine->state, LIB_MEMORY_ORDER_SEQ_CST);
}

lib_bool common_machine_enqueue_input(common_machine *machine,
    const kvm_input_event *event)
{
    if (machine == NULL || event == NULL ||
        common_machine_state_get(machine) != COMMON_MACHINE_RUNNING ||
        !common_machine_input_queue_push(machine->input_queue, event))
        return LIB_FALSE;
    base_sync_event_signal(machine->input_event);
    machine->driver.request_wake(machine->driver.context);
    return LIB_TRUE;
}

lib_bool common_machine_copy_published_frame(common_machine *machine,
    kvm_frame *destination, lib_u32 run_generation)
{
    lib_bool copied;
    if (machine == NULL || destination == NULL) return LIB_FALSE;
    base_sync_mutex_lock(machine->frame_lock);
    copied = machine->published_frame_run_generation == run_generation &&
        machine->frame_buffers[machine->published_frame_index]->valid != 0u;
    if (copied)
        lib_memory_copy(destination, machine->frame_buffers[machine->published_frame_index],
            sizeof(*destination));
    base_sync_mutex_unlock(machine->frame_lock);
    return copied;
}

lib_u32 common_machine_published_frame_sequence(const common_machine *machine)
{
    lib_u32 sequence;
    if (machine == NULL) return 0u;
    base_sync_mutex_lock(machine->frame_lock);
    sequence = machine->published_frame_sequence;
    base_sync_mutex_unlock(machine->frame_lock);
    return sequence;
}

lib_u32 common_machine_published_frame_run_generation(const common_machine *machine)
{
    lib_u32 generation;
    if (machine == NULL) return 0u;
    base_sync_mutex_lock(machine->frame_lock);
    generation = machine->published_frame_run_generation;
    base_sync_mutex_unlock(machine->frame_lock);
    return generation;
}

lib_u32 common_machine_run_generation(const common_machine *machine)
{
    return machine == NULL ? 0u : (lib_u32)lib_atomic_i32_load_explicit(&machine->run_generation, LIB_MEMORY_ORDER_SEQ_CST);
}

lib_status common_machine_debug_acquire(common_machine *machine,
    common_machine_debug_lease *out_lease)
{
    lib_i32 generation;

    if (machine == NULL || out_lease == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (common_machine_state_get(machine) != COMMON_MACHINE_PAUSED)
        return LIB_STATUS_INVALID_STATE;
    if (machine->driver.execute_debug == NULL) return LIB_STATUS_UNSUPPORTED;
    generation = lib_atomic_i32_load_explicit(&machine->debug_generation, LIB_MEMORY_ORDER_SEQ_CST);
    if (generation == 0) return LIB_STATUS_INVALID_STATE;
    out_lease->generation = (lib_u64)(lib_u32)generation;
    return LIB_STATUS_OK;
}

lib_status common_machine_debug_execute_with_lease(common_machine *machine,
    const common_machine_debug_lease *lease,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result)
{
    lib_i32 generation;

    if (machine == NULL || lease == NULL || request == NULL || out_result == NULL ||
        request->bytes > COMMON_MACHINE_DEBUG_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
    if (common_machine_state_get(machine) != COMMON_MACHINE_PAUSED)
        return LIB_STATUS_INVALID_STATE;
    generation = lib_atomic_i32_load_explicit(&machine->debug_generation, LIB_MEMORY_ORDER_SEQ_CST);
    if (lease->generation == 0u || lease->generation != (lib_u64)(lib_u32)generation)
        return LIB_STATUS_INVALID_STATE;
    if (machine->driver.execute_debug == NULL) return LIB_STATUS_UNSUPPORTED;
    machine->debug_request = *request;
    machine->debug_lease = *lease;
    base_sync_event_reset(machine->debug_event);
    lib_atomic_i32_exchange_explicit(&machine->debug_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    base_sync_event_signal(machine->command_event);
    if (base_sync_event_wait(machine->debug_event, UINT32_MAX) !=
        BASE_SYNC_WAIT_SIGNALED) return LIB_STATUS_IO_ERROR;
    *out_result = machine->debug_result;
    return machine->debug_status;
}

void common_machine_shutdown(common_machine *machine)
{
    if (machine == NULL || machine->worker == NULL) return;
    common_machine_debug_invalidate(machine);
    (void)common_machine_stop(machine);
    lib_atomic_i32_exchange_explicit(&machine->terminate_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    if (machine->resume_event != NULL) base_sync_event_signal(machine->resume_event);
    if (machine->command_event != NULL) base_sync_event_signal(machine->command_event);
    base_sync_task_destroy(machine->worker);
    machine->worker = NULL;
}

void common_machine_destroy(common_machine *machine)
{
    if (machine == NULL) return;
    common_machine_shutdown(machine);
    base_sync_event_destroy(machine->ready_event);
    base_sync_event_destroy(machine->resume_event);
    base_sync_event_destroy(machine->input_event);
    base_sync_event_destroy(machine->media_event);
    base_sync_event_destroy(machine->debug_event);
    common_machine_input_queue_destroy(machine->input_queue);
    base_sync_mutex_destroy(machine->frame_lock);
    lib_release(machine->frame_buffers[0]);
    lib_release(machine->frame_buffers[1]);
    base_sync_event_destroy(machine->command_event);
    lib_release(machine);
}
