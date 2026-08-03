#include <stdio.h>
#include <string.h>

#include "version.h"

int main(void)
{
    char banner[160];

    if (!ntvdm64_version_format_nxvm_banner(banner, sizeof(banner), "m5t47")) {
        return 1;
    }
    if (STD_STRCMP(ntvdm64_version_nxvm_name(), "Neko's x86 Virtual Machine") != 0 ||
        STD_STRCMP(ntvdm64_version_nxvm_release(), "0.4.015d") != 0 ||
        STD_STRCMP(ntvdm64_version_nxvm_copyright(),
            "Copyright (c) 2012-2014 Neko.") != 0 ||
        STD_STRCMP(banner, "Neko's x86 Virtual Machine [0.4.015d.m5t47]\n"
            "Copyright (c) 2012-2014 Neko.") != 0) {
        return 1;
    }

    puts("M5:T47:S1:VERSION-FOUNDATION:OK");
    return 0;
}
