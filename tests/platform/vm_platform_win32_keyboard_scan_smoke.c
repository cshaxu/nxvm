#include "type.h"

#include "vm/platform/win32/win32app.h"
#include "vm/platform/win32/win32con.h"

int main(void)
{
    LPARAM window_l_param = ((LPARAM)0x48u << 16) | (1L << 24);

    if (vm_platform_win32app_decode_scan_code(window_l_param) != 0x0148u ||
        vm_platform_win32app_decode_scan_code((LPARAM)0x480000u) != 0x0048u ||
        vm_platform_win32con_decode_scan_code(0x48u, ENHANCED_KEY) != 0x0148u ||
        vm_platform_win32con_decode_scan_code(0x48u, 0u) != 0x0048u) {
        return 1;
    }
    puts("M5:T247:S1:WIN32-E0-SCAN:OK");
    return 0;
}
