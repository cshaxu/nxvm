/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_QDCGA_H
#define VM_PROFILE_DEFAULT_QDCGA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/machine/display_interface.h"

typedef struct vm_profile_default_context vm_profile_default_context;
typedef struct core_machine_firmware_context core_machine_firmware_context;

C_VOID vm_profile_default_cga_reset(vm_profile_default_context *profile,
    core_machine_firmware_context *firmware);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
