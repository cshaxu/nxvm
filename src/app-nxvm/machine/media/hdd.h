/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_HDD_H
#define VM_MACHINE_HDD_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "type.h"
#include "app-nxvm/devices/media_interface.h"
#include "lib/storage/medium_interface.h"

typedef struct t_hdd t_hdd;

#define VM_MACHINE_DEVICE_HDD "Unknown Hard Disk Drive"
#define VHDD_BYTE_PER_MB (1 << 20)

lib_size vm_machine_hdd_image_size(const t_hdd *hdd);
lib_size vm_machine_hdd_raw_byte_count(const t_hdd *hdd);
lib_u32 vm_machine_hdd_cylinders(const t_hdd *hdd);
C_INT vm_machine_hdd_has_media(const t_hdd *hdd);
C_VOID vm_machine_hdd_initialize(t_hdd *hdd);
C_VOID vm_machine_hdd_reset(t_hdd *hdd);
C_VOID vm_machine_hdd_finalize(t_hdd *hdd);
C_INT vm_machine_hdd_create(t_hdd *hdd, lib_u16 cylinders);
C_INT vm_machine_hdd_replace_bytes(t_hdd *hdd, const C_VOID *bytes,
    lib_size raw_byte_count);
C_INT vm_machine_hdd_insert(t_hdd *hdd, const C_CHAR *file_name,
    lib_storage_medium_mode mode);
C_INT vm_machine_hdd_set_geometry(t_hdd *hdd, lib_u32 cylinders,
    lib_u16 heads, lib_u16 sectors_per_track);
C_INT vm_machine_hdd_remove(t_hdd *hdd);
const core_machine_media_provider *vm_machine_hdd_media_provider(C_VOID);

#ifdef __cplusplus
}
#endif

#endif
