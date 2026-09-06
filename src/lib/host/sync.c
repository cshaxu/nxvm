#include "lib/base/base.h"
#include "lib/host/sync.h"

#define HOST_SYNC_WAIT_POLL_MILLISECONDS 20u

host_sync_wait_result host_sync_wait_milliseconds(lib_u32 milliseconds,
    host_sync_cancel_predicate cancelled, void *context)
{
    lib_u32 waited = 0u;

    if (cancelled == LIB_NULL) return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    while (waited < milliseconds) {
        lib_u32 interval;

        if (cancelled(context)) return HOST_SYNC_WAIT_CANCELLED;
        interval = milliseconds - waited;
        if (interval > HOST_SYNC_WAIT_POLL_MILLISECONDS) {
            interval = HOST_SYNC_WAIT_POLL_MILLISECONDS;
        }
        host_sync_sleep_milliseconds(interval);
        waited += interval;
    }
    return cancelled(context) ? HOST_SYNC_WAIT_CANCELLED : HOST_SYNC_WAIT_COMPLETED;
}
