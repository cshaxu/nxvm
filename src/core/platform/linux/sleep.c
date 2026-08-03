#include "type.h"

#include "core/platform/sleep.h"


#include <unistd.h>

C_VOID core_platform_sleep_milliseconds(uint32_t milliseconds)
{
    usleep(milliseconds * 1000u);
}
