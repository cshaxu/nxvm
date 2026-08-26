#include "type.h"

#include "core/platform/sleep.h"


#include <unistd.h>
#include <sched.h>

C_VOID core_platform_sleep_milliseconds(type_unsigned_32 milliseconds)
{
    usleep(milliseconds * 1000u);
}

C_VOID core_platform_yield(C_VOID)
{
    sched_yield();
}
