/* Copyright 2012-2026 Neko. */

#ifndef NTVDM64_VERSION_H
#define NTVDM64_VERSION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *ntvdm64_version_nxvm_name(void);
const char *ntvdm64_version_nxvm_release(void);
const char *ntvdm64_version_nxvm_copyright(void);
const char *ntvdm64_version_build_date(void);
const char *ntvdm64_version_build_time(void);

int ntvdm64_version_format_nxvm_banner(char *buffer, size_t buffer_size,
    const char *build_suffix);

#ifdef __cplusplus
}
#endif

#endif
