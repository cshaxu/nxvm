#include "lib/types/test.h"
#ifndef COMMON_TEST_MACHINE_FIXTURE_H
#define COMMON_TEST_MACHINE_FIXTURE_H

#include "common/machine/machine_interface.h"
#include "lib/types/win32/test.h"

typedef struct machine_fake {
    lib_win32_handle stopped;
    lib_win32_handle state_stopped;
    lib_win32_handle reset_completed;
    lib_win32_handle wake;
    lib_win32_handle running;
    lib_win32_handle run_waiting;
    lib_win32_handle frame;
    lib_win32_handle input;
    common_machine_executor_callback callback;
    void *callback_context;
    lib_win32_long resets;
    lib_win32_long runs;
    lib_win32_long inputs;
    lib_win32_long debug_calls;
    lib_win32_dword executor_thread;
    lib_win32_handle callback_entered;
    lib_win32_handle callback_release;
    lib_win32_long notifications;
    common_machine_debug_execute debug;
    void *debug_context;
    lib_u8 state_byte;
    lib_win32_long state_reads;
    lib_win32_long state_writes;
    lib_win32_long defer_state_read;
    lib_win32_long state_read_waiting;
    lib_win32_long state_read_ready;
    common_machine_state_writer deferred_state_writer;
    lib_win32_long running_notifications;
    lib_win32_long paused_notifications;
    lib_win32_long media_calls;
    lib_storage_medium_mode media_mode;
    char media_path[COMMON_MACHINE_PATH_CAPACITY];
    lib_win32_long fail_frame;
    lib_win32_long fail_on_read_wake;
    lib_win32_long fail_after_read_arm;
    lib_win32_long fail_run;
} machine_fake;

void machine_fake_initialize(machine_fake *fake, common_machine_driver *driver);
void machine_fake_dispose(machine_fake *fake);
void machine_fake_note_state(void *opaque, common_machine_state state,
    lib_u32 generation);
void machine_fake_note_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation);

#endif
