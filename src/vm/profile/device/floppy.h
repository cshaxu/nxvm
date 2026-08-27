#ifndef VM_PROFILE_DEVICE_FLOPPY_H
#define VM_PROFILE_DEVICE_FLOPPY_H

#include "type.h"
#include "core/machine/media_interface.h"

typedef enum vm_profile_floppy_kind {
    VM_PROFILE_FLOPPY_35_1440K = 0,
    VM_PROFILE_FLOPPY_525_1200K,
    VM_PROFILE_FLOPPY_525_360K,
    VM_PROFILE_FLOPPY_35_720K
} vm_profile_floppy_kind;

const core_machine_media_geometry *vm_profile_floppy_geometry_get(
    vm_profile_floppy_kind kind);
type_unsigned_8 vm_profile_floppy_cmos_type_get(vm_profile_floppy_kind kind);

#endif
