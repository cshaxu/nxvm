#include "x86-nxvm/machine/preparation_interface.h"

lib_status x86_nxvm_machine_asset_copy(lib_u8 *destination,
    lib_size expected, x86_nxvm_machine_asset_bytes source)
{
    if (destination == LIB_NULL || source.data == LIB_NULL || source.bytes != expected) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_copy(destination, source.data, expected);
    return LIB_STATUS_OK;
}

static lib_status x86_nxvm_machine_prepare_text_glyphs(x86_nxvm_machine_construction *candidate,
    x86_nxvm_machine_asset_bytes source)
{
    lib_size character;

    if (source.data == LIB_NULL && source.bytes == 0u) return LIB_STATUS_OK;
    if (source.data == LIB_NULL || source.bytes != X86_NXVM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (character = 0u; character < X86_VIDEO_TEXT_GLYPH_COUNT; ++character) {
        lib_memory_copy(&candidate->text_glyphs.bytes[character * X86_VIDEO_TEXT_GLYPH_DEFAULT_ROWS],
            &source.data[character * 8u], 8u);
        lib_memory_copy(&candidate->text_glyphs.bytes[character * X86_VIDEO_TEXT_GLYPH_DEFAULT_ROWS + 8u],
            &source.data[X86_NXVM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES + character * 8u], 8u);
    }
    candidate->text_glyphs.present = LIB_TRUE;
    return LIB_STATUS_OK;
}

static lib_status x86_nxvm_machine_prepare_cmos(x86_nxvm_machine_construction *candidate,
    x86_nxvm_machine_asset_bytes source)
{
    if (source.data == LIB_NULL && source.bytes == 0u) return LIB_STATUS_OK;
    if (source.bytes != X86_NXVM_MACHINE_CMOS_SEED_BYTES &&
        source.bytes != X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (x86_nxvm_machine_asset_copy(candidate->cmos_seed, source.bytes,
            source) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    candidate->cmos_seed_bytes = source.bytes;
    candidate->cmos_seed_present = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_floppy_select(const x86_nxvm_machine_config *config,
    x86_nxvm_board_floppy_kind default_kind, lib_u32 allowed_kinds,
    x86_nxvm_board_floppy_kind *out_media)
{
    const x86_nxvm_machine_floppy_format format = config == LIB_NULL ?
        X86_NXVM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT : config->floppy_format;
    x86_nxvm_board_floppy_kind kind;

    if (out_media == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (format) {
    case X86_NXVM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT: kind = default_kind; break;
    case X86_NXVM_MACHINE_FLOPPY_FORMAT_360K: kind = X86_NXVM_BOARD_FLOPPY_525_360K; break;
    case X86_NXVM_MACHINE_FLOPPY_FORMAT_720K: kind = X86_NXVM_BOARD_FLOPPY_35_720K; break;
    case X86_NXVM_MACHINE_FLOPPY_FORMAT_1200K: kind = X86_NXVM_BOARD_FLOPPY_525_1200K; break;
    case X86_NXVM_MACHINE_FLOPPY_FORMAT_1440K: kind = X86_NXVM_BOARD_FLOPPY_35_1440K; break;
    default: return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (kind < X86_NXVM_BOARD_FLOPPY_35_1440K || kind > X86_NXVM_BOARD_FLOPPY_35_720K ||
        (allowed_kinds & (1u << kind)) == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    *out_media = kind;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_construction_begin(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *out_construction)
{
    if (out_construction == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_construction = (x86_nxvm_machine_construction){0};
    if (config == LIB_NULL || assets == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_construction_finish(x86_nxvm_machine_construction *candidate,
    const x86_nxvm_machine_config *config, const x86_nxvm_machine_assets *assets,
    lib_status status, x86_nxvm_machine_construction *out_construction)
{
    if (out_construction != LIB_NULL) *out_construction = (x86_nxvm_machine_construction){0};
    if (candidate == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (config == LIB_NULL || assets == LIB_NULL || out_construction == LIB_NULL) {
        status = LIB_STATUS_INVALID_ARGUMENT;
    }
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_prepare_cmos(candidate, assets->cmos_seed);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_prepare_text_glyphs(candidate, assets->font);
    if (status == LIB_STATUS_OK && config->floppy_image[1u] != LIB_NULL &&
        candidate->floppy_slot_count < 2u) status = LIB_STATUS_INVALID_ARGUMENT;
    if (status != LIB_STATUS_OK) {
        if (candidate->profile.release != LIB_NULL)
            candidate->profile.release(candidate->profile.context);
        return status;
    }
    *out_construction = *candidate;
    return LIB_STATUS_OK;
}
