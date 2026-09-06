#include "lib/base/base.h"
#include "lib/host/sync.h"

#include <sched.h>
#include <unistd.h>

void host_sync_sleep_milliseconds(lib_u32 milliseconds)
{
    usleep(milliseconds * 1000u);
}

void host_sync_yield(void)
{
    sched_yield();
}
