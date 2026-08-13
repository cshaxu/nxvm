#include "type.h"

#include "core/machine/timeline.h"

static C_INT core_machine_timeline_precedes(const core_machine_timeline_event *left,
    const core_machine_timeline_event *right)
{
    return left->due_tick < right->due_tick ||
        (left->due_tick == right->due_tick && left->sequence < right->sequence);
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
            event->active = TYPE_TRUE;
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
    event->active = TYPE_FALSE;
    event->callback = STD_NULL;
    event->context = STD_NULL;
    return TYPE_STATUS_OK;
}

type_status core_machine_timeline_advance(core_machine_timeline *timeline,
    type_unsigned_64 target_tick)
{
    type_unsigned_32 index;

    if (timeline == STD_NULL || target_tick < timeline->now) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (;;) {
        core_machine_timeline_event *next = STD_NULL;

        for (index = 0u; index < CORE_MACHINE_TIMELINE_EVENT_CAPACITY; ++index) {
            core_machine_timeline_event *candidate = &timeline->events[index];

            if (candidate->active && candidate->due_tick <= target_tick &&
                (next == STD_NULL || core_machine_timeline_precedes(candidate, next))) {
                next = candidate;
            }
        }
        if (next == STD_NULL) break;
        {
            core_machine_timeline_callback callback = next->callback;
            C_VOID *context = next->context;
            type_unsigned_64 due_tick = next->due_tick;

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
