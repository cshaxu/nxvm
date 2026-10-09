#ifndef VM_PROFILE_DEVICE_FLOPPY_H
#define VM_PROFILE_DEVICE_FLOPPY_H
#include "lib/types/types_interface.h"

#include "core/board-base/media_interface.h"
#include "core/board-base/controller_interface.h"

typedef enum vm_profile_floppy_kind {
    VM_PROFILE_FLOPPY_35_1440K = 0,
    VM_PROFILE_FLOPPY_525_1200K,
    VM_PROFILE_FLOPPY_525_360K,
    VM_PROFILE_FLOPPY_35_720K
} vm_profile_floppy_kind;

const core_machine_media_geometry *vm_profile_floppy_geometry_get(
    vm_profile_floppy_kind kind);
lib_u8 vm_profile_floppy_cmos_type_get(vm_profile_floppy_kind kind);
core_machine_fdc_channel_provider vm_profile_floppy_channel_get(
    vm_profile_floppy_kind kind);

#endif
