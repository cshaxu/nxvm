#include "type.h"



#include "vm/platform/execution.h"

typedef struct vm_platform_execution_smoke_state {
    vm_platform_execution_lifecycle lifecycle;
    C_INT flip;
    C_UINT starts;
    C_UINT stops;
} vm_platform_execution_smoke_state;

static vm_platform_execution_lifecycle vm_platform_execution_smoke_get_lifecycle(
    C_VOID *context)
{
    return ((vm_platform_execution_smoke_state *)context)->lifecycle;
}

static C_INT vm_platform_execution_smoke_get_flip(C_VOID *context)
{
    return ((vm_platform_execution_smoke_state *)context)->flip;
}

static C_VOID vm_platform_execution_smoke_start(C_VOID *context)
{
    vm_platform_execution_smoke_state *state = context;

    state->starts += 1u;
    state->lifecycle = VM_PLATFORM_EXECUTION_RUNNING;
    state->flip = 1;
}

static C_VOID vm_platform_execution_smoke_stop(C_VOID *context)
{
    vm_platform_execution_smoke_state *state = context;

    state->stops += 1u;
    state->lifecycle = VM_PLATFORM_EXECUTION_STOPPED;
}

C_INT main(C_VOID)
{
    vm_platform_execution_smoke_state state = {0};
    vm_platform_execution_smoke_state second_state = {0};
    vm_platform_execution_sink sink = {
        vm_platform_execution_smoke_get_lifecycle,
        vm_platform_execution_smoke_get_flip,
        vm_platform_execution_smoke_start,
        vm_platform_execution_smoke_stop
    };
    vm_platform_execution_transport *transport = STD_NULL;
    vm_platform_execution_transport *second_transport = STD_NULL;

    if (vm_platform_execution_transport_create(&sink, &state, &transport) !=
            TYPE_STATUS_OK || vm_platform_execution_transport_create(&sink,
            &second_state, &second_transport) != TYPE_STATUS_OK) goto fail;
    vm_platform_execution_start_for(transport);
    vm_platform_execution_start_for(second_transport);
    vm_platform_execution_stop_for(transport);
    if (vm_platform_execution_is_running_for(transport) ||
        !vm_platform_execution_is_running_for(second_transport) ||
        state.stops != 1u || second_state.starts != 1u) {
        goto fail;
    }
    second_state.lifecycle = VM_PLATFORM_EXECUTION_PAUSED;
    if (vm_platform_execution_is_running_for(second_transport) ||
        vm_platform_execution_get_lifecycle_for(second_transport) !=
            VM_PLATFORM_EXECUTION_PAUSED ||
        vm_platform_execution_get_lifecycle_for(transport) !=
            VM_PLATFORM_EXECUTION_STOPPED) goto fail;

    vm_platform_execution_transport_destroy(second_transport);
    vm_platform_execution_transport_destroy(transport);
    puts("M5:T80:S1:VM-PLATFORM-EXECUTION:OK");
    return 0;

fail:
    vm_platform_execution_transport_destroy(second_transport);
    vm_platform_execution_transport_destroy(transport);
    return 1;
}
