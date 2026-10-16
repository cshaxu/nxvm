#include "lib/types/types_interface.h"
#include "x86-nxvm/machine/control.h"

void x86_nxvm_machine_executor_state_start(x86_nxvm_machine_executor_state *state)
{
    if (state == LIB_NULL) return;
    if (lib_atomic_i32_load_explicit(&state->active, LIB_MEMORY_ORDER_ACQUIRE)) return;
    lib_atomic_i32_store_explicit(&state->reset_requested, LIB_FALSE, LIB_MEMORY_ORDER_RELEASE);
    lib_atomic_i32_store_explicit(&state->active, LIB_TRUE, LIB_MEMORY_ORDER_RELEASE);
}

void x86_nxvm_machine_executor_state_stop(x86_nxvm_machine_executor_state *state)
{
    if (state == LIB_NULL) return;
    lib_atomic_i32_store_explicit(&state->active, LIB_FALSE, LIB_MEMORY_ORDER_RELEASE);
    lib_atomic_i32_store_explicit(&state->reset_requested, LIB_FALSE, LIB_MEMORY_ORDER_RELEASE);
}

void x86_nxvm_machine_executor_state_request_reset(x86_nxvm_machine_executor_state *state)
{
    if (state != LIB_NULL) lib_atomic_i32_store_explicit(&state->reset_requested, LIB_TRUE, LIB_MEMORY_ORDER_RELEASE);
}

lib_bool x86_nxvm_machine_executor_state_take_reset(x86_nxvm_machine_executor_state *state)
{
    return state != LIB_NULL && lib_atomic_i32_exchange_explicit(&state->reset_requested, LIB_FALSE, LIB_MEMORY_ORDER_ACQ_REL);
}

lib_bool x86_nxvm_machine_executor_state_is_active(const x86_nxvm_machine_executor_state *state)
{ return state != LIB_NULL && lib_atomic_i32_load_explicit(&state->active, LIB_MEMORY_ORDER_ACQUIRE); }
