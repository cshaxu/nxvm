#ifndef CORE_MACHINE_TRANSACTION_H
#define CORE_MACHINE_TRANSACTION_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum core_machine_transaction_owner {
    CORE_MACHINE_TRANSACTION_OWNER_NONE = 0,
    CORE_MACHINE_TRANSACTION_OWNER_CPU,
    CORE_MACHINE_TRANSACTION_OWNER_DMA,
    CORE_MACHINE_TRANSACTION_OWNER_REFRESH
} core_machine_transaction_owner;

typedef enum core_machine_transaction_kind {
    CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ = 1,
    CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE,
    CORE_MACHINE_TRANSACTION_CPU_PORT_READ,
    CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE,
    CORE_MACHINE_TRANSACTION_DMA_MEMORY_READ,
    CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE,
    CORE_MACHINE_TRANSACTION_DMA_MEMORY_COPY,
    CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE,
    /* ESC command issue is CPU-owned control traffic.  It deliberately is
     * not a HOLD grant: an 80287/80387 asks its paired CPU to move operands
     * through the processor-extension channel. */
    CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND,
    /* The first logical INTA acknowledges the selected PIC request.  Vector
     * delivery remains the CPU interrupt-entry operation that follows. */
    CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE
} core_machine_transaction_kind;

typedef enum core_machine_transaction_phase {
    CORE_MACHINE_TRANSACTION_PHASE_BEGIN = 1,
    CORE_MACHINE_TRANSACTION_PHASE_COMMIT,
    CORE_MACHINE_TRANSACTION_PHASE_CANCEL,
    CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST,
    CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE,
    CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE
} core_machine_transaction_phase;

typedef C_VOID (*core_machine_transaction_trace_callback)(C_VOID *context,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, type_unsigned_32 address,
    type_unsigned_32 value, type_unsigned_32 detail);

typedef struct core_machine_transaction_state {
    core_machine_transaction_owner owner;
    core_machine_transaction_kind kind;
    type_unsigned_32 address;
    type_unsigned_32 value;
    type_unsigned_32 detail;
    type_unsigned_64 committed_count;
    type_unsigned_64 cancelled_count;
    core_machine_transaction_owner hold_owner;
    type_unsigned_32 hold_detail;
    type_bool hold_acknowledged;
    core_machine_transaction_trace_callback trace;
    C_VOID *trace_context;
} core_machine_transaction_state;

C_VOID core_machine_transaction_initialize(core_machine_transaction_state *state);
C_VOID core_machine_transaction_reset(core_machine_transaction_state *state);
C_VOID core_machine_transaction_bind_trace(core_machine_transaction_state *state,
    core_machine_transaction_trace_callback callback, C_VOID *context);
type_status core_machine_transaction_begin(core_machine_transaction_state *state,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    type_unsigned_32 address, type_unsigned_32 value, type_unsigned_32 detail);
C_VOID core_machine_transaction_set_value(core_machine_transaction_state *state,
    type_unsigned_32 value);
C_VOID core_machine_transaction_commit(core_machine_transaction_state *state);
C_VOID core_machine_transaction_cancel(core_machine_transaction_state *state);
type_status core_machine_transaction_hold_request(
    core_machine_transaction_state *state, core_machine_transaction_owner owner,
    type_unsigned_32 detail);
type_status core_machine_transaction_hold_acknowledge(
    core_machine_transaction_state *state, core_machine_transaction_owner owner);
C_VOID core_machine_transaction_hold_release(
    core_machine_transaction_state *state, core_machine_transaction_owner owner);

#ifdef __cplusplus
}
#endif

#endif
