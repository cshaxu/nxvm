#include "type.h"

#include "core/platform/sleep.h"


#include <windows.h>

C_VOID core_platform_sleep_milliseconds(uint32_t milliseconds)
{
    Sleep(milliseconds);
}
