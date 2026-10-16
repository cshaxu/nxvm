#ifndef X86_CORE_MACHINE_TIMELINE_H
#define X86_CORE_MACHINE_TIMELINE_H
#include "lib/types/types_interface.h"


#define X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY 32u

typedef void (*x86_core_machine_timeline_callback)(void *context,
    lib_u64 due_tick);

typedef struct x86_core_machine_timeline_token {
    lib_u32 slot;
    lib_u64 sequence;
} x86_core_machine_timeline_token;

typedef struct x86_core_machine_timeline_event {
    lib_u64 due_tick;
    lib_u64 sequence;
    x86_core_machine_timeline_callback callback;
    void *context;
    lib_u32 heap_index;
    lib_u8 active;
} x86_core_machine_timeline_event;

typedef struct x86_core_machine_timeline {
    lib_u64 now;
    lib_u64 next_sequence;
    lib_u32 heap_size;
    lib_u32 heap[X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY];
    x86_core_machine_timeline_event events[X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY];
} x86_core_machine_timeline;

lib_status x86_core_machine_timeline_initialize(x86_core_machine_timeline *timeline);
void x86_core_machine_timeline_reset(x86_core_machine_timeline *timeline);
lib_status x86_core_machine_timeline_schedule(x86_core_machine_timeline *timeline,
    lib_u64 due_tick, x86_core_machine_timeline_callback callback,
    void *context, x86_core_machine_timeline_token *out_token);
lib_status x86_core_machine_timeline_cancel(x86_core_machine_timeline *timeline,
    const x86_core_machine_timeline_token *token);
lib_status x86_core_machine_timeline_advance(x86_core_machine_timeline *timeline,
    lib_u64 target_tick);
lib_status x86_core_machine_timeline_next_due(const x86_core_machine_timeline *timeline,
    lib_u64 *out_due_tick);
lib_u32 x86_core_machine_timeline_pending_count(
    const x86_core_machine_timeline *timeline);

#endif
