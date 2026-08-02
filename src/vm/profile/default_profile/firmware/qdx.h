/* Copyright 2012-2014 Neko. */

#ifndef NXVM_QDX_H
#define NXVM_QDX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"
#include "core/machine/cpu_instructions.h"

typedef struct t_qdx {
    t_faddrcc table[0x100];
} t_qdx;

typedef struct vm_profile_default_context vm_profile_default_context;

void vm_profile_default_qdx_initialize(vm_profile_default_context *profile,
    core_machine_cpu_execution_context *execution);
void vm_profile_default_qdx_reset(vm_profile_default_context *profile);
void vm_profile_default_qdx_refresh(vm_profile_default_context *profile);
void vm_profile_default_qdx_finalize(vm_profile_default_context *profile);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
