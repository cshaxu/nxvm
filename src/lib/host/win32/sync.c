#include "lib/base/base.h"
#include "lib/host/sync.h"

#include <windows.h>

void host_sync_sleep_milliseconds(lib_u32 milliseconds)
{
    Sleep(milliseconds);
}

void host_sync_yield(void)
{
    Sleep(0u);
}
