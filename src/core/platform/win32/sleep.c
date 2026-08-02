#include "core/platform/sleep.h"

#include <windows.h>

void core_platform_sleep_milliseconds(uint32_t milliseconds)
{
    Sleep(milliseconds);
}
