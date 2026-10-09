#include "lib/types/types_interface.h"
#include "core/machine/control.h"

void vm_machine_executor_state_start(vm_machine_executor_state *state)
{
    if (state == LIB_NULL) return;
    if (lib_atomic_i32_load_explicit(&state->active, LIB_MEMORY_ORDER_ACQUIRE)) return;
    lib_atomic_i32_store_explicit(&state->reset_requested, LIB_FALSE, LIB_MEMORY_ORDER_RELEASE);
    lib_atomic_i32_store_explicit(&state->active, LIB_TRUE, LIB_MEMORY_ORDER_RELEASE);
}

void vm_machine_executor_state_stop(vm_machine_executor_state *state)
{
    if (state == LIB_NULL) return;
    lib_atomic_i32_store_explicit(&state->active, LIB_FALSE, LIB_MEMORY_ORDER_RELEASE);
    lib_atomic_i32_store_explicit(&state->reset_requested, LIB_FALSE, LIB_MEMORY_ORDER_RELEASE);
}

void vm_machine_executor_state_request_reset(vm_machine_executor_state *state)
{
    if (state != LIB_NULL) lib_atomic_i32_store_explicit(&state->reset_requested, LIB_TRUE, LIB_MEMORY_ORDER_RELEASE);
}

lib_bool vm_machine_executor_state_take_reset(vm_machine_executor_state *state)
{
    return state != LIB_NULL && lib_atomic_i32_exchange_explicit(&state->reset_requested, LIB_FALSE, LIB_MEMORY_ORDER_ACQ_REL);
}

lib_bool vm_machine_executor_state_is_active(const vm_machine_executor_state *state)
{ return state != LIB_NULL && lib_atomic_i32_load_explicit(&state->active, LIB_MEMORY_ORDER_ACQUIRE); }
