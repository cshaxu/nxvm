#include "type.h"

#include "core/machine/timeline.h"

static C_INT core_machine_timeline_precedes(const core_machine_timeline_event *left,
    const core_machine_timeline_event *right)
{
    return left->due_tick < right->due_tick ||
        (left->due_tick == right->due_tick && left->sequence < right->sequence);
}

static C_VOID core_machine_timeline_heap_swap(core_machine_timeline *timeline,
    type_unsigned_32 left, type_unsigned_32 right)
{
    type_unsigned_32 slot = timeline->heap[left];

    timeline->heap[left] = timeline->heap[right];
    timeline->heap[right] = slot;
    timeline->events[timeline->heap[left]].heap_index = left;
    timeline->events[timeline->heap[right]].heap_index = right;
}

static C_VOID core_machine_timeline_heap_up(core_machine_timeline *timeline,
    type_unsigned_32 index)
{
    while (index != 0u) {
        type_unsigned_32 parent = (index - 1u) / 2u;

        if (!core_machine_timeline_precedes(&timeline->events[timeline->heap[index]],
                &timeline->events[timeline->heap[parent]])) break;
        core_machine_timeline_heap_swap(timeline, index, parent);
        index = parent;
    }
}

static C_VOID core_machine_timeline_heap_down(core_machine_timeline *timeline,
    type_unsigned_32 index)
{
    for (;;) {
        type_unsigned_32 left = index * 2u + 1u;
        type_unsigned_32 right = left + 1u;
        type_unsigned_32 smallest = index;

        if (left < timeline->heap_size && core_machine_timeline_precedes(
                &timeline->events[timeline->heap[left]],
                &timeline->events[timeline->heap[smallest]])) smallest = left;
        if (right < timeline->heap_size && core_machine_timeline_precedes(
                &timeline->events[timeline->heap[right]],
                &timeline->events[timeline->heap[smallest]])) smallest = right;
        if (smallest == index) return;
        core_machine_timeline_heap_swap(timeline, index, smallest);
        index = smallest;
    }
}

static C_VOID core_machine_timeline_heap_remove(core_machine_timeline *timeline,
    type_unsigned_32 index)
{
    type_unsigned_32 last;

    if (timeline == STD_NULL || index >= timeline->heap_size) return;
    last = --timeline->heap_size;
    if (index == last) return;
    timeline->heap[index] = timeline->heap[last];
    timeline->events[timeline->heap[index]].heap_index = index;
    if (index != 0u && core_machine_timeline_precedes(
            &timeline->events[timeline->heap[index]],
            &timeline->events[timeline->heap[(index - 1u) / 2u]])) {
        core_machine_timeline_heap_up(timeline, index);
    } else {
        core_machine_timeline_heap_down(timeline, index);
    }
}

type_status core_machine_timeline_initialize(core_machine_timeline *timeline)
{
    if (timeline == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    core_machine_timeline_reset(timeline);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_timeline_reset(core_machine_timeline *timeline)
{
    if (timeline != STD_NULL) STD_MEMSET(timeline, 0, sizeof(*timeline));
}

type_status core_machine_timeline_schedule(core_machine_timeline *timeline,
    type_unsigned_64 due_tick, core_machine_timeline_callback callback,
    C_VOID *context, core_machine_timeline_token *out_token)
{
    type_unsigned_32 slot;

    if (timeline == STD_NULL || callback == STD_NULL || out_token == STD_NULL ||
        due_tick < timeline->now || timeline->next_sequence == UINT64_MAX) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (slot = 0u; slot < CORE_MACHINE_TIMELINE_EVENT_CAPACITY; ++slot) {
        if (!timeline->events[slot].active) {
            core_machine_timeline_event *event = &timeline->events[slot];

            event->due_tick = due_tick;
            event->sequence = timeline->next_sequence++;
            event->callback = callback;
            event->context = context;
            event->heap_index = timeline->heap_size;
            event->active = TYPE_TRUE;
            timeline->heap[timeline->heap_size++] = slot;
            core_machine_timeline_heap_up(timeline, event->heap_index);
            out_token->slot = slot;
            out_token->sequence = event->sequence;
            return TYPE_STATUS_OK;
        }
    }
    return TYPE_STATUS_NO_MEMORY;
}

type_status core_machine_timeline_cancel(core_machine_timeline *timeline,
    const core_machine_timeline_token *token)
{
    core_machine_timeline_event *event;

    if (timeline == STD_NULL || token == STD_NULL ||
        token->slot >= CORE_MACHINE_TIMELINE_EVENT_CAPACITY) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    event = &timeline->events[token->slot];
    if (!event->active || event->sequence != token->sequence) {
        return TYPE_STATUS_INVALID_STATE;
    }
    core_machine_timeline_heap_remove(timeline, event->heap_index);
    event->active = TYPE_FALSE;
    event->callback = STD_NULL;
    event->context = STD_NULL;
    return TYPE_STATUS_OK;
}

type_status core_machine_timeline_advance(core_machine_timeline *timeline,
    type_unsigned_64 target_tick)
{
    if (timeline == STD_NULL || target_tick < timeline->now) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (;;) {
        core_machine_timeline_event *next;

        if (timeline->heap_size == 0u) break;
        next = &timeline->events[timeline->heap[0]];
        if (next->due_tick > target_tick) break;
        {
            core_machine_timeline_callback callback = next->callback;
            C_VOID *context = next->context;
            type_unsigned_64 due_tick = next->due_tick;

            core_machine_timeline_heap_remove(timeline, 0u);
            next->active = TYPE_FALSE;
            next->callback = STD_NULL;
            next->context = STD_NULL;
            timeline->now = due_tick;
            callback(context, due_tick);
        }
    }
    timeline->now = target_tick;
    return TYPE_STATUS_OK;
}

type_status core_machine_timeline_next_due(const core_machine_timeline *timeline,
    type_unsigned_64 *out_due_tick)
{
    if (timeline == STD_NULL || out_due_tick == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (timeline->heap_size == 0u) return TYPE_STATUS_INVALID_STATE;
    *out_due_tick = timeline->events[timeline->heap[0]].due_tick;
    return TYPE_STATUS_OK;
}

type_unsigned_32 core_machine_timeline_pending_count(
    const core_machine_timeline *timeline)
{
    type_unsigned_32 count = 0u;
    type_unsigned_32 index;

    if (timeline == STD_NULL) return 0u;
    for (index = 0u; index < CORE_MACHINE_TIMELINE_EVENT_CAPACITY; ++index) {
        if (timeline->events[index].active) ++count;
    }
    return count;
}
