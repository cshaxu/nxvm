/* Copyright 2012-2026 Neko. */

#include "type.h"
#include "version.h"

static const C_CHAR *const nxvm_name = "Neko's x86 Virtual Machine";
static const C_CHAR *const nxvm_release = "0.4.015d";
static const C_CHAR *const nxvm_copyright = "Copyright (c) 2012-2026 Neko.";

const C_CHAR *ntvdm64_version_nxvm_name(C_VOID)
{
    return nxvm_name;
}

const C_CHAR *ntvdm64_version_nxvm_release(C_VOID)
{
    return nxvm_release;
}

const C_CHAR *ntvdm64_version_nxvm_copyright(C_VOID)
{
    return nxvm_copyright;
}

const C_CHAR *ntvdm64_version_build_date(C_VOID)
{
    return __DATE__;
}

const C_CHAR *ntvdm64_version_build_time(C_VOID)
{
    return __TIME__;
}

C_INT ntvdm64_version_format_nxvm_banner(C_CHAR *buffer, STD_SIZE_T buffer_size,
    const C_CHAR *build_suffix)
{
    C_INT written;
    const C_INT versioned_suffix = build_suffix != STD_NULL &&
        build_suffix[0] >= '0' && build_suffix[0] <= '9';

    if (buffer == STD_NULL || buffer_size == 0 || build_suffix == STD_NULL ||
        build_suffix[0] == '\0') {
        return 0;
    }

    if (versioned_suffix) {
        written = STD_SNPRINTF(buffer, buffer_size, "%s [%s]\n%s",
            ntvdm64_version_nxvm_name(), build_suffix,
            ntvdm64_version_nxvm_copyright());
    } else {
        written = STD_SNPRINTF(buffer, buffer_size, "%s [%s.%s]\n%s",
            ntvdm64_version_nxvm_name(), ntvdm64_version_nxvm_release(),
            build_suffix, ntvdm64_version_nxvm_copyright());
    }
    return written >= 0 && (STD_SIZE_T)written < buffer_size;
}
