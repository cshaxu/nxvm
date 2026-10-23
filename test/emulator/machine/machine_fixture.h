#ifndef EMULATOR_TEST_MACHINE_FIXTURE_H
#define EMULATOR_TEST_MACHINE_FIXTURE_H

#include "emulator/machine/machine_interface.h"
#include "lib/types/atomic.h"
#include "lib/base/sync_interface.h"

typedef struct machine_fake {
    base_sync_event *stopped;
    base_sync_event *state_stopped;
    base_sync_event *reset_completed;
    base_sync_event *wake;
    base_sync_event *running;
    base_sync_event *run_waiting;
    base_sync_event *frame;
    base_sync_event *input;
    emulator_machine_executor_callback callback;
    void *callback_context;
    lib_atomic_i32 resets;
    lib_atomic_i32 runs;
    lib_atomic_i32 inputs;
    lib_atomic_i32 debug_calls;
    base_sync_event *callback_entered;
    base_sync_event *callback_release;
    lib_atomic_i32 notifications;
    emulator_machine_debug_execute debug;
    void *debug_context;
    lib_u8 state_byte;
    lib_atomic_i32 state_reads;
    lib_atomic_i32 state_writes;
    lib_atomic_i32 defer_state_read;
    lib_atomic_i32 state_read_waiting;
    lib_atomic_i32 state_read_ready;
    emulator_machine_state_writer deferred_state_writer;
    lib_atomic_i32 running_notifications;
    lib_atomic_i32 paused_notifications;
    lib_atomic_i32 media_calls;
    lib_storage_medium_mode media_mode;
    char media_path[EMULATOR_MACHINE_PATH_CAPACITY];
    lib_atomic_i32 fail_frame;
    lib_atomic_i32 fail_on_read_wake;
    lib_atomic_i32 fail_after_read_arm;
    lib_atomic_i32 fail_run;
} machine_fake;

void machine_fake_initialize(machine_fake *fake, emulator_machine_driver *driver);
void machine_fake_dispose(machine_fake *fake);
void machine_fake_note_state(void *opaque, emulator_machine_state state,
    lib_u32 generation);
void machine_fake_note_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation);

#endif
