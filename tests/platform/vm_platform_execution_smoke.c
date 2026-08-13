#include "type.h"



#include "vm/platform/execution.h"

typedef struct vm_platform_execution_smoke_state {
    C_INT running;
    C_INT flip;
    C_UINT starts;
    C_UINT stops;
} vm_platform_execution_smoke_state;

static C_INT vm_platform_execution_smoke_is_running(C_VOID *context)
{
    return ((vm_platform_execution_smoke_state *)context)->running;
}

static C_INT vm_platform_execution_smoke_get_flip(C_VOID *context)
{
    return ((vm_platform_execution_smoke_state *)context)->flip;
}

static C_VOID vm_platform_execution_smoke_start(C_VOID *context)
{
    vm_platform_execution_smoke_state *state = context;

    state->starts += 1u;
    state->running = 1;
    state->flip = 1;
}

static C_VOID vm_platform_execution_smoke_stop(C_VOID *context)
{
    vm_platform_execution_smoke_state *state = context;

    state->stops += 1u;
    state->running = 0;
}

C_INT main(C_VOID)
{
    vm_platform_execution_smoke_state state = {0};
    vm_platform_execution_smoke_state second_state = {0};
    vm_platform_execution_sink sink = {
        vm_platform_execution_smoke_is_running,
        vm_platform_execution_smoke_get_flip,
        vm_platform_execution_smoke_start,
        vm_platform_execution_smoke_stop
    };
    vm_platform_execution_transport transport;
    vm_platform_execution_transport second_transport;

    vm_platform_execution_transport_initialize(&transport, &sink, &state);
    vm_platform_execution_transport_initialize(&second_transport, &sink,
                                               &second_state);
    vm_platform_execution_start_for(&transport);
    vm_platform_execution_start_for(&second_transport);
    vm_platform_execution_stop_for(&transport);
    if (vm_platform_execution_is_running_for(&transport) ||
        !vm_platform_execution_is_running_for(&second_transport) ||
        state.stops != 1u || second_state.starts != 1u) {
        return 1;
    }

    puts("M5:T80:S1:VM-PLATFORM-EXECUTION:OK");
    return 0;
}
