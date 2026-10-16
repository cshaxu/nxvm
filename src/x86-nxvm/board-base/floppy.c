#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/floppy_interface.h"

static const x86_nxvm_board_media_geometry x86_nxvm_board_floppy_geometries[] = {
    {2880u, 512u, 80u, 2u, 18u},
    {2400u, 512u, 80u, 2u, 15u},
    {720u, 512u, 40u, 2u, 9u},
    {1440u, 512u, 80u, 2u, 9u}
};

const x86_nxvm_board_media_geometry *x86_nxvm_board_floppy_geometry_get(
    x86_nxvm_board_floppy_kind kind)
{
    return kind < sizeof(x86_nxvm_board_floppy_geometries) /
        sizeof(x86_nxvm_board_floppy_geometries[0]) ?
        &x86_nxvm_board_floppy_geometries[kind] : LIB_NULL;
}

lib_u8 x86_nxvm_board_floppy_cmos_type_get(x86_nxvm_board_floppy_kind kind)
{
    static const lib_u8 cmos_types[] = {0x40u, 0x20u, 0x10u, 0x30u};

    return kind < sizeof(cmos_types) / sizeof(cmos_types[0]) ?
        cmos_types[kind] : 0u;
}

static lib_bool x86_nxvm_board_floppy_channel_sample(const void *context,
    lib_u8 drive, const x86_nxvm_board_media_geometry *geometry,
    lib_u16 physical_cylinder, lib_u32 rate_bps, lib_bool mfm,
    lib_u16 *out_cylinder)
{
    const x86_nxvm_board_floppy_kind kind = *(const x86_nxvm_board_floppy_kind *)context;
    lib_u32 expected_rate;
    lib_u16 step = 1u;
    lib_size media;
    (void)drive;

    if (!mfm || geometry == LIB_NULL || out_cylinder == LIB_NULL) return LIB_FALSE;
    for (media = 0u; media < sizeof(x86_nxvm_board_floppy_geometries) /
            sizeof(x86_nxvm_board_floppy_geometries[0]); ++media) {
        const x86_nxvm_board_media_geometry *candidate = &x86_nxvm_board_floppy_geometries[media];
        if (geometry->cylinders == candidate->cylinders &&
            geometry->heads == candidate->heads &&
            geometry->sectors_per_track == candidate->sectors_per_track &&
            geometry->bytes_per_sector == candidate->bytes_per_sector &&
            geometry->logical_sector_count == candidate->logical_sector_count) break;
    }
    if (media == sizeof(x86_nxvm_board_floppy_geometries) /
            sizeof(x86_nxvm_board_floppy_geometries[0])) return LIB_FALSE;
    expected_rate = media == X86_NXVM_BOARD_FLOPPY_35_1440K ||
        media == X86_NXVM_BOARD_FLOPPY_525_1200K ? 500000u : 250000u;
    if (kind == X86_NXVM_BOARD_FLOPPY_525_1200K) {
        if (media != X86_NXVM_BOARD_FLOPPY_525_1200K &&
            media != X86_NXVM_BOARD_FLOPPY_525_360K) return LIB_FALSE;
        if (media == X86_NXVM_BOARD_FLOPPY_525_360K) {
            expected_rate = 300000u;
            step = 2u;
        }
    } else if (kind == X86_NXVM_BOARD_FLOPPY_525_360K) {
        if (media != X86_NXVM_BOARD_FLOPPY_525_360K) return LIB_FALSE;
    } else if (kind == X86_NXVM_BOARD_FLOPPY_35_720K) {
        if (media != X86_NXVM_BOARD_FLOPPY_35_720K) return LIB_FALSE;
    }
    /* The default logical mechanism admits the existing four image formats.
     * It is not a claim that a physical 3.5-inch drive accepts 5.25-inch media. */
    if (rate_bps != expected_rate || physical_cylinder % step != 0u ||
        physical_cylinder / step >= geometry->cylinders) return LIB_FALSE;
    *out_cylinder = physical_cylinder / step;
    return LIB_TRUE;
}

x86_nxvm_board_fdc_channel_provider x86_nxvm_board_floppy_channel_get(
    x86_nxvm_board_floppy_kind kind)
{
    static const x86_nxvm_board_floppy_kind kinds[] = {
        X86_NXVM_BOARD_FLOPPY_35_1440K, X86_NXVM_BOARD_FLOPPY_525_1200K,
        X86_NXVM_BOARD_FLOPPY_525_360K, X86_NXVM_BOARD_FLOPPY_35_720K
    };
    return kind < sizeof(kinds) / sizeof(kinds[0]) ?
        (x86_nxvm_board_fdc_channel_provider) {x86_nxvm_board_floppy_channel_sample, &kinds[kind]} :
        (x86_nxvm_board_fdc_channel_provider) {0};
}
