#ifndef COMMON_TEST_MACHINE_FIXTURE_H
#define COMMON_TEST_MACHINE_FIXTURE_H

#include "common/machine/machine_interface.h"
#include <windows.h>

typedef struct machine_fake {
    HANDLE stopped;
    HANDLE state_stopped;
    HANDLE reset_completed;
    HANDLE wake;
    HANDLE running;
    HANDLE run_waiting;
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
    common_machine_debug_execute debug;
    void *debug_context;
    lib_u8 state_byte;
    LONG state_reads;
    LONG state_writes;
    LONG defer_state_read;
    LONG state_read_waiting;
    LONG state_read_ready;
    common_machine_state_writer deferred_state_writer;
    LONG running_notifications;
    LONG paused_notifications;
    LONG media_calls;
    lib_storage_medium_mode media_mode;
    char media_path[COMMON_MACHINE_PATH_CAPACITY];
    LONG fail_frame;
    LONG fail_on_read_wake;
    LONG fail_after_read_arm;
    LONG fail_run;
} machine_fake;

void machine_fake_initialize(machine_fake *fake, common_machine_driver *driver);
void machine_fake_dispose(machine_fake *fake);
void machine_fake_note_state(void *opaque, common_machine_state state,
    lib_u32 generation);
void machine_fake_note_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation);

#endif
