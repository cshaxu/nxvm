#ifndef CORE_MACHINE_TIMELINE_H
#define CORE_MACHINE_TIMELINE_H

#include "type.h"

#define CORE_MACHINE_TIMELINE_EVENT_CAPACITY 32u

typedef C_VOID (*core_machine_timeline_callback)(C_VOID *context,
    type_unsigned_64 due_tick);

typedef struct core_machine_timeline_token {
    type_unsigned_32 slot;
    type_unsigned_64 sequence;
} core_machine_timeline_token;

typedef struct core_machine_timeline_event {
    type_unsigned_64 due_tick;
    type_unsigned_64 sequence;
    core_machine_timeline_callback callback;
    C_VOID *context;
    type_unsigned_32 heap_index;
    type_bool active;
} core_machine_timeline_event;

typedef struct core_machine_timeline {
    type_unsigned_64 now;
    type_unsigned_64 next_sequence;
    type_unsigned_32 heap_size;
    type_unsigned_32 heap[CORE_MACHINE_TIMELINE_EVENT_CAPACITY];
    core_machine_timeline_event events[CORE_MACHINE_TIMELINE_EVENT_CAPACITY];
} core_machine_timeline;

type_status core_machine_timeline_initialize(core_machine_timeline *timeline);
C_VOID core_machine_timeline_reset(core_machine_timeline *timeline);
type_status core_machine_timeline_schedule(core_machine_timeline *timeline,
    type_unsigned_64 due_tick, core_machine_timeline_callback callback,
    C_VOID *context, core_machine_timeline_token *out_token);
type_status core_machine_timeline_cancel(core_machine_timeline *timeline,
    const core_machine_timeline_token *token);
type_status core_machine_timeline_advance(core_machine_timeline *timeline,
    type_unsigned_64 target_tick);
type_status core_machine_timeline_next_due(const core_machine_timeline *timeline,
    type_unsigned_64 *out_due_tick);
type_unsigned_32 core_machine_timeline_pending_count(
    const core_machine_timeline *timeline);

#endif
