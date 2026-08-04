/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_QDCGA_H
#define VM_PROFILE_DEFAULT_QDCGA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/machine/display_interface.h"

typedef struct t_qdx t_qdx;
typedef struct vm_profile_default_context vm_profile_default_context;

C_VOID vm_profile_default_cga_initialize(t_qdx *qdx);
C_VOID vm_profile_default_cga_reset(vm_profile_default_context *profile);
C_INT vm_profile_default_display_capture(C_VOID *context,
    core_machine_display_snapshot *out_snapshot);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
