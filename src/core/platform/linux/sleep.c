#include "type.h"

#include "core/platform/sleep.h"


#include <unistd.h>

C_VOID core_platform_sleep_milliseconds(type_unsigned_32 milliseconds)
{
    usleep(milliseconds * 1000u);
}
