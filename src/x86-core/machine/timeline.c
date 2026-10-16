#include "lib/types/types_interface.h"

#include "x86-core/machine/timeline.h"

static lib_i32 x86_core_machine_timeline_precedes(const x86_core_machine_timeline_event *left,
    const x86_core_machine_timeline_event *right)
{
    return left->due_tick < right->due_tick ||
        (left->due_tick == right->due_tick && left->sequence < right->sequence);
}

static void x86_core_machine_timeline_heap_swap(x86_core_machine_timeline *timeline,
    lib_u32 left, lib_u32 right)
{
    lib_u32 slot = timeline->heap[left];

    timeline->heap[left] = timeline->heap[right];
    timeline->heap[right] = slot;
    timeline->events[timeline->heap[left]].heap_index = left;
    timeline->events[timeline->heap[right]].heap_index = right;
}

static void x86_core_machine_timeline_heap_up(x86_core_machine_timeline *timeline,
    lib_u32 index)
{
    while (index != 0u) {
        lib_u32 parent = (index - 1u) / 2u;

        if (!x86_core_machine_timeline_precedes(&timeline->events[timeline->heap[index]],
                &timeline->events[timeline->heap[parent]])) break;
        x86_core_machine_timeline_heap_swap(timeline, index, parent);
        index = parent;
    }
}

static void x86_core_machine_timeline_heap_down(x86_core_machine_timeline *timeline,
    lib_u32 index)
{
    for (;;) {
        lib_u32 left = index * 2u + 1u;
        lib_u32 right = left + 1u;
        lib_u32 smallest = index;

        if (left < timeline->heap_size && x86_core_machine_timeline_precedes(
                &timeline->events[timeline->heap[left]],
                &timeline->events[timeline->heap[smallest]])) smallest = left;
        if (right < timeline->heap_size && x86_core_machine_timeline_precedes(
                &timeline->events[timeline->heap[right]],
                &timeline->events[timeline->heap[smallest]])) smallest = right;
        if (smallest == index) return;
        x86_core_machine_timeline_heap_swap(timeline, index, smallest);
        index = smallest;
    }
}

static void x86_core_machine_timeline_heap_remove(x86_core_machine_timeline *timeline,
    lib_u32 index)
{
    lib_u32 last;

    if (timeline == LIB_NULL || index >= timeline->heap_size) return;
    last = --timeline->heap_size;
    if (index == last) return;
    timeline->heap[index] = timeline->heap[last];
    timeline->events[timeline->heap[index]].heap_index = index;
    if (index != 0u && x86_core_machine_timeline_precedes(
            &timeline->events[timeline->heap[index]],
            &timeline->events[timeline->heap[(index - 1u) / 2u]])) {
        x86_core_machine_timeline_heap_up(timeline, index);
    } else {
        x86_core_machine_timeline_heap_down(timeline, index);
    }
}

lib_status x86_core_machine_timeline_initialize(x86_core_machine_timeline *timeline)
{
    if (timeline == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    x86_core_machine_timeline_reset(timeline);
    return LIB_STATUS_OK;
}

void x86_core_machine_timeline_reset(x86_core_machine_timeline *timeline)
{
    if (timeline != LIB_NULL) lib_memory_set(timeline, 0, sizeof(*timeline));
}

lib_status x86_core_machine_timeline_schedule(x86_core_machine_timeline *timeline,
    lib_u64 due_tick, x86_core_machine_timeline_callback callback,
    void *context, x86_core_machine_timeline_token *out_token)
{
    lib_u32 slot;

    if (timeline == LIB_NULL || callback == LIB_NULL || out_token == LIB_NULL ||
        due_tick < timeline->now || timeline->next_sequence == LIB_UINT64_MAX) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (slot = 0u; slot < X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY; ++slot) {
        if (!timeline->events[slot].active) {
            x86_core_machine_timeline_event *event = &timeline->events[slot];

            event->due_tick = due_tick;
            event->sequence = timeline->next_sequence++;
            event->callback = callback;
            event->context = context;
            event->heap_index = timeline->heap_size;
            event->active = LIB_TRUE;
            timeline->heap[timeline->heap_size++] = slot;
            x86_core_machine_timeline_heap_up(timeline, event->heap_index);
            out_token->slot = slot;
            out_token->sequence = event->sequence;
            return LIB_STATUS_OK;
        }
    }
    return LIB_STATUS_NO_MEMORY;
}

lib_status x86_core_machine_timeline_cancel(x86_core_machine_timeline *timeline,
    const x86_core_machine_timeline_token *token)
{
    x86_core_machine_timeline_event *event;

    if (timeline == LIB_NULL || token == LIB_NULL ||
        token->slot >= X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    event = &timeline->events[token->slot];
    if (!event->active || event->sequence != token->sequence) {
        return LIB_STATUS_INVALID_STATE;
    }
    x86_core_machine_timeline_heap_remove(timeline, event->heap_index);
    event->active = LIB_FALSE;
    event->callback = LIB_NULL;
    event->context = LIB_NULL;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_timeline_advance(x86_core_machine_timeline *timeline,
    lib_u64 target_tick)
{
    if (timeline == LIB_NULL || target_tick < timeline->now) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (;;) {
        x86_core_machine_timeline_event *next;

        if (timeline->heap_size == 0u) break;
        next = &timeline->events[timeline->heap[0]];
        if (next->due_tick > target_tick) break;
        {
            x86_core_machine_timeline_callback callback = next->callback;
            void *context = next->context;
            lib_u64 due_tick = next->due_tick;

            x86_core_machine_timeline_heap_remove(timeline, 0u);
            next->active = LIB_FALSE;
            next->callback = LIB_NULL;
            next->context = LIB_NULL;
            timeline->now = due_tick;
            callback(context, due_tick);
        }
    }
    timeline->now = target_tick;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_timeline_next_due(const x86_core_machine_timeline *timeline,
    lib_u64 *out_due_tick)
{
    if (timeline == LIB_NULL || out_due_tick == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (timeline->heap_size == 0u) return LIB_STATUS_INVALID_STATE;
    *out_due_tick = timeline->events[timeline->heap[0]].due_tick;
    return LIB_STATUS_OK;
}

lib_u32 x86_core_machine_timeline_pending_count(
    const x86_core_machine_timeline *timeline)
{
    lib_u32 count = 0u;
    lib_u32 index;

    if (timeline == LIB_NULL) return 0u;
    for (index = 0u; index < X86_CORE_MACHINE_TIMELINE_EVENT_CAPACITY; ++index) {
        if (timeline->events[index].active) ++count;
    }
    return count;
}
