#include "type.h"

#include "core/machine/transaction.h"

static C_VOID core_machine_transaction_record(
    core_machine_transaction_state *state,
    core_machine_transaction_phase phase)
{
    if (state != STD_NULL && state->trace != STD_NULL) {
        state->trace(state->trace_context, state->owner, state->kind, phase,
            state->address, state->value, state->detail);
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
        kind > CORE_MACHINE_TRANSACTION_DMA_MEMORY_COPY ||
        state->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE) {
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
