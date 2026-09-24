#include "lib/types/types_interface.h"
#include "app-nxvm/machine/executor_state.h"

struct vm_machine_executor_state {
    lib_atomic_i32 active;
    lib_atomic_i32 reset_requested;
};

lib_status vm_machine_executor_state_create(vm_machine_executor_state **out_state)
{
    vm_machine_executor_state *state;

    if (out_state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_state = LIB_NULL;
    state = lib_allocate_zero(1u, sizeof(*state));
    if (state == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_atomic_i32_initialize(&state->active, LIB_FALSE);
    lib_atomic_i32_initialize(&state->reset_requested, LIB_FALSE);
    *out_state = state;
    return LIB_STATUS_OK;
}

void vm_machine_executor_state_destroy(vm_machine_executor_state *state)
{
    lib_release(state);
}

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

lib_i32 vm_machine_executor_state_take_reset(vm_machine_executor_state *state)
{
    return state != LIB_NULL && lib_atomic_i32_exchange_explicit(&state->reset_requested, LIB_FALSE, LIB_MEMORY_ORDER_ACQ_REL);
}

lib_i32 vm_machine_executor_state_is_active(const vm_machine_executor_state *state)
{ return state != LIB_NULL && lib_atomic_i32_load_explicit(&state->active, LIB_MEMORY_ORDER_ACQUIRE); }
