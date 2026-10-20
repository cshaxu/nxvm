#include "myraidenii/machine/driver_interface.h"

#include "lib/base/sync_interface.h"
#include "myraidenii/machine/audio_interface.h"

enum {
    /* L1 host-control bound: it creates no guest ticks or timing conversion. */
    MYRAIDENII_MACHINE_RUN_INSTRUCTION_QUANTUM = 256u
};

struct myraidenii_machine {
    myraidenii_core_board *board;
    x86_core_machine *core_machine;
    lib_audio_pcm_stream *stream;
    myraidenii_machine_audio *audio;
    emulator_machine_executor_callback executor_callback;
    void *executor_context;
    lib_atomic_i32 stop_requested;
    lib_atomic_i32 wake_requested;
    lib_atomic_i32 failed;
    lib_u32 frame_sequence;
};

static void myraidenii_machine_fail(myraidenii_machine *machine)
{
    if (machine != LIB_NULL)
        lib_atomic_i32_store_explicit(&machine->failed, 1, LIB_MEMORY_ORDER_SEQ_CST);
}

static lib_bool myraidenii_machine_reset(void *context)
{
    myraidenii_machine *machine = context;

    if (machine == LIB_NULL || myraidenii_machine_audio_clear(machine->audio) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine->core_machine) != LIB_STATUS_OK) {
        myraidenii_machine_fail(machine);
        return LIB_FALSE;
    }
    lib_atomic_i32_store_explicit(&machine->stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    return LIB_TRUE;
}

static lib_bool myraidenii_machine_run(void *context)
{
    myraidenii_machine *machine = context;
    x86_core_machine_run_result result;
    lib_status status;

    if (machine == LIB_NULL ||
        lib_atomic_i32_load_explicit(&machine->failed, LIB_MEMORY_ORDER_SEQ_CST) != 0)
        return LIB_FALSE;
    if (machine->executor_callback != LIB_NULL)
        machine->executor_callback(machine->executor_context);
    if (lib_atomic_i32_load_explicit(&machine->stop_requested, LIB_MEMORY_ORDER_SEQ_CST) != 0)
        return LIB_TRUE;
    status = x86_core_machine_run(machine->core_machine,
        (x86_core_machine_run_budget){ MYRAIDENII_MACHINE_RUN_INSTRUCTION_QUANTUM, 0u },
        &result);
    if (status != LIB_STATUS_OK || result.reason == X86_CORE_MACHINE_STOP_FAULT) {
        myraidenii_machine_fail(machine);
        return LIB_FALSE;
    }
    if (result.reason == X86_CORE_MACHINE_STOP_REQUESTED) return LIB_TRUE;
    status = myraidenii_machine_audio_drain(machine->audio);
    if (status == LIB_STATUS_LIMIT_EXCEEDED) {
        if (lib_atomic_i32_exchange_explicit(&machine->wake_requested, 0,
                LIB_MEMORY_ORDER_SEQ_CST) != 0) return LIB_TRUE;
        status = lib_audio_pcm_stream_wait_writable(machine->stream);
        if (status == LIB_STATUS_INVALID_STATE &&
            lib_atomic_i32_load_explicit(&machine->stop_requested,
                LIB_MEMORY_ORDER_SEQ_CST) != 0) return LIB_TRUE;
        if (status == LIB_STATUS_INVALID_STATE &&
            lib_atomic_i32_exchange_explicit(&machine->wake_requested, 0,
                LIB_MEMORY_ORDER_SEQ_CST) != 0) return LIB_TRUE;
    }
    if (status != LIB_STATUS_OK) {
        myraidenii_machine_fail(machine);
        return LIB_FALSE;
    }
    if (result.reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        lib_u8 advanced = LIB_FALSE;

        if (x86_core_machine_advance_to_next_deadline(machine->core_machine,
                &advanced) != LIB_STATUS_OK) {
            myraidenii_machine_fail(machine);
            return LIB_FALSE;
        }
        if (advanced == LIB_FALSE) base_sync_yield();
    }
    if (machine->executor_callback != LIB_NULL)
        machine->executor_callback(machine->executor_context);
    return LIB_TRUE;
}

static void myraidenii_machine_request_stop(void *context)
{
    myraidenii_machine *machine = context;

    if (machine == LIB_NULL) return;
    lib_atomic_i32_store_explicit(&machine->stop_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    (void)x86_core_machine_request_stop(machine->core_machine);
    (void)lib_audio_pcm_stream_cancel_wait(machine->stream);
}

static void myraidenii_machine_request_wake(void *context)
{
    myraidenii_machine *machine = context;

    if (machine == LIB_NULL) return;
    lib_atomic_i32_store_explicit(&machine->wake_requested, 1, LIB_MEMORY_ORDER_SEQ_CST);
    (void)lib_audio_pcm_stream_cancel_wait(machine->stream);
}

static void myraidenii_machine_set_heartbeat(void *context, lib_bool enabled)
{
    myraidenii_machine *machine = context;
    lib_status status;

    if (machine == LIB_NULL) return;
    if (enabled == LIB_FALSE) {
        status = myraidenii_machine_audio_clear(machine->audio);
        if (status == LIB_STATUS_OK)
            status = lib_audio_pcm_stream_set_active(machine->stream, LIB_FALSE);
    } else {
        status = lib_audio_pcm_stream_set_active(machine->stream, LIB_TRUE);
    }
    if (status != LIB_STATUS_OK) myraidenii_machine_fail(machine);
}

static void myraidenii_machine_set_executor_callback(void *context,
    emulator_machine_executor_callback callback, void *callback_context)
{
    myraidenii_machine *machine = context;

    if (machine == LIB_NULL) return;
    machine->executor_callback = callback;
    machine->executor_context = callback_context;
}

static void myraidenii_machine_deliver_input(void *context, const kvm_input_event *event)
{
    (void)context;
    (void)event;
}

static lib_status myraidenii_machine_copy_frame(void *context,
    kvm_window_frame_storage *out_frame, kvm_console_text_frame *out_console,
    lib_bool *out_produced)
{
    myraidenii_machine *machine = context;
    kvm_window_graphics_frame *graphics;
    lib_status status;

    (void)out_console;
    if (out_produced == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_produced = LIB_FALSE;
    if (machine == LIB_NULL || out_frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    graphics = kvm_window_frame_begin_graphics(out_frame);
    graphics->width = MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH;
    graphics->height = MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT;
    graphics->stride = MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH;
    status = myraidenii_core_board_frame_copy(machine->board, graphics->pixels,
        MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS);
    if (status != LIB_STATUS_OK) return status;
    out_frame->frame.sequence = ++machine->frame_sequence;
    *out_produced = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status myraidenii_machine_create(myraidenii_core_board *board, lib_audio_pcm_stream *stream,
    myraidenii_machine **out_machine)
{
    myraidenii_machine *machine;
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (board == LIB_NULL || stream == LIB_NULL ||
        myraidenii_core_board_machine_get(board) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    machine = lib_allocate_zero(1u, sizeof(*machine));
    if (machine == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    machine->board = board;
    machine->core_machine = myraidenii_core_board_machine_get(board);
    lib_atomic_i32_initialize(&machine->stop_requested, 0);
    lib_atomic_i32_initialize(&machine->wake_requested, 0);
    lib_atomic_i32_initialize(&machine->failed, 0);
    status = myraidenii_machine_audio_create(board, stream, &machine->audio);
    if (status != LIB_STATUS_OK) {
        lib_release(machine);
        return status;
    }
    machine->stream = stream;
    *out_machine = machine;
    return LIB_STATUS_OK;
}

lib_status myraidenii_machine_driver_get(myraidenii_machine *machine,
    emulator_machine_driver *out_driver)
{
    if (machine == LIB_NULL || out_driver == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_driver = (emulator_machine_driver) {
        .context = machine,
        .reset = myraidenii_machine_reset,
        .run = myraidenii_machine_run,
        .request_stop = myraidenii_machine_request_stop,
        .request_wake = myraidenii_machine_request_wake,
        .set_heartbeat = myraidenii_machine_set_heartbeat,
        .set_executor_callback = myraidenii_machine_set_executor_callback,
        .deliver_input = myraidenii_machine_deliver_input,
        .copy_frame = myraidenii_machine_copy_frame
    };
    return LIB_STATUS_OK;
}

lib_status myraidenii_machine_destroy(myraidenii_machine **machine)
{
    lib_status status;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (*machine == LIB_NULL) return LIB_STATUS_OK;
    status = lib_audio_pcm_stream_destroy(&(*machine)->stream);
    if (status != LIB_STATUS_OK) return status;
    myraidenii_machine_audio_destroy((*machine)->audio);
    lib_release(*machine);
    *machine = LIB_NULL;
    return LIB_STATUS_OK;
}
