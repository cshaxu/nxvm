#include "type.h"

#include "core/platform/sleep.h"
#include "core/platform/wait_interface.h"

/* A host wait polls often enough for lifecycle cancellation, but it must not
 * turn one UI cadence wait into many scheduler-granularity Sleep(1) calls. */
#define CORE_PLATFORM_WAIT_POLL_MILLISECONDS 20u

core_platform_wait_result core_platform_wait_milliseconds(uint32_t milliseconds,
    core_platform_wait_cancel_predicate cancelled, C_VOID *context)
{
    uint32_t waited = 0u;

    if (cancelled == STD_NULL) return CORE_PLATFORM_WAIT_INVALID_ARGUMENT;
    while (waited < milliseconds) {
        uint32_t interval;

        if (cancelled(context)) return CORE_PLATFORM_WAIT_CANCELLED;
        interval = milliseconds - waited;
        if (interval > CORE_PLATFORM_WAIT_POLL_MILLISECONDS) {
            interval = CORE_PLATFORM_WAIT_POLL_MILLISECONDS;
        }
        core_platform_sleep_milliseconds(interval);
        waited += interval;
    }
    return cancelled(context) ? CORE_PLATFORM_WAIT_CANCELLED :
        CORE_PLATFORM_WAIT_COMPLETED;
}
