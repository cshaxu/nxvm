/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_QDDISK_H
#define VM_PROFILE_DEFAULT_QDDISK_H



#include "type.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct vm_profile_default_context vm_profile_default_context;
C_VOID vm_profile_default_disk_handle_hdd_read(vm_profile_default_context *profile);
C_VOID vm_profile_default_disk_handle_hdd_write(vm_profile_default_context *profile);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
