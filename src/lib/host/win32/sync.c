#include "lib/host/sync.h"

#include <windows.h>

C_VOID host_sync_sleep_milliseconds(type_unsigned_32 milliseconds)
{
    Sleep(milliseconds);
}

C_VOID host_sync_yield(C_VOID)
{
    Sleep(0u);
}
