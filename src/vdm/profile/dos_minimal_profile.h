#ifndef NTVDM64_VDM_DOS_MINIMAL_PROFILE_H
#define NTVDM64_VDM_DOS_MINIMAL_PROFILE_H

#include "type.h"

#include "core/product/runtime/profile_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

const core_product_runtime_profile_descriptor *ntvdm64_dos_minimal_profile_descriptor(C_VOID);

#ifdef __cplusplus
}
#endif

#endif
