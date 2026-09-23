#ifndef CORE_DRIVER_H
#define CORE_DRIVER_H

#include "core/driver_interface.h"
#include "core/machine_interface.h"
#include "lib/audio/stream_interface.h"

#define CORE_DRIVER_INPUT_SOURCE_CAPACITY 2u
#define CORE_DRIVER_INPUT_BINDING_CAPACITY 16u
#define CORE_DRIVER_AUDIO_STAGING_CAPACITY 4096u
#define CORE_DRIVER_AUDIO_PLAY_BLOCK LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION

struct core_driver {
    core_machine *machine;
    lib_audio_stream *audio;
    lib_i32 audio_staging[CORE_DRIVER_AUDIO_STAGING_CAPACITY];
    lib_u16 audio_staging_read;
    lib_u16 audio_staging_write;
    lib_u16 audio_staging_count;
    core_driver_options options;
    common_machine_executor_callback executor_callback;
    void *executor_context;
    lib_atomic_i32 stop_requested;
    lib_atomic_i32 wake_requested;
    lib_atomic_i32 debug_stop_requested;
    lib_atomic_i32 input_reset_requested;
    /* Pressed buttons belong to an input source.  The controller receives the
     * union, so one retiring source cannot release a button another source
     * still holds. */
    lib_u64 input_source_identities[CORE_DRIVER_INPUT_SOURCE_CAPACITY];
    lib_u8 input_source_buttons[CORE_DRIVER_INPUT_SOURCE_CAPACITY];
    lib_u16 input_source_bindings[CORE_DRIVER_INPUT_SOURCE_CAPACITY];
    lib_bool input_source_active[CORE_DRIVER_INPUT_SOURCE_CAPACITY];
    lib_u32 published_frame_revision;
    lib_u64 pacing_origin_ms;
    lib_u64 pacing_origin_cycles;
    lib_bool pacing_ready;
    lib_bool text_output;
    common_machine_state_writer state_writer;
    lib_status state_read_status;
    lib_bool state_read_ready;
};

lib_bool core_driver_reset(void *context);
lib_bool core_driver_run(void *context);
void core_driver_request_stop(void *context);
void core_driver_request_wake(void *context);
void core_driver_set_heartbeat(void *context, lib_bool enabled);
void core_driver_set_executor_callback(void *context,
    common_machine_executor_callback callback, void *callback_context);
void core_driver_deliver_input(void *context, const kvm_input_event *event);
lib_status core_driver_copy_frame(void *context, common_machine_frame *out_frame);
lib_bool core_driver_take_debug_stop(void *context);
void core_driver_cancel_debug(void *context);
common_machine_debug_execute core_driver_debug_execute(void);
lib_bool core_driver_set_media(void *context, const char *path,
    lib_storage_medium_mode mode);
lib_status core_driver_begin_state_read(void *context,
    const common_machine_state_writer *writer);
lib_bool core_driver_take_state_read_result(void *context, lib_status *out_status);
lib_status core_driver_write_state(void *context,
    const common_machine_state_reader *reader);

#endif
