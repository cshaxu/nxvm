#include "type.h"

#include "core/machine/transaction.h"

static C_VOID core_machine_transaction_record(
    core_machine_transaction_state *state,
    core_machine_transaction_phase phase)
{
    if (state != STD_NULL && state->trace != STD_NULL) {
        core_machine_transaction_owner owner = state->owner;
        type_unsigned_32 detail = state->detail;

        if (phase >= CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST) {
            owner = state->hold_owner;
            detail = state->hold_detail;
        }
        state->trace(state->trace_context, owner, state->kind, phase,
            state->address, state->value, detail);
    }
}

C_VOID core_machine_transaction_initialize(core_machine_transaction_state *state)
{
    if (state != STD_NULL) {
        STD_MEMSET(state, 0, sizeof(*state));
    }
}

C_VOID core_machine_transaction_reset(core_machine_transaction_state *state)
{
    core_machine_transaction_trace_callback trace;
    C_VOID *trace_context;

    if (state == STD_NULL) {
        return;
    }
    core_machine_transaction_cancel(state);
    core_machine_transaction_hold_release(state, state->hold_owner);
    trace = state->trace;
    trace_context = state->trace_context;
    STD_MEMSET(state, 0, sizeof(*state));
    state->trace = trace;
    state->trace_context = trace_context;
}

C_VOID core_machine_transaction_bind_trace(core_machine_transaction_state *state,
    core_machine_transaction_trace_callback callback, C_VOID *context)
{
    if (state != STD_NULL) {
        state->trace = callback;
        state->trace_context = context;
    }
}

type_status core_machine_transaction_begin(core_machine_transaction_state *state,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    type_unsigned_32 address, type_unsigned_32 value, type_unsigned_32 detail)
{
    if (state == STD_NULL || owner == CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        kind < CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ ||
        kind > CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        (state->hold_owner != CORE_MACHINE_TRANSACTION_OWNER_NONE &&
         (state->hold_owner != owner || !state->hold_acknowledged))) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    state->owner = owner;
    state->kind = kind;
    state->address = address;
    state->value = value;
    state->detail = detail;
    core_machine_transaction_record(state, CORE_MACHINE_TRANSACTION_PHASE_BEGIN);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_transaction_set_value(core_machine_transaction_state *state,
    type_unsigned_32 value)
{
    if (state != STD_NULL && state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        state->value = value;
    }
}

C_VOID core_machine_transaction_commit(core_machine_transaction_state *state)
{
    if (state == STD_NULL || state->owner == CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    core_machine_transaction_record(state, CORE_MACHINE_TRANSACTION_PHASE_COMMIT);
    ++state->committed_count;
    state->owner = CORE_MACHINE_TRANSACTION_OWNER_NONE;
}

C_VOID core_machine_transaction_cancel(core_machine_transaction_state *state)
{
    if (state == STD_NULL || state->owner == CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    core_machine_transaction_record(state, CORE_MACHINE_TRANSACTION_PHASE_CANCEL);
    ++state->cancelled_count;
    state->owner = CORE_MACHINE_TRANSACTION_OWNER_NONE;
}

type_status core_machine_transaction_hold_request(
    core_machine_transaction_state *state, core_machine_transaction_owner owner,
    type_unsigned_32 detail)
{
    if (state == STD_NULL || owner == CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    state->hold_owner = owner;
    state->hold_detail = detail;
    state->hold_acknowledged = TYPE_FALSE;
    core_machine_transaction_record(state,
        CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST);
    return TYPE_STATUS_OK;
}

type_status core_machine_transaction_hold_acknowledge(
    core_machine_transaction_state *state, core_machine_transaction_owner owner)
{
    if (state == STD_NULL || owner == CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != owner || state->hold_acknowledged) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    state->hold_acknowledged = TYPE_TRUE;
    core_machine_transaction_record(state,
        CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_transaction_hold_release(
    core_machine_transaction_state *state, core_machine_transaction_owner owner)
{
    if (state == STD_NULL || owner == CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != owner ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    core_machine_transaction_record(state,
        CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE);
    state->hold_owner = CORE_MACHINE_TRANSACTION_OWNER_NONE;
    state->hold_detail = 0u;
    state->hold_acknowledged = TYPE_FALSE;
}
