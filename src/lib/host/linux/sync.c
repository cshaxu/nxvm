#include "lib/host/sync.h"

#include <sched.h>
#include <unistd.h>

C_VOID host_sync_sleep_milliseconds(type_unsigned_32 milliseconds)
{
    usleep(milliseconds * 1000u);
}

C_VOID host_sync_yield(C_VOID)
{
    sched_yield();
}
