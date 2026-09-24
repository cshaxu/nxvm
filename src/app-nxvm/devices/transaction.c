#include "lib/types/types_interface.h"

#include "app-nxvm/devices/transaction.h"

static void core_machine_transaction_record(
    core_machine_transaction_state *state,
    core_machine_transaction_phase phase)
{
    if (state != LIB_NULL && state->trace != LIB_NULL) {
        core_machine_transaction_owner owner = state->owner;
        lib_u32 detail = state->detail;

        if (phase >= CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST) {
            owner = state->hold_owner;
            detail = state->hold_detail;
        }
        state->trace(state->trace_context, owner, state->kind, phase,
            state->address, state->value, detail);
    }
}

void core_machine_transaction_initialize(core_machine_transaction_state *state)
{
    if (state != LIB_NULL) {
        lib_memory_set(state, 0, sizeof(*state));
    }
}

void core_machine_transaction_reset(core_machine_transaction_state *state)
{
    core_machine_transaction_trace_callback trace;
    void *trace_context;

    if (state == LIB_NULL) {
        return;
    }
    core_machine_transaction_cancel(state);
    core_machine_transaction_hold_release(state, state->hold_owner);
    trace = state->trace;
    trace_context = state->trace_context;
    lib_memory_set(state, 0, sizeof(*state));
    state->trace = trace;
    state->trace_context = trace_context;
}

void core_machine_transaction_bind_trace(core_machine_transaction_state *state,
    core_machine_transaction_trace_callback callback, void *context)
{
    if (state != LIB_NULL) {
        state->trace = callback;
        state->trace_context = context;
    }
}

lib_status core_machine_transaction_begin(core_machine_transaction_state *state,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    lib_u32 address, lib_u32 value, lib_u32 detail)
{
    if (state == LIB_NULL || owner == CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        kind < CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ ||
        kind > CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        (state->hold_owner != CORE_MACHINE_TRANSACTION_OWNER_NONE &&
         (state->hold_owner != owner || !state->hold_acknowledged))) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    state->owner = owner;
    state->kind = kind;
    state->address = address;
    state->value = value;
    state->detail = detail;
    core_machine_transaction_record(state, CORE_MACHINE_TRANSACTION_PHASE_BEGIN);
    return LIB_STATUS_OK;
}

void core_machine_transaction_set_value(core_machine_transaction_state *state,
    lib_u32 value)
{
    if (state != LIB_NULL && state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        state->value = value;
    }
}

void core_machine_transaction_commit(core_machine_transaction_state *state)
{
    if (state == LIB_NULL || state->owner == CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    core_machine_transaction_record(state, CORE_MACHINE_TRANSACTION_PHASE_COMMIT);
    ++state->committed_count;
    state->owner = CORE_MACHINE_TRANSACTION_OWNER_NONE;
}

void core_machine_transaction_cancel(core_machine_transaction_state *state)
{
    if (state == LIB_NULL || state->owner == CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    core_machine_transaction_record(state, CORE_MACHINE_TRANSACTION_PHASE_CANCEL);
    ++state->cancelled_count;
    state->owner = CORE_MACHINE_TRANSACTION_OWNER_NONE;
}

lib_status core_machine_transaction_hold_request(
    core_machine_transaction_state *state, core_machine_transaction_owner owner,
    lib_u32 detail)
{
    if (state == LIB_NULL || owner == CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    state->hold_owner = owner;
    state->hold_detail = detail;
    state->hold_acknowledged = LIB_FALSE;
    core_machine_transaction_record(state,
        CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST);
    return LIB_STATUS_OK;
}

lib_status core_machine_transaction_hold_acknowledge(
    core_machine_transaction_state *state, core_machine_transaction_owner owner)
{
    if (state == LIB_NULL || owner == CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != owner || state->hold_acknowledged) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    state->hold_acknowledged = LIB_TRUE;
    core_machine_transaction_record(state,
        CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE);
    return LIB_STATUS_OK;
}

void core_machine_transaction_hold_release(
    core_machine_transaction_state *state, core_machine_transaction_owner owner)
{
    if (state == LIB_NULL || owner == CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != owner ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    core_machine_transaction_record(state,
        CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE);
    state->hold_owner = CORE_MACHINE_TRANSACTION_OWNER_NONE;
    state->hold_detail = 0u;
    state->hold_acknowledged = LIB_FALSE;
}
