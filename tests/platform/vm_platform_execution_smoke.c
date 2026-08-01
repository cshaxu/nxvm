#include <stdio.h>

#include "vm/platform/execution.h"

typedef struct vm_platform_execution_smoke_state {
    int running;
    int flip;
    unsigned starts;
    unsigned stops;
} vm_platform_execution_smoke_state;

static int vm_platform_execution_smoke_is_running(void *context)
{
    return ((vm_platform_execution_smoke_state *)context)->running;
}

static int vm_platform_execution_smoke_get_flip(void *context)
{
    return ((vm_platform_execution_smoke_state *)context)->flip;
}

static void vm_platform_execution_smoke_start(void *context)
{
    vm_platform_execution_smoke_state *state = context;

    state->starts += 1u;
    state->running = 1;
    state->flip = 1;
}

static void vm_platform_execution_smoke_stop(void *context)
{
    vm_platform_execution_smoke_state *state = context;

    state->stops += 1u;
    state->running = 0;
}

int main(void)
{
    vm_platform_execution_smoke_state state = {0};
    vm_platform_execution_sink sink = {
        vm_platform_execution_smoke_is_running,
        vm_platform_execution_smoke_get_flip,
        vm_platform_execution_smoke_start,
        vm_platform_execution_smoke_stop
    };

    vm_platform_execution_bind(&sink, &state);
    vm_platform_execution_start();
    if (!vm_platform_execution_is_running() ||
        vm_platform_execution_get_flip() != 1 || state.starts != 1u) {
        return 1;
    }
    vm_platform_execution_stop();
    if (vm_platform_execution_is_running() || state.stops != 1u) {
        return 1;
    }
    vm_platform_execution_bind(NULL, NULL);
    if (vm_platform_execution_is_running() ||
        vm_platform_execution_get_flip() != 0) {
        return 1;
    }
    puts("M5:T14:S3:VM-PLATFORM-EXECUTION:OK");
    return 0;
}
