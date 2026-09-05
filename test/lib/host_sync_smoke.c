#include "type.h"

#include "lib/host/sync.h"

typedef struct host_sync_smoke_state {
    C_UINT calls;
    C_UINT cancel_after;
} host_sync_smoke_state;

static C_INT host_sync_smoke_cancelled(C_VOID *context)
{
    host_sync_smoke_state *state = context;

    if (state == STD_NULL) return TYPE_TRUE;
    ++state->calls;
    return state->calls >= state->cancel_after;
}

C_INT main(C_VOID)
{
    host_sync_smoke_state completed = {0u, 100u};
    host_sync_smoke_state cancelled = {0u, 2u};

    if (host_sync_wait_milliseconds(2u, host_sync_smoke_cancelled, &completed) !=
            HOST_SYNC_WAIT_COMPLETED || completed.calls < 2u ||
        host_sync_wait_milliseconds(5u, host_sync_smoke_cancelled, &cancelled) !=
            HOST_SYNC_WAIT_CANCELLED || cancelled.calls != 2u ||
        host_sync_wait_milliseconds(1u, STD_NULL, STD_NULL) !=
            HOST_SYNC_WAIT_INVALID_ARGUMENT) return 1;
    puts("M5:T522:S7:HOST-SYNC:OK");
    return 0;
}
