#include "lib/types/types_interface.h"

#include "x86-core/machine/transaction.h"

static void x86_core_machine_transaction_record(
    x86_core_machine_transaction_state *state,
    x86_core_machine_transaction_phase phase)
{
    if (state != LIB_NULL && state->trace != LIB_NULL) {
        x86_core_machine_transaction_owner owner = state->owner;
        lib_u32 detail = state->detail;

        if (phase >= X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST) {
            owner = state->hold_owner;
            detail = state->hold_detail;
        }
        state->trace(state->trace_context, owner, state->kind, phase,
            state->address, state->value, detail);
    }
}

void x86_core_machine_transaction_initialize(x86_core_machine_transaction_state *state)
{
    if (state != LIB_NULL) {
        lib_memory_set(state, 0, sizeof(*state));
    }
}

void x86_core_machine_transaction_reset(x86_core_machine_transaction_state *state)
{
    x86_core_machine_transaction_trace_callback trace;
    void *trace_context;

    if (state == LIB_NULL) {
        return;
    }
    x86_core_machine_transaction_cancel(state);
    x86_core_machine_transaction_hold_release(state, state->hold_owner);
    trace = state->trace;
    trace_context = state->trace_context;
    lib_memory_set(state, 0, sizeof(*state));
    state->trace = trace;
    state->trace_context = trace_context;
}

void x86_core_machine_transaction_bind_trace(x86_core_machine_transaction_state *state,
    x86_core_machine_transaction_trace_callback callback, void *context)
{
    if (state != LIB_NULL) {
        state->trace = callback;
        state->trace_context = context;
    }
}

lib_status x86_core_machine_transaction_begin(x86_core_machine_transaction_state *state,
    x86_core_machine_transaction_owner owner, x86_core_machine_transaction_kind kind,
    lib_u32 address, lib_u32 value, lib_u32 detail)
{
    if (state == LIB_NULL || owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        kind < X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ ||
        kind > X86_CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE ||
        state->owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        (state->hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE &&
         (state->hold_owner != owner || !state->hold_acknowledged))) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    state->owner = owner;
    state->kind = kind;
    state->address = address;
    state->value = value;
    state->detail = detail;
    x86_core_machine_transaction_record(state, X86_CORE_MACHINE_TRANSACTION_PHASE_BEGIN);
    return LIB_STATUS_OK;
}

void x86_core_machine_transaction_set_value(x86_core_machine_transaction_state *state,
    lib_u32 value)
{
    if (state != LIB_NULL && state->owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        state->value = value;
    }
}

void x86_core_machine_transaction_commit(x86_core_machine_transaction_state *state)
{
    if (state == LIB_NULL || state->owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    x86_core_machine_transaction_record(state, X86_CORE_MACHINE_TRANSACTION_PHASE_COMMIT);
    ++state->committed_count;
    state->owner = X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
}

void x86_core_machine_transaction_cancel(x86_core_machine_transaction_state *state)
{
    if (state == LIB_NULL || state->owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    x86_core_machine_transaction_record(state, X86_CORE_MACHINE_TRANSACTION_PHASE_CANCEL);
    ++state->cancelled_count;
    state->owner = X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
}

lib_status x86_core_machine_transaction_hold_request(
    x86_core_machine_transaction_state *state, x86_core_machine_transaction_owner owner,
    lib_u32 detail)
{
    if (state == LIB_NULL || owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    state->hold_owner = owner;
    state->hold_detail = detail;
    state->hold_acknowledged = LIB_FALSE;
    x86_core_machine_transaction_record(state,
        X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_transaction_hold_acknowledge(
    x86_core_machine_transaction_state *state, x86_core_machine_transaction_owner owner)
{
    if (state == LIB_NULL || owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != owner || state->hold_acknowledged) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    state->hold_acknowledged = LIB_TRUE;
    x86_core_machine_transaction_record(state,
        X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE);
    return LIB_STATUS_OK;
}

void x86_core_machine_transaction_hold_release(
    x86_core_machine_transaction_state *state, x86_core_machine_transaction_owner owner)
{
    if (state == LIB_NULL || owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        state->hold_owner != owner ||
        state->owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        return;
    }
    x86_core_machine_transaction_record(state,
        X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE);
    state->hold_owner = X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
    state->hold_detail = 0u;
    state->hold_acknowledged = LIB_FALSE;
}
