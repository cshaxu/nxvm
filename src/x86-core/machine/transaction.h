#ifndef X86_CORE_MACHINE_TRANSACTION_H
#define X86_CORE_MACHINE_TRANSACTION_H
#include "lib/types/types_interface.h"
#include "x86-core/machine/trace_interface.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum x86_core_machine_transaction_phase {
    X86_CORE_MACHINE_TRANSACTION_PHASE_BEGIN = 1,
    X86_CORE_MACHINE_TRANSACTION_PHASE_COMMIT,
    X86_CORE_MACHINE_TRANSACTION_PHASE_CANCEL,
    X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST,
    X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE,
    X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE
} x86_core_machine_transaction_phase;

typedef void (*x86_core_machine_transaction_trace_callback)(void *context,
    x86_core_machine_transaction_owner owner, x86_core_machine_transaction_kind kind,
    x86_core_machine_transaction_phase phase, lib_u32 address,
    lib_u32 value, lib_u32 detail);

typedef struct x86_core_machine_transaction_state {
    x86_core_machine_transaction_owner owner;
    x86_core_machine_transaction_kind kind;
    lib_u32 address;
    lib_u32 value;
    lib_u32 detail;
    lib_u64 committed_count;
    lib_u64 cancelled_count;
    x86_core_machine_transaction_owner hold_owner;
    lib_u32 hold_detail;
    lib_u8 hold_acknowledged;
    x86_core_machine_transaction_trace_callback trace;
    void *trace_context;
} x86_core_machine_transaction_state;

void x86_core_machine_transaction_initialize(x86_core_machine_transaction_state *state);
void x86_core_machine_transaction_reset(x86_core_machine_transaction_state *state);
void x86_core_machine_transaction_bind_trace(x86_core_machine_transaction_state *state,
    x86_core_machine_transaction_trace_callback callback, void *context);
lib_status x86_core_machine_transaction_begin(x86_core_machine_transaction_state *state,
    x86_core_machine_transaction_owner owner, x86_core_machine_transaction_kind kind,
    lib_u32 address, lib_u32 value, lib_u32 detail);
void x86_core_machine_transaction_set_value(x86_core_machine_transaction_state *state,
    lib_u32 value);
void x86_core_machine_transaction_commit(x86_core_machine_transaction_state *state);
void x86_core_machine_transaction_cancel(x86_core_machine_transaction_state *state);
lib_status x86_core_machine_transaction_hold_request(
    x86_core_machine_transaction_state *state, x86_core_machine_transaction_owner owner,
    lib_u32 detail);
lib_status x86_core_machine_transaction_hold_acknowledge(
    x86_core_machine_transaction_state *state, x86_core_machine_transaction_owner owner);
void x86_core_machine_transaction_hold_release(
    x86_core_machine_transaction_state *state, x86_core_machine_transaction_owner owner);

#ifdef __cplusplus
}
#endif

#endif
