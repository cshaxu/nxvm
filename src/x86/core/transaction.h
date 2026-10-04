#ifndef CORE_MACHINE_TRANSACTION_H
#define CORE_MACHINE_TRANSACTION_H
#include "lib/types/types_interface.h"
#include "x86/core/trace_interface.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum core_machine_transaction_phase {
    CORE_MACHINE_TRANSACTION_PHASE_BEGIN = 1,
    CORE_MACHINE_TRANSACTION_PHASE_COMMIT,
    CORE_MACHINE_TRANSACTION_PHASE_CANCEL,
    CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST,
    CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE,
    CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE
} core_machine_transaction_phase;

typedef void (*core_machine_transaction_trace_callback)(void *context,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, lib_u32 address,
    lib_u32 value, lib_u32 detail);

typedef struct core_machine_transaction_state {
    core_machine_transaction_owner owner;
    core_machine_transaction_kind kind;
    lib_u32 address;
    lib_u32 value;
    lib_u32 detail;
    lib_u64 committed_count;
    lib_u64 cancelled_count;
    core_machine_transaction_owner hold_owner;
    lib_u32 hold_detail;
    lib_u8 hold_acknowledged;
    core_machine_transaction_trace_callback trace;
    void *trace_context;
} core_machine_transaction_state;

void core_machine_transaction_initialize(core_machine_transaction_state *state);
void core_machine_transaction_reset(core_machine_transaction_state *state);
void core_machine_transaction_bind_trace(core_machine_transaction_state *state,
    core_machine_transaction_trace_callback callback, void *context);
lib_status core_machine_transaction_begin(core_machine_transaction_state *state,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    lib_u32 address, lib_u32 value, lib_u32 detail);
void core_machine_transaction_set_value(core_machine_transaction_state *state,
    lib_u32 value);
void core_machine_transaction_commit(core_machine_transaction_state *state);
void core_machine_transaction_cancel(core_machine_transaction_state *state);
lib_status core_machine_transaction_hold_request(
    core_machine_transaction_state *state, core_machine_transaction_owner owner,
    lib_u32 detail);
lib_status core_machine_transaction_hold_acknowledge(
    core_machine_transaction_state *state, core_machine_transaction_owner owner);
void core_machine_transaction_hold_release(
    core_machine_transaction_state *state, core_machine_transaction_owner owner);

#ifdef __cplusplus
}
#endif

#endif
