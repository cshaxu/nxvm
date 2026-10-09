#include "lib/types/types_interface.h"
#include "core/board-base/floppy_interface.h"

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
        &vm_profile_floppy_geometries[kind] : LIB_NULL;
}

lib_u8 vm_profile_floppy_cmos_type_get(vm_profile_floppy_kind kind)
{
    static const lib_u8 cmos_types[] = {0x40u, 0x20u, 0x10u, 0x30u};

    return kind < sizeof(cmos_types) / sizeof(cmos_types[0]) ?
        cmos_types[kind] : 0u;
}

static lib_bool vm_profile_floppy_channel_sample(const void *context,
    lib_u8 drive, const core_machine_media_geometry *geometry,
    lib_u16 physical_cylinder, lib_u32 rate_bps, lib_bool mfm,
    lib_u16 *out_cylinder)
{
    const vm_profile_floppy_kind kind = *(const vm_profile_floppy_kind *)context;
    lib_u32 expected_rate;
    lib_u16 step = 1u;
    lib_size media;
    (void)drive;

    if (!mfm || geometry == LIB_NULL || out_cylinder == LIB_NULL) return LIB_FALSE;
    for (media = 0u; media < sizeof(vm_profile_floppy_geometries) /
            sizeof(vm_profile_floppy_geometries[0]); ++media) {
        const core_machine_media_geometry *candidate = &vm_profile_floppy_geometries[media];
        if (geometry->cylinders == candidate->cylinders &&
            geometry->heads == candidate->heads &&
            geometry->sectors_per_track == candidate->sectors_per_track &&
            geometry->bytes_per_sector == candidate->bytes_per_sector &&
            geometry->logical_sector_count == candidate->logical_sector_count) break;
    }
    if (media == sizeof(vm_profile_floppy_geometries) /
            sizeof(vm_profile_floppy_geometries[0])) return LIB_FALSE;
    expected_rate = media == VM_PROFILE_FLOPPY_35_1440K ||
        media == VM_PROFILE_FLOPPY_525_1200K ? 500000u : 250000u;
    if (kind == VM_PROFILE_FLOPPY_525_1200K) {
        if (media != VM_PROFILE_FLOPPY_525_1200K &&
            media != VM_PROFILE_FLOPPY_525_360K) return LIB_FALSE;
        if (media == VM_PROFILE_FLOPPY_525_360K) {
            expected_rate = 300000u;
            step = 2u;
        }
    } else if (kind == VM_PROFILE_FLOPPY_525_360K) {
        if (media != VM_PROFILE_FLOPPY_525_360K) return LIB_FALSE;
    } else if (kind == VM_PROFILE_FLOPPY_35_720K) {
        if (media != VM_PROFILE_FLOPPY_35_720K) return LIB_FALSE;
    }
    /* The default logical mechanism admits the existing four image formats.
     * It is not a claim that a physical 3.5-inch drive accepts 5.25-inch media. */
    if (rate_bps != expected_rate || physical_cylinder % step != 0u ||
        physical_cylinder / step >= geometry->cylinders) return LIB_FALSE;
    *out_cylinder = physical_cylinder / step;
    return LIB_TRUE;
}

core_machine_fdc_channel_provider vm_profile_floppy_channel_get(
    vm_profile_floppy_kind kind)
{
    static const vm_profile_floppy_kind kinds[] = {
        VM_PROFILE_FLOPPY_35_1440K, VM_PROFILE_FLOPPY_525_1200K,
        VM_PROFILE_FLOPPY_525_360K, VM_PROFILE_FLOPPY_35_720K
    };
    return kind < sizeof(kinds) / sizeof(kinds[0]) ?
        (core_machine_fdc_channel_provider) {vm_profile_floppy_channel_sample, &kinds[kind]} :
        (core_machine_fdc_channel_provider) {0};
}
