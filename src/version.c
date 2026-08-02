/* Copyright 2012-2026 Neko. */

#include <stdio.h>

#include "version.h"

static const char *const nxvm_name = "Neko's x86 Virtual Machine";
static const char *const nxvm_release = "0.4.015d";
static const char *const nxvm_copyright = "Copyright (c) 2012-2014 Neko.";

const char *ntvdm64_version_nxvm_name(void)
{
    return nxvm_name;
}

const char *ntvdm64_version_nxvm_release(void)
{
    return nxvm_release;
}

const char *ntvdm64_version_nxvm_copyright(void)
{
    return nxvm_copyright;
}

const char *ntvdm64_version_build_date(void)
{
    return __DATE__;
}

const char *ntvdm64_version_build_time(void)
{
    return __TIME__;
}

int ntvdm64_version_format_nxvm_banner(char *buffer, size_t buffer_size,
    const char *build_suffix)
{
    int written;

    if (buffer == NULL || buffer_size == 0 || build_suffix == NULL ||
        build_suffix[0] == '\0') {
        return 0;
    }

    written = snprintf(buffer, buffer_size, "%s [%s.%s]\n%s",
        ntvdm64_version_nxvm_name(), ntvdm64_version_nxvm_release(),
        build_suffix, ntvdm64_version_nxvm_copyright());
    return written >= 0 && (size_t)written < buffer_size;
}
