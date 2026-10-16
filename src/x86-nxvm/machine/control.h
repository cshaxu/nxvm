#ifndef X86_NXVM_MACHINE_CONTROL_H
#define X86_NXVM_MACHINE_CONTROL_H
#include "lib/types/types_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_nxvm_machine x86_nxvm_machine;

typedef struct x86_nxvm_machine_executor_state {
    lib_atomic_i32 active;
    lib_atomic_i32 reset_requested;
} x86_nxvm_machine_executor_state;

typedef struct x86_nxvm_machine_control_state {
    x86_nxvm_machine_executor_state state;
    x86_nxvm_machine *machine;
} x86_nxvm_machine_control_state;

#include "x86-nxvm/machine/machine_interface.h"

void x86_nxvm_machine_executor_state_start(x86_nxvm_machine_executor_state *state);
void x86_nxvm_machine_executor_state_stop(x86_nxvm_machine_executor_state *state);
void x86_nxvm_machine_executor_state_request_reset(x86_nxvm_machine_executor_state *state);
lib_bool x86_nxvm_machine_executor_state_take_reset(x86_nxvm_machine_executor_state *state);
lib_bool x86_nxvm_machine_executor_state_is_active(const x86_nxvm_machine_executor_state *state);

void x86_nxvm_machine_control_start(x86_nxvm_machine_control_state *control);
lib_status x86_nxvm_machine_control_reset(x86_nxvm_machine_control_state *control);
void x86_nxvm_machine_control_stop(x86_nxvm_machine_control_state *control);
/* End the host executor without injecting a Core stop request. */
void x86_nxvm_machine_control_end_run(x86_nxvm_machine_control_state *control);
lib_status x86_nxvm_machine_control_reset_at_boundary(x86_nxvm_machine_control_state *control);
void x86_nxvm_machine_control_refresh_debug(x86_nxvm_machine_control_state *control);
lib_status x86_nxvm_machine_control_initialize(x86_nxvm_machine_control_state *control,
    x86_nxvm_machine *machine);
void x86_nxvm_machine_control_finalize(x86_nxvm_machine_control_state *control,
    x86_nxvm_machine *machine);
lib_bool x86_nxvm_machine_control_is_running(const x86_nxvm_machine_control_state *control);

#ifdef __cplusplus
}
#endif

#endif
