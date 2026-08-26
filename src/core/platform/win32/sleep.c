#include "type.h"

#include "core/platform/sleep.h"


#include <windows.h>

C_VOID core_platform_sleep_milliseconds(type_unsigned_32 milliseconds)
{
    Sleep(milliseconds);
}

C_VOID core_platform_yield(C_VOID)
{
    Sleep(0u);
}
