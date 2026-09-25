#include "common/machine/machine_interface.h"
#include "common/machine/input_queue.h"
#include "lib/base/sync_interface.h"


struct common_machine {
    common_machine_driver driver;
    common_machine_input_queue input_queue;
    common_machine_frame *frame_buffers[2];
    base_sync_mutex *frame_lock;
    base_sync_mutex *request_lock;
    lib_i32 published_frame_index;
    lib_u32 published_frame_sequence;
    lib_u32 published_frame_run_generation;
    base_sync_event *command_event;
    base_sync_event *resume_event;
    base_sync_event *input_event;
    base_sync_event *media_event;
    base_sync_event *debug_event;
    base_sync_event *state_event;
    lib_u8 debug_request[COMMON_MACHINE_DEBUG_REQUEST_CAPACITY];
    lib_u8 debug_response[COMMON_MACHINE_DEBUG_RESPONSE_CAPACITY];
    lib_size debug_request_size, debug_response_capacity, debug_response_size;
    common_machine_debug_lease debug_lease;
    lib_status debug_status;
    lib_atomic_i32 debug_requested;
    lib_atomic_i32 debug_cancel_requested;
    base_sync_task *worker;
    lib_atomic_i32 state;
    lib_atomic_u32 run_generation;
    lib_atomic_i32 debug_generation;
    lib_atomic_i32 pause_requested;
    lib_atomic_i32 stop_requested;
    lib_atomic_i32 start_requested;
    lib_atomic_i32 reset_requested;
    lib_atomic_i32 reset_active;
    lib_atomic_i32 terminate_requested;
    lib_atomic_i32 media_requested;
    lib_atomic_i32 state_read_requested;
    lib_atomic_i32 state_read_armed;
    lib_atomic_i32 state_read_ready;
    lib_atomic_i32 state_write_requested;
    lib_atomic_i32 state_write_waiting;
    lib_atomic_i32 restored_start_requested;
    lib_bool state_read_continuing;
    common_machine_state_writer state_writer;
    common_machine_state_reader state_reader;
    lib_status state_status;
    common_machine_state_sink state_sink;
    void *state_context;
    common_machine_frame_sink frame_sink;
    void *frame_context;
    lib_bool media_succeeded;
    lib_storage_medium_mode media_mode;
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

static lib_bool common_machine_text_frame_changed(const common_machine_frame *previous,
    const common_machine_frame *candidate)
{
    const kvm_text_frame *old_text = &previous->window.text.base;
    const kvm_text_frame *new_text = &candidate->window.text.base;
    if (previous->window.valid == 0u || previous->window.graphics != 0u)
        return LIB_TRUE;
    return old_text->text_columns != new_text->text_columns ||
        old_text->text_rows != new_text->text_rows ||
        old_text->cursor_column != new_text->cursor_column ||
        old_text->cursor_row != new_text->cursor_row ||
        old_text->cursor_top != new_text->cursor_top ||
        old_text->cursor_bottom != new_text->cursor_bottom ||
        old_text->cursor_visible != new_text->cursor_visible ||
        old_text->cursor_phase != new_text->cursor_phase ||
        old_text->font_height != new_text->font_height ||
        lib_memory_compare(old_text->cells, new_text->cells, sizeof(new_text->cells)) != 0 ||
        lib_memory_compare(old_text->text_palette, new_text->text_palette,
            sizeof(new_text->text_palette)) != 0 ||
        lib_memory_compare(previous->window.text.font, candidate->window.text.font, sizeof(candidate->window.text.font)) != 0 ||
        lib_memory_compare(previous->window.text.secondary_font, candidate->window.text.secondary_font,
            sizeof(candidate->window.text.secondary_font)) != 0 ||
        lib_memory_compare(&previous->characters, &candidate->characters,
            sizeof(candidate->characters)) != 0;
}

static lib_status common_machine_publish(common_machine *machine)
{
    lib_status status;
    common_machine_frame *frame;
    lib_i32 staging_index;
    common_machine_frame_sink sink = NULL;
    void *sink_context = NULL;
    lib_u32 sequence = 0u;
    lib_u32 generation = 0u;
    lib_bool graphics = LIB_FALSE;
    common_machine_frame *published_frame = NULL;

    if (machine == NULL || machine->driver.copy_frame == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(machine->frame_lock);
    staging_index = machine->published_frame_index == 0 ? 1 : 0;
    frame = machine->frame_buffers[staging_index];
    frame->window.valid = LIB_FALSE;
    status = machine->driver.copy_frame(machine->driver.context, frame);
    if (status != LIB_STATUS_OK) {
        base_sync_mutex_unlock(machine->frame_lock);
        return status;
    }
    if (frame->window.valid == 0u || (frame->window.graphics == 0u &&
        !common_machine_text_frame_changed(
            machine->frame_buffers[machine->published_frame_index], frame))) {
        base_sync_mutex_unlock(machine->frame_lock);
        return LIB_STATUS_OK;
    }
    /* Zero is reserved for consumers without a published frame. */
    if (++machine->published_frame_sequence == 0u)
        ++machine->published_frame_sequence;
    frame->sequence = machine->published_frame_sequence;
    generation = common_machine_run_generation(machine);
    machine->published_frame_run_generation = generation;
    machine->published_frame_index = staging_index;
    sink = machine->frame_sink;
    sink_context = machine->frame_context;
    sequence = frame->sequence;
    graphics = frame->window.graphics != 0u;
    published_frame = frame;
    base_sync_mutex_unlock(machine->frame_lock);
    if (machine->driver.frame_published != NULL)
        machine->driver.frame_published(machine->driver.context, published_frame);
    if (sink != NULL)
        sink(sink_context, sequence, graphics, generation);
    return LIB_STATUS_OK;
}

static void common_machine_drain_input(common_machine *machine)
{
    kvm_input_event event;
    if (machine == NULL || machine->driver.deliver_input == NULL) return;
    if (common_machine_input_queue_pop(&machine->input_queue, &event)) {
        machine->driver.deliver_input(machine->driver.context, &event);
        if (common_machine_input_queue_pending(&machine->input_queue) &&
            machine->driver.request_wake != NULL) {
            /* The driver uses this wake edge to schedule another safe host
             * callback; it is not a lifecycle stop request. */
            machine->driver.request_wake(machine->driver.context);
        }
    }
}

/* Fatal synchronization failure is not a retryable product operation. The
 * existing task cancellation is independent of its completion event. */
static lib_status common_machine_fail_sync(common_machine *machine)
{
    lib_atomic_i32_store_explicit(&machine->state, COMMON_MACHINE_ERROR, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&machine->reset_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&machine->terminate_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    machine->driver.request_stop(machine->driver.context);
    return base_sync_task_request_cancel(machine->worker);
}

static void common_machine_complete_request(common_machine *machine,
    base_sync_event *completed)
{
    if (base_sync_event_signal(completed) != LIB_STATUS_OK)
        (void)common_machine_fail_sync(machine);
}

/* Registration and claiming serialize; no product code runs under this lock. */
static lib_bool common_machine_take_request(common_machine *machine,
    lib_atomic_i32 *requested)
{
    lib_bool taken = LIB_FALSE;
    base_sync_mutex_lock(machine->request_lock);
    if (lib_atomic_i32_load_explicit(&machine->terminate_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0)
        taken = lib_atomic_i32_exchange_explicit(requested, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0;
    base_sync_mutex_unlock(machine->request_lock);
    return taken;
}

static lib_status common_machine_wait_request(common_machine *machine,
    base_sync_event *completed)
{
    lib_status status;
    base_sync_wait_result result = base_sync_wait_any(&completed, 1u,
        machine->worker, LIB_UINT32_MAX, NULL);
    if (result == BASE_SYNC_WAIT_SIGNALED) return LIB_STATUS_OK;
    if (result != BASE_SYNC_WAIT_CANCELLED)
        (void)common_machine_fail_sync(machine);
    /* Do not end a borrowed state stream while its driver may still use it.
     * A failed join retains the machine; native synchronization loss beyond
     * this independent cancellation channel is not recoverable here. */
    status = base_sync_task_join(machine->worker);
    return status == LIB_STATUS_OK ? LIB_STATUS_IO_ERROR : status;
}

static void common_machine_service_media(common_machine *machine)
{
    if (machine == NULL || !common_machine_take_request(machine, &machine->media_requested))
        return;
    machine->media_succeeded = machine->driver.set_removable_media != NULL &&
        machine->driver.set_removable_media(machine->driver.context,
            machine->media_path[0] == '\0' ? NULL : machine->media_path,
            machine->media_mode);
    common_machine_complete_request(machine, machine->media_event);
}

/* The control thread submits one synchronous operation at a time. Only the
 * existing executor calls the driver, including while parked in PAUSED. */
static void common_machine_service_debug(common_machine *machine)
{
    if (lib_atomic_i32_exchange_explicit(&machine->debug_cancel_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0 &&
        machine->driver.cancel_debug != NULL)
        machine->driver.cancel_debug(machine->driver.context);
    if (!common_machine_take_request(machine, &machine->debug_requested)) return;
    machine->debug_status = LIB_STATUS_INVALID_STATE;
    machine->debug_response_size = 0u;
    lib_memory_set(machine->debug_response, 0, machine->debug_response_capacity);
    if (common_machine_state_get(machine) == COMMON_MACHINE_PAUSED &&
        lib_atomic_i32_load_explicit(&machine->pause_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0 &&
        machine->debug_lease.generation == (lib_u64)(lib_u32)
            lib_atomic_i32_load_explicit(&machine->debug_generation, LIB_MEMORY_ORDER_SEQ_CST))
        machine->debug_status = machine->driver.execute_debug(
            machine->driver.context, machine->debug_request, machine->debug_request_size,
            machine->debug_response, machine->debug_response_capacity,
            &machine->debug_response_size);
    if (machine->debug_response_size > machine->debug_response_capacity)
        machine->debug_status = LIB_STATUS_IO_ERROR;
    if (machine->debug_status != LIB_STATUS_OK) machine->debug_response_size = 0u;
    common_machine_complete_request(machine, machine->debug_event);
}

static void common_machine_service_state_read(common_machine *machine)
{
    lib_status status;
    lib_bool begin;
    if (machine == NULL) return;
    base_sync_mutex_lock(machine->request_lock);
    if (lib_atomic_i32_load_explicit(&machine->terminate_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0 ||
        lib_atomic_i32_load_explicit(&machine->state_read_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0) {
        base_sync_mutex_unlock(machine->request_lock);
        return;
    }
    begin = lib_atomic_i32_exchange_explicit(&machine->state_read_armed, 1,
        LIB_MEMORY_ORDER_SEQ_CST) == 0;
    base_sync_mutex_unlock(machine->request_lock);
    if (begin) {
        status = machine->driver.begin_state_read == NULL ? LIB_STATUS_UNSUPPORTED :
            machine->driver.begin_state_read(machine->driver.context,
                &machine->state_writer);
        if (status != LIB_STATUS_OK) {
            machine->state_status = status;
            lib_atomic_i32_exchange_explicit(&machine->state_read_ready, 1,
                LIB_MEMORY_ORDER_SEQ_CST);
        }
    }
    if (lib_atomic_i32_load_explicit(&machine->state_read_ready,
            LIB_MEMORY_ORDER_SEQ_CST) == 0 &&
        machine->driver.take_state_read_result != NULL &&
        machine->driver.take_state_read_result(machine->driver.context, &status)) {
        machine->state_status = status;
        lib_atomic_i32_exchange_explicit(&machine->state_read_ready, 1,
            LIB_MEMORY_ORDER_SEQ_CST);
    }
    if (lib_atomic_i32_load_explicit(&machine->state_read_ready,
            LIB_MEMORY_ORDER_SEQ_CST) != 0)
        lib_atomic_i32_exchange_explicit(&machine->pause_requested, 1,
            LIB_MEMORY_ORDER_SEQ_CST);
}

static void common_machine_complete_state_read(common_machine *machine)
{
    if (machine == NULL || lib_atomic_i32_exchange_explicit(
            &machine->state_read_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) == 0)
        return;
    lib_atomic_i32_exchange_explicit(&machine->state_read_armed, 0,
        LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->state_read_ready, 0,
        LIB_MEMORY_ORDER_SEQ_CST);
    common_machine_complete_request(machine, machine->state_event);
}

/* The sole control caller copies the payload before publishing its request.
 * Serialize registration with terminal cleanup, never driver execution. */
static lib_status common_machine_submit_request(common_machine *machine,
    lib_atomic_i32 *requested, base_sync_event *completed, lib_u32 allowed_states)
{
    lib_status status = LIB_STATUS_INVALID_STATE;
    base_sync_mutex_lock(machine->request_lock);
    if (lib_atomic_i32_load_explicit(&machine->terminate_requested,
        LIB_MEMORY_ORDER_SEQ_CST) == 0 &&
        (allowed_states & (1u << common_machine_state_get(machine))) != 0u) {
        status = base_sync_event_reset(completed);
        if (status == LIB_STATUS_OK) {
            lib_atomic_i32_store_explicit(requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
            status = base_sync_event_signal(machine->command_event);
            if (status != LIB_STATUS_OK)
                lib_atomic_i32_store_explicit(requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
        }
    }
    base_sync_mutex_unlock(machine->request_lock);
    return status;
}

/* Only after the driver has unwound: borrowed request data is no longer in use.
 * Requests already completed have cleared their flag and keep their result. */
static void common_machine_finish_requests(common_machine *machine,
    common_machine_state terminal)
{
    base_sync_mutex_lock(machine->request_lock);
    if (common_machine_state_get(machine) == COMMON_MACHINE_ERROR)
        terminal = COMMON_MACHINE_ERROR;
    lib_atomic_i32_store_explicit(&machine->state, terminal, LIB_MEMORY_ORDER_SEQ_CST);
    if (lib_atomic_i32_load_explicit(&machine->state_read_requested,
            LIB_MEMORY_ORDER_SEQ_CST) != 0) {
        machine->state_status = LIB_STATUS_IO_ERROR;
        machine->state_read_continuing = LIB_FALSE;
        common_machine_complete_state_read(machine);
    }
    if (lib_atomic_i32_exchange_explicit(&machine->state_write_requested, 0,
            LIB_MEMORY_ORDER_SEQ_CST) |
        lib_atomic_i32_exchange_explicit(&machine->state_write_waiting, 0,
            LIB_MEMORY_ORDER_SEQ_CST)) {
        machine->state_status = LIB_STATUS_IO_ERROR;
        common_machine_complete_request(machine, machine->state_event);
    }
    if (lib_atomic_i32_exchange_explicit(&machine->debug_requested, 0,
            LIB_MEMORY_ORDER_SEQ_CST) != 0) {
        machine->debug_status = LIB_STATUS_IO_ERROR;
        machine->debug_response_size = 0u;
        common_machine_complete_request(machine, machine->debug_event);
    }
    if (lib_atomic_i32_exchange_explicit(&machine->media_requested, 0,
            LIB_MEMORY_ORDER_SEQ_CST) != 0) {
        machine->media_succeeded = LIB_FALSE;
        common_machine_complete_request(machine, machine->media_event);
    }
    base_sync_mutex_unlock(machine->request_lock);
}

static void common_machine_executor_event(void *opaque)
{
    common_machine *machine = (common_machine *)opaque;
    lib_bool suppress_paused_fact = LIB_FALSE;
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
    if (common_machine_publish(machine) != LIB_STATUS_OK) {
        lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_ERROR, LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_exchange_explicit(&machine->reset_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_exchange_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
        machine->driver.request_stop(machine->driver.context);
        return;
    }
    common_machine_service_state_read(machine);
    if (machine->state_read_continuing && lib_atomic_i32_load_explicit(
            &machine->state_read_ready, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
        suppress_paused_fact = LIB_TRUE;
        machine->state_read_continuing = LIB_FALSE;
    }
    if (lib_atomic_i32_load_explicit(&machine->pause_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0 &&
        lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0) {
        lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_PAUSED, LIB_MEMORY_ORDER_SEQ_CST);
        if (lib_atomic_i32_exchange_explicit(&machine->reset_active, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0)
            common_machine_notify_state(machine, COMMON_MACHINE_RESET_COMPLETED);
        else if (!suppress_paused_fact)
            common_machine_notify_state(machine, COMMON_MACHINE_PAUSED);
        if (lib_atomic_i32_load_explicit(&machine->state_read_ready,
                LIB_MEMORY_ORDER_SEQ_CST) != 0)
            common_machine_complete_state_read(machine);
        if (lib_atomic_i32_exchange_explicit(&machine->state_write_waiting, 0,
                LIB_MEMORY_ORDER_SEQ_CST) != 0)
            common_machine_complete_request(machine, machine->state_event);
        while (lib_atomic_i32_load_explicit(&machine->pause_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0 &&
            lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) == 0) {
            base_sync_event *events[3] = { machine->resume_event,
                machine->command_event, machine->input_event };
            lib_u32 index = LIB_UINT32_MAX;
            base_sync_wait_result result = base_sync_wait_any(events, 3u,
                machine->worker, LIB_UINT32_MAX, &index);
            if (result != BASE_SYNC_WAIT_SIGNALED) {
                if (result != BASE_SYNC_WAIT_CANCELLED)
                    lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_ERROR, LIB_MEMORY_ORDER_SEQ_CST);
                lib_atomic_i32_exchange_explicit(&machine->reset_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
                lib_atomic_i32_exchange_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
                machine->driver.request_stop(machine->driver.context);
                break;
            }
            if (base_sync_event_reset(events[index]) != LIB_STATUS_OK) {
                (void)common_machine_fail_sync(machine);
                break;
            }
            if (index == 1u) {
                common_machine_service_media(machine);
                common_machine_service_debug(machine);
                common_machine_service_state_read(machine);
                if (lib_atomic_i32_load_explicit(&machine->state_read_requested,
                        LIB_MEMORY_ORDER_SEQ_CST) != 0) {
                    if (lib_atomic_i32_load_explicit(&machine->state_read_ready,
                            LIB_MEMORY_ORDER_SEQ_CST) != 0)
                        common_machine_complete_state_read(machine);
                    else {
                        machine->state_read_continuing = LIB_TRUE;
                        lib_atomic_i32_exchange_explicit(&machine->pause_requested,
                            0, LIB_MEMORY_ORDER_SEQ_CST);
                        machine->driver.request_wake(machine->driver.context);
                    }
                }
            } else if (index == 2u) {
                common_machine_drain_input(machine);
            }
        }
        if (lib_atomic_i32_load_explicit(&machine->stop_requested,
                LIB_MEMORY_ORDER_SEQ_CST) == 0 &&
            !machine->state_read_continuing) {
            lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_RUNNING, LIB_MEMORY_ORDER_SEQ_CST);
            common_machine_notify_state(machine, COMMON_MACHINE_RUNNING);
        }
    }
}

static void common_machine_begin_cold_run(common_machine *machine,
    lib_bool pause_after_start)
{
    common_machine_debug_invalidate(machine);
    common_machine_input_queue_clear(&machine->input_queue);
    common_machine_invalidate_published_frame(machine);
    lib_atomic_i32_exchange_explicit(&machine->pause_requested, pause_after_start != 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_STARTING, LIB_MEMORY_ORDER_SEQ_CST);
    (void)lib_atomic_u32_fetch_add_explicit(&machine->run_generation, 1u,
        LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&machine->start_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
}

static lib_bool common_machine_schedule_cold_run(common_machine *machine,
    lib_bool pause_after_start)
{
    if (machine == NULL || machine->worker == NULL || lib_atomic_i32_load_explicit(&machine->state, LIB_MEMORY_ORDER_SEQ_CST) !=
        COMMON_MACHINE_STOPPED) return LIB_FALSE;
    common_machine_begin_cold_run(machine, pause_after_start);
    base_sync_event_signal(machine->command_event);
    return LIB_TRUE;
}

static void common_machine_service_state_write(common_machine *machine)
{
    lib_status status;
    if (machine == NULL || !common_machine_take_request(machine, &machine->state_write_requested))
        return;
    status = machine->driver.write_state == NULL ? LIB_STATUS_UNSUPPORTED :
        machine->driver.write_state(machine->driver.context, &machine->state_reader);
    machine->state_status = status;
    if (status == LIB_STATUS_OK) {
        common_machine_debug_invalidate(machine);
        common_machine_input_queue_clear(&machine->input_queue);
        common_machine_invalidate_published_frame(machine);
        lib_atomic_i32_exchange_explicit(&machine->pause_requested, 1,
            LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_exchange_explicit(&machine->stop_requested, 0,
            LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_exchange_explicit(&machine->state,
            COMMON_MACHINE_STARTING, LIB_MEMORY_ORDER_SEQ_CST);
        (void)lib_atomic_u32_fetch_add_explicit(&machine->run_generation, 1u,
            LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_exchange_explicit(&machine->restored_start_requested, 1,
            LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_exchange_explicit(&machine->state_write_waiting, 1,
            LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_exchange_explicit(&machine->start_requested, 1,
            LIB_MEMORY_ORDER_SEQ_CST);
        if (base_sync_event_signal(machine->command_event) != LIB_STATUS_OK)
            (void)common_machine_fail_sync(machine);
    } else
        common_machine_complete_request(machine, machine->state_event);
}

static void common_machine_worker(void *opaque, const base_sync_task *task)
{
    common_machine *machine = (common_machine *)opaque;
    lib_bool failure_notified = LIB_FALSE;
    for (;;) {
        lib_bool succeeded;
        /* The paused callback may have consumed the terminal command wake. */
        base_sync_wait_result result = base_sync_wait_any(&machine->command_event,
            1u, task, LIB_UINT32_MAX, NULL);
        if (result == BASE_SYNC_WAIT_CANCELLED) break;
        if (result != BASE_SYNC_WAIT_SIGNALED) {
            common_machine_finish_requests(machine, COMMON_MACHINE_ERROR);
            common_machine_notify_state(machine, COMMON_MACHINE_ERROR);
            failure_notified = LIB_TRUE;
            break;
        }
        if (base_sync_event_reset(machine->command_event) != LIB_STATUS_OK) {
            (void)common_machine_fail_sync(machine);
            break;
        }
        if (lib_atomic_i32_load_explicit(&machine->terminate_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0)
            break;
        common_machine_service_debug(machine);
        if (lib_atomic_i32_load_explicit(&machine->terminate_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0)
            break;
        if (lib_atomic_i32_load_explicit(&machine->media_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
            common_machine_service_media(machine);
            continue;
        }
        if (lib_atomic_i32_load_explicit(&machine->state_write_requested,
                LIB_MEMORY_ORDER_SEQ_CST) != 0) {
            common_machine_service_state_write(machine);
            continue;
        }
        {
            lib_bool restored = lib_atomic_i32_exchange_explicit(
                &machine->restored_start_requested, 0,
                LIB_MEMORY_ORDER_SEQ_CST) != 0;
            if (lib_atomic_i32_exchange_explicit(&machine->start_requested, 0,
                    LIB_MEMORY_ORDER_SEQ_CST) == 0) continue;
            succeeded = restored || machine->driver.reset(machine->driver.context);
            if (restored) machine->state_status = LIB_STATUS_OK;
        }
        if (!succeeded) {
            common_machine_finish_requests(machine, COMMON_MACHINE_ERROR);
            common_machine_notify_state(machine, COMMON_MACHINE_ERROR);
            failure_notified = LIB_TRUE;
            continue;
        }
        if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
            common_machine_finish_requests(machine, COMMON_MACHINE_STOPPED);
            common_machine_notify_state(machine, COMMON_MACHINE_STOPPED);
            continue;
        }
        machine->driver.set_executor_callback(machine->driver.context,
            common_machine_executor_event, machine);
        machine->driver.set_heartbeat(machine->driver.context, LIB_TRUE);
        lib_atomic_i32_exchange_explicit(&machine->state, COMMON_MACHINE_RUNNING, LIB_MEMORY_ORDER_SEQ_CST);
        if (lib_atomic_i32_load_explicit(&machine->reset_active, LIB_MEMORY_ORDER_SEQ_CST) == 0 &&
            lib_atomic_i32_load_explicit(&machine->pause_requested,
                LIB_MEMORY_ORDER_SEQ_CST) == 0)
            common_machine_notify_state(machine, COMMON_MACHINE_RUNNING);
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
            common_machine_finish_requests(machine, COMMON_MACHINE_STARTING);
            lib_atomic_i32_exchange_explicit(&machine->stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
            common_machine_begin_cold_run(machine, LIB_TRUE);
            if (base_sync_event_signal(machine->command_event) != LIB_STATUS_OK)
                (void)common_machine_fail_sync(machine);
            continue;
        }
        common_machine_finish_requests(machine, succeeded ? COMMON_MACHINE_STOPPED :
            COMMON_MACHINE_ERROR);
        common_machine_notify_state(machine, succeeded ? COMMON_MACHINE_STOPPED :
            COMMON_MACHINE_ERROR);
        failure_notified = !succeeded;
    }
    lib_atomic_i32_store_explicit(&machine->terminate_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    common_machine_finish_requests(machine, common_machine_state_get(machine));
    if (!failure_notified && common_machine_state_get(machine) == COMMON_MACHINE_ERROR)
        common_machine_notify_state(machine, COMMON_MACHINE_ERROR);
}

lib_status common_machine_create(common_machine **out_machine,
    const common_machine_driver *driver)
{
    common_machine *machine;
    lib_status status;
    if (out_machine == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = NULL;
    if (driver == NULL || driver->reset == NULL ||
        driver->run == NULL || driver->request_stop == NULL ||
        driver->request_wake == NULL ||
        driver->set_heartbeat == NULL || driver->set_executor_callback == NULL ||
        driver->deliver_input == NULL || driver->copy_frame == NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    machine = lib_allocate_zero(1u, sizeof(*machine));
    if (machine == NULL) return LIB_STATUS_NO_MEMORY;
    lib_atomic_i32_initialize(&machine->debug_requested, 0);
    lib_atomic_i32_initialize(&machine->debug_cancel_requested, 0);
    lib_atomic_i32_initialize(&machine->state, COMMON_MACHINE_STOPPED);
    lib_atomic_u32_initialize(&machine->run_generation, 0u);
    lib_atomic_i32_initialize(&machine->debug_generation, 1);
    lib_atomic_i32_initialize(&machine->pause_requested, 0);
    lib_atomic_i32_initialize(&machine->stop_requested, 0);
    lib_atomic_i32_initialize(&machine->start_requested, 0);
    lib_atomic_i32_initialize(&machine->reset_requested, 0);
    lib_atomic_i32_initialize(&machine->reset_active, 0);
    lib_atomic_i32_initialize(&machine->terminate_requested, 0);
    lib_atomic_i32_initialize(&machine->media_requested, 0);
    lib_atomic_i32_initialize(&machine->state_read_requested, 0);
    lib_atomic_i32_initialize(&machine->state_read_armed, 0);
    lib_atomic_i32_initialize(&machine->state_read_ready, 0);
    lib_atomic_i32_initialize(&machine->state_write_requested, 0);
    lib_atomic_i32_initialize(&machine->state_write_waiting, 0);
    lib_atomic_i32_initialize(&machine->restored_start_requested, 0);
    machine->driver = *driver;
    status = base_sync_mutex_create(&machine->frame_lock);
    if (status == LIB_STATUS_OK) status = base_sync_mutex_create(&machine->request_lock);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->command_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->resume_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->input_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->media_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->debug_event);
    if (status == LIB_STATUS_OK) status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &machine->state_event);
    if (status == LIB_STATUS_OK) status = common_machine_input_queue_initialize(&machine->input_queue);
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
    const char *path, lib_storage_medium_mode mode)
{
    lib_size length;
    if (machine == NULL || machine->worker == NULL ||
        machine->driver.set_removable_media == NULL ||
        mode > LIB_STORAGE_MEDIUM_OVERLAY)
        return LIB_FALSE;
    if (path == NULL) machine->media_path[0] = '\0';
    else {
        length = lib_text_length(path);
        if (length >= sizeof(machine->media_path)) return LIB_FALSE;
        lib_memory_copy(machine->media_path, path, length + 1u);
    }
    machine->media_mode = mode;
    if (common_machine_submit_request(machine, &machine->media_requested,
            machine->media_event, (1u << COMMON_MACHINE_STOPPED) |
                (1u << COMMON_MACHINE_PAUSED)) != LIB_STATUS_OK) return LIB_FALSE;
    return common_machine_wait_request(machine, machine->media_event) ==
        LIB_STATUS_OK && machine->media_succeeded;
}

lib_status common_machine_read_state(common_machine *machine,
    const common_machine_state_writer *writer)
{
    lib_status status;
    if (machine == NULL || writer == NULL || writer->write == NULL)
        return LIB_STATUS_INVALID_STATE;
    machine->state_writer = *writer;
    status = common_machine_submit_request(machine, &machine->state_read_requested,
            machine->state_event, (1u << COMMON_MACHINE_RUNNING) |
                (1u << COMMON_MACHINE_PAUSED));
    if (status != LIB_STATUS_OK) return status;
    machine->driver.request_wake(machine->driver.context);
    status = common_machine_wait_request(machine, machine->state_event);
    return status == LIB_STATUS_OK ? machine->state_status : status;
}

lib_status common_machine_write_state(common_machine *machine,
    const common_machine_state_reader *reader)
{
    lib_status status;
    if (machine == NULL || reader == NULL || reader->read == NULL)
        return LIB_STATUS_INVALID_STATE;
    machine->state_reader = *reader;
    status = common_machine_submit_request(machine, &machine->state_write_requested,
        machine->state_event, 1u << COMMON_MACHINE_STOPPED);
    if (status != LIB_STATUS_OK) return status;
    status = common_machine_wait_request(machine, machine->state_event);
    return status == LIB_STATUS_OK ? machine->state_status : status;
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
        !common_machine_input_queue_push(&machine->input_queue, event))
        return LIB_FALSE;
    base_sync_event_signal(machine->input_event);
    machine->driver.request_wake(machine->driver.context);
    return LIB_TRUE;
}

lib_bool common_machine_copy_published_frame(common_machine *machine,
    common_machine_frame *destination, lib_u32 run_generation)
{
    lib_bool copied;
    if (machine == NULL || destination == NULL) return LIB_FALSE;
    base_sync_mutex_lock(machine->frame_lock);
    copied = machine->published_frame_run_generation == run_generation &&
        machine->frame_buffers[machine->published_frame_index]->window.valid != 0u;
    if (copied)
        copied = common_machine_frame_copy(destination,
            machine->frame_buffers[machine->published_frame_index]);
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
    return machine == NULL ? 0u : lib_atomic_u32_load_explicit(
        &machine->run_generation, LIB_MEMORY_ORDER_SEQ_CST);
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
    const void *request, lib_size request_size,
    void *response, lib_size response_capacity, lib_size *response_size)
{
    lib_i32 generation;
    lib_status status;

    if (response_size == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *response_size = 0u;
    if (machine == NULL || lease == NULL || (request == NULL && request_size != 0u) ||
        (response == NULL && response_capacity != 0u)) return LIB_STATUS_INVALID_ARGUMENT;
    if (request_size > sizeof(machine->debug_request) ||
        response_capacity > sizeof(machine->debug_response)) return LIB_STATUS_UNSUPPORTED;
    if (common_machine_state_get(machine) != COMMON_MACHINE_PAUSED)
        return LIB_STATUS_INVALID_STATE;
    generation = lib_atomic_i32_load_explicit(&machine->debug_generation, LIB_MEMORY_ORDER_SEQ_CST);
    if (lease->generation == 0u || lease->generation != (lib_u64)(lib_u32)generation)
        return LIB_STATUS_INVALID_STATE;
    if (machine->driver.execute_debug == NULL) return LIB_STATUS_UNSUPPORTED;
    if (request_size != 0u) lib_memory_copy(machine->debug_request, request, request_size);
    machine->debug_request_size = request_size;
    machine->debug_response_capacity = response_capacity;
    machine->debug_lease = *lease;
    status = common_machine_submit_request(machine, &machine->debug_requested,
        machine->debug_event, 1u << COMMON_MACHINE_PAUSED);
    if (status != LIB_STATUS_OK) return status;
    status = common_machine_wait_request(machine, machine->debug_event);
    if (status != LIB_STATUS_OK) return status;
    if (machine->debug_status == LIB_STATUS_OK) {
        if (machine->debug_response_size != 0u)
            lib_memory_copy(response, machine->debug_response, machine->debug_response_size);
        *response_size = machine->debug_response_size;
    }
    return machine->debug_status;
}

lib_status common_machine_shutdown(common_machine *machine)
{
    lib_status status;
    if (machine == NULL || machine->worker == NULL) return LIB_STATUS_OK;
    common_machine_debug_invalidate(machine);
    (void)common_machine_stop(machine);
    lib_atomic_i32_exchange_explicit(&machine->terminate_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    if (machine->resume_event != NULL) base_sync_event_signal(machine->resume_event);
    if (machine->command_event != NULL) base_sync_event_signal(machine->command_event);
    status = base_sync_task_destroy(machine->worker);
    if (status != LIB_STATUS_OK) return status;
    machine->worker = NULL;
    return LIB_STATUS_OK;
}

lib_status common_machine_destroy(common_machine *machine)
{
    lib_status status;
    if (machine == NULL) return LIB_STATUS_OK;
    status = common_machine_shutdown(machine);
    if (status != LIB_STATUS_OK) return status;
    base_sync_event_destroy(machine->resume_event);
    base_sync_event_destroy(machine->input_event);
    base_sync_event_destroy(machine->media_event);
    base_sync_event_destroy(machine->debug_event);
    base_sync_event_destroy(machine->state_event);
    common_machine_input_queue_dispose(&machine->input_queue);
    base_sync_mutex_destroy(machine->frame_lock);
    base_sync_mutex_destroy(machine->request_lock);
    lib_release(machine->frame_buffers[0]);
    lib_release(machine->frame_buffers[1]);
    base_sync_event_destroy(machine->command_event);
    lib_release(machine);
    return LIB_STATUS_OK;
}
