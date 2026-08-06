#include "type.h"

#include "core/platform/wait_interface.h"

typedef struct core_platform_wait_smoke_state {
    C_UINT calls;
    C_UINT cancel_after;
} core_platform_wait_smoke_state;

static C_INT core_platform_wait_smoke_cancelled(C_VOID *context)
{
    core_platform_wait_smoke_state *state = context;

    if (state == STD_NULL) return TYPE_TRUE;
    ++state->calls;
    return state->calls >= state->cancel_after;
}

C_INT main(C_VOID)
{
    core_platform_wait_smoke_state completed = {0u, 100u};
    core_platform_wait_smoke_state cancelled = {0u, 2u};

    if (core_platform_wait_milliseconds(2u,
            core_platform_wait_smoke_cancelled, &completed) !=
        CORE_PLATFORM_WAIT_COMPLETED || completed.calls < 2u) return 1;
    if (core_platform_wait_milliseconds(5u,
            core_platform_wait_smoke_cancelled, &cancelled) !=
        CORE_PLATFORM_WAIT_CANCELLED || cancelled.calls != 2u) return 1;
    if (core_platform_wait_milliseconds(1u, STD_NULL, STD_NULL) !=
        CORE_PLATFORM_WAIT_INVALID_ARGUMENT) return 1;
    puts("M5:T251:S2:CORE-PLATFORM-WAIT:OK");
    return 0;
}
