/* Copyright 2012-2026 Neko. */

#ifndef NTVDM64_VERSION_H
#define NTVDM64_VERSION_H

#include "type.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const C_CHAR *ntvdm64_version_nxvm_name(C_VOID);
const C_CHAR *ntvdm64_version_nxvm_release(C_VOID);
const C_CHAR *ntvdm64_version_nxvm_copyright(C_VOID);
const C_CHAR *ntvdm64_version_build_date(C_VOID);
const C_CHAR *ntvdm64_version_build_time(C_VOID);

C_INT ntvdm64_version_format_nxvm_banner(C_CHAR *buffer, size_t buffer_size,
    const C_CHAR *build_suffix);

#ifdef __cplusplus
}
#endif

#endif
