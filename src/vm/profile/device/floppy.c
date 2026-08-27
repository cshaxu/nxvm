#include "vm/profile/device/floppy.h"

static const core_machine_media_geometry vm_profile_floppy_geometries[] = {
    {2880u, 512u, 80u, 2u, 18u},
    {2400u, 512u, 80u, 2u, 15u},
    {720u, 512u, 40u, 2u, 9u},
    {1440u, 512u, 80u, 2u, 9u}
};

const core_machine_media_geometry *vm_profile_floppy_geometry_get(
    vm_profile_floppy_kind kind)
{
    return kind < sizeof(vm_profile_floppy_geometries) /
        sizeof(vm_profile_floppy_geometries[0]) ?
        &vm_profile_floppy_geometries[kind] : STD_NULL;
}

type_unsigned_8 vm_profile_floppy_cmos_type_get(vm_profile_floppy_kind kind)
{
    static const type_unsigned_8 cmos_types[] = {0x40u, 0x20u, 0x10u, 0x30u};

    return kind < sizeof(cmos_types) / sizeof(cmos_types[0]) ?
        cmos_types[kind] : 0u;
}
