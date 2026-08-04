/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_QDX_H
#define VM_PROFILE_DEFAULT_QDX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/cpu_instructions.h"

typedef struct vm_profile_default_context vm_profile_default_context;
typedef C_VOID (*vm_profile_default_qdx_handler)(
    vm_profile_default_context *profile);

typedef struct t_qdx {
    vm_profile_default_qdx_handler table[0x100];
} t_qdx;

C_VOID vm_profile_default_qdx_initialize(vm_profile_default_context *profile,
    core_machine_cpu_execution_context *execution);
C_VOID vm_profile_default_qdx_reset(vm_profile_default_context *profile);
C_VOID vm_profile_default_qdx_refresh(vm_profile_default_context *profile);
C_VOID vm_profile_default_qdx_finalize(vm_profile_default_context *profile);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
