#ifndef MACHINE_DRIVER_H
#define MACHINE_DRIVER_H

#include "machine/driver_interface.h"
#include "core/machine_interface.h"
#include "lib/audio/pcm/stream_interface.h"

#define MACHINE_DRIVER_INPUT_SOURCE_CAPACITY 2u
#define MACHINE_DRIVER_INPUT_BINDING_CAPACITY 16u
#define MACHINE_DRIVER_AUDIO_STAGING_CAPACITY 4096u
#define MACHINE_DRIVER_AUDIO_PLAY_BLOCK LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION

struct machine_driver {
    core_machine *machine;
    lib_audio_pcm_stream *audio;
    lib_i32 audio_staging[MACHINE_DRIVER_AUDIO_STAGING_CAPACITY];
    lib_u16 audio_staging_read;
    lib_u16 audio_staging_write;
    lib_u16 audio_staging_count;
    lib_bool audio_needs_clear;
    machine_driver_options options;
    emulator_machine_executor_callback executor_callback;
    void *executor_context;
    lib_atomic_i32 stop_requested;
    lib_atomic_i32 wake_requested;
    lib_atomic_i32 debug_stop_requested;
    lib_atomic_i32 input_reset_requested;
    /* Pressed buttons belong to an input source.  The controller receives the
     * union, so one retiring source cannot release a button another source
     * still holds. */
    lib_u64 input_source_identities[MACHINE_DRIVER_INPUT_SOURCE_CAPACITY];
    lib_u8 input_source_buttons[MACHINE_DRIVER_INPUT_SOURCE_CAPACITY];
    lib_u16 input_source_bindings[MACHINE_DRIVER_INPUT_SOURCE_CAPACITY];
    lib_bool input_source_active[MACHINE_DRIVER_INPUT_SOURCE_CAPACITY];
    lib_u32 published_frame_revision;
    lib_u16 video_samples[CORE_MACHINE_VIDEO_SAMPLE_CAPACITY];
    lib_u64 pacing_origin_ms;
    lib_u64 pacing_origin_cycles;
    lib_bool pacing_ready;
    lib_bool text_output;
    emulator_machine_state_writer state_writer;
    lib_status state_read_status;
    lib_bool state_read_ready;
};

lib_bool machine_driver_reset(void *context);
lib_bool machine_driver_run(void *context);
void machine_driver_request_stop(void *context);
void machine_driver_request_wake(void *context);
void machine_driver_set_heartbeat(void *context, lib_bool enabled);
void machine_driver_set_executor_callback(void *context,
    emulator_machine_executor_callback callback, void *callback_context);
void machine_driver_deliver_input(void *context, const kvm_input_event *event);
lib_status machine_driver_copy_frame(void *context, emulator_machine_frame *out_frame);
lib_bool machine_driver_take_debug_stop(void *context);
void machine_driver_cancel_debug(void *context);
emulator_machine_debug_execute machine_driver_debug_execute(void);
lib_bool machine_driver_set_media(void *context, const char *path,
    lib_storage_medium_mode mode);
lib_status machine_driver_begin_state_read(void *context,
    const emulator_machine_state_writer *writer);
lib_bool machine_driver_take_state_read_result(void *context, lib_status *out_status);
lib_status machine_driver_write_state(void *context,
    const emulator_machine_state_reader *reader);

#endif
