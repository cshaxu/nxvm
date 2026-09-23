/* Copyright 2012-2014 Neko. */

/* VFDD implements profile-configured floppy media. */
#include "lib/types/types_interface.h"

#include "type.h"

#include "app-nxvm/machine/media/fdd_private.h"

static core_machine_media_result vm_machine_fdd_media_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    const t_fdd *fdd = (const t_fdd *)context;

    if (fdd == LIB_NULL || out_info == LIB_NULL) return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    out_info->generation = fdd->connect.media_generation;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS;
    if (fdd->connect.flagReadOnly)
        out_info->capabilities |= CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY;
    out_info->present = fdd->connect.flagDiskExist;
    out_info->geometry.logical_sector_count = (lib_u64)fdd->data.ncyl *
        fdd->data.nhead * fdd->data.nsector;
    out_info->geometry.bytes_per_sector = fdd->data.nbyte;
    out_info->geometry.cylinders = fdd->data.ncyl;
    out_info->geometry.heads = fdd->data.nhead;
    out_info->geometry.sectors_per_track = fdd->data.nsector;
    return out_info->present ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result vm_machine_fdd_media_read(C_VOID *context,
    lib_u64 offset, C_VOID *buffer, lib_u32 byte_count)
{
    const t_fdd *fdd = (const t_fdd *)context;
    lib_size image_size;

    if (fdd == LIB_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (buffer == LIB_NULL || fdd->connect.medium == LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    image_size = vm_machine_fdd_image_size(fdd);
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    return lib_storage_medium_read_at(fdd->connect.medium, (lib_size)offset,
        buffer, byte_count) == LIB_STATUS_OK ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_PERMANENT;
}

static core_machine_media_result vm_machine_fdd_media_write(C_VOID *context,
    lib_u64 offset, const C_VOID *buffer, lib_u32 byte_count)
{
    t_fdd *fdd = (t_fdd *)context;
    lib_size image_size;

    if (fdd == LIB_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (buffer == LIB_NULL || fdd->connect.medium == LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    image_size = vm_machine_fdd_image_size(fdd);
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    return lib_storage_medium_write_at(fdd->connect.medium, (lib_size)offset,
        buffer, byte_count) == LIB_STATUS_OK ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_PERMANENT;
}

static core_machine_media_result vm_machine_fdd_media_format(C_VOID *context,
    lib_u64 logical_sector, lib_u32 sector_count, lib_u8 fill)
{
    t_fdd *fdd = (t_fdd *)context;
    lib_u64 sector_total;

    if (fdd == LIB_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (fdd->connect.medium == LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (lib_u64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total || sector_count > sector_total - logical_sector)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    if (lib_storage_medium_fill_at(fdd->connect.medium,
        (lib_size)(logical_sector * fdd->data.nbyte),
        (lib_size)sector_count * fdd->data.nbyte, fill) != LIB_STATUS_OK)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    ++fdd->connect.media_generation;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_fdd_media_get_address_mark(
    C_VOID *context, lib_u64 logical_sector,
    core_machine_media_address_mark *out_mark)
{
    const t_fdd *fdd = (const t_fdd *)context;
    lib_u64 sector_total;

    if (fdd == LIB_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (out_mark == LIB_NULL || fdd->connect.pAddressMarks == (type_virtual_address)LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (lib_u64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *out_mark = TYPE_DEREFERENCE_UNSIGNED_8(fdd->connect.pAddressMarks + (lib_size)logical_sector) ?
        CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA : CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_fdd_media_set_address_mark(
    C_VOID *context, lib_u64 logical_sector,
    core_machine_media_address_mark mark)
{
    t_fdd *fdd = (t_fdd *)context;
    lib_u64 sector_total;

    if (fdd == LIB_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (fdd->connect.pAddressMarks == (type_virtual_address)LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (lib_u64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total || (mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA))
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    TYPE_DEREFERENCE_UNSIGNED_8(fdd->connect.pAddressMarks + (lib_size)logical_sector) =
        mark == CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

const core_machine_media_provider *vm_machine_fdd_media_provider(C_VOID)
{
    static const core_machine_media_provider provider = {
        vm_machine_fdd_media_query,
        vm_machine_fdd_media_read,
        vm_machine_fdd_media_write,
        vm_machine_fdd_media_format,
        LIB_NULL,
        vm_machine_fdd_media_get_address_mark,
        vm_machine_fdd_media_set_address_mark
    };
    return &provider;
}

static C_INT vm_machine_fdd_geometry_is_valid(
    const core_machine_media_geometry *geometry)
{
    lib_size sector_count;

    if (geometry == LIB_NULL || geometry->cylinders == 0u ||
        geometry->heads == 0u || geometry->sectors_per_track == 0u ||
        geometry->bytes_per_sector == 0u) return LIB_FALSE;
    if (geometry->cylinders > (lib_size)-1 / geometry->heads) {
        return LIB_FALSE;
    }
    sector_count = (lib_size)geometry->cylinders * geometry->heads;
    if (geometry->sectors_per_track > (lib_size)-1 / sector_count) {
        return LIB_FALSE;
    }
    sector_count *= geometry->sectors_per_track;
    return geometry->logical_sector_count == sector_count &&
        geometry->bytes_per_sector <= (lib_size)-1 / sector_count;
}

static C_VOID vm_machine_fdd_apply_geometry(t_fdd *fdd)
{
    fdd->data.ncyl = fdd->geometry.cylinders;
    fdd->data.nhead = fdd->geometry.heads;
    fdd->data.nsector = fdd->geometry.sectors_per_track;
    fdd->data.nbyte = fdd->geometry.bytes_per_sector;
}
lib_size vm_machine_fdd_image_size(const t_fdd *fdd)
{
    return (lib_size)fdd->data.nbyte * fdd->data.nsector * fdd->data.nhead *
        fdd->data.ncyl;
}

C_INT vm_machine_fdd_has_media(const t_fdd *fdd)
{
    return fdd != LIB_NULL && fdd->connect.flagDiskExist;
}

static C_VOID vm_machine_fdd_install_medium(t_fdd *fdd,
    lib_storage_medium *candidate, type_virtual_address marks)
{
    lib_storage_medium *old_medium = LIB_NULL;
    type_virtual_address old_marks = fdd->connect.pAddressMarks;

    if (lib_storage_medium_replace(&fdd->connect.medium, candidate, &old_medium) !=
        LIB_STATUS_OK) return;
    fdd->connect.pAddressMarks = marks;
    fdd->connect.flagDiskExist = LIB_TRUE;
    ++fdd->connect.media_generation;
    lib_storage_medium_destroy(&old_medium);
    if (old_marks != (type_virtual_address)LIB_NULL) {
        lib_release((C_VOID *)old_marks);
    }
}

C_INT vm_machine_fdd_replace_bytes(t_fdd *fdd, const C_VOID *bytes,
    lib_size byte_count)
{
    lib_size image_size;
    lib_storage_medium *candidate = LIB_NULL;
    type_virtual_address marks = (type_virtual_address)LIB_NULL;

    if (fdd == LIB_NULL || bytes == LIB_NULL ||
        byte_count != (image_size = vm_machine_fdd_image_size(fdd)) ||
        lib_storage_medium_create_overlay(bytes, image_size, &candidate) !=
            TYPE_STATUS_OK ||
        (marks = (type_virtual_address)lib_allocate_zero((lib_size)fdd->data.ncyl *
            fdd->data.nhead * fdd->data.nsector, sizeof(lib_u8))) ==
            (type_virtual_address)LIB_NULL) {
        lib_storage_medium_destroy(&candidate);
        return LIB_TRUE;
    }
    vm_machine_fdd_install_medium(fdd, candidate, marks);
    return LIB_FALSE;
}

static lib_size vm_machine_fdd_byte_offset(const t_fdd *fdd,
    lib_u16 cylinder, lib_u16 head,
    lib_u16 sector, lib_u16 offset)
{
    return (((cylinder * fdd->data.nhead + head) *
        fdd->data.nsector + (sector - 1u)) * fdd->data.nbyte) + offset;
}

C_INT vm_machine_fdd_chs_valid(const t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 bytes)
{
    return fdd != LIB_NULL && fdd->connect.flagDiskExist &&
        fdd->connect.medium != LIB_NULL &&
        cylinder < fdd->data.ncyl && head < fdd->data.nhead && sector > 0u &&
        sector <= fdd->data.nsector && bytes == fdd->data.nbyte;
}

C_INT vm_machine_fdd_read_byte(const t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 offset,
    lib_u8 *out_byte)
{
    if (out_byte == LIB_NULL || !vm_machine_fdd_chs_valid(fdd, cylinder, head,
        sector, fdd != LIB_NULL ? fdd->data.nbyte : 0u) ||
        offset >= fdd->data.nbyte) return LIB_TRUE;
    return lib_storage_medium_read_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, offset), out_byte,
        1u) == LIB_STATUS_OK ? LIB_FALSE : LIB_TRUE;
}

C_INT vm_machine_fdd_write_byte(t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 offset,
    lib_u8 value)
{
    if (fdd == LIB_NULL || fdd->connect.flagReadOnly || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte) || offset >= fdd->data.nbyte) {
        return LIB_TRUE;
    }
    return lib_storage_medium_write_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, offset), &value,
        1u) == LIB_STATUS_OK ? LIB_FALSE : LIB_TRUE;
}

C_INT vm_machine_fdd_format_sector(t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u8 fill_byte)
{
    if (fdd == LIB_NULL || fdd->connect.flagReadOnly || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte)) return LIB_TRUE;
    if (lib_storage_medium_fill_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, 0u),
        fdd->data.nbyte, fill_byte) != LIB_STATUS_OK) return LIB_TRUE;
    ++fdd->connect.media_generation;
    return LIB_FALSE;
}

C_INT vm_machine_fdd_initialize_with_geometry(t_fdd *fdd,
    const core_machine_media_geometry *geometry)
{
    lib_size sector_count;

    if (fdd == LIB_NULL || !vm_machine_fdd_geometry_is_valid(geometry)) {
        return LIB_TRUE;
    }
    lib_memory_set((C_VOID *)fdd, TYPE_ZERO_8, sizeof(*fdd));
    fdd->geometry = *geometry;
    vm_machine_fdd_reset(fdd);
    sector_count = (lib_size)fdd->data.ncyl * fdd->data.nhead *
        fdd->data.nsector;
    if (lib_storage_medium_create_zero_overlay(vm_machine_fdd_image_size(fdd),
            &fdd->connect.medium) != TYPE_STATUS_OK) {
        fdd->connect.medium = LIB_NULL;
    }
    fdd->connect.pAddressMarks = (type_virtual_address)lib_allocate_zero(sector_count,
        sizeof(lib_u8));
    if (fdd->connect.medium == LIB_NULL || fdd->connect.pAddressMarks ==
        (type_virtual_address)LIB_NULL) {
        vm_machine_fdd_finalize(fdd);
        return LIB_TRUE;
    }
    return LIB_FALSE;
}

C_VOID vm_machine_fdd_reset(t_fdd *fdd)
{
    if (fdd == LIB_NULL) return;
    lib_memory_set((C_VOID *)&fdd->data, TYPE_ZERO_8, sizeof(fdd->data));
    vm_machine_fdd_apply_geometry(fdd);
}


C_VOID vm_machine_fdd_finalize(t_fdd *fdd)
{
    if (fdd != LIB_NULL) lib_storage_medium_destroy(&fdd->connect.medium);
    if (fdd != LIB_NULL && fdd->connect.pAddressMarks)
        lib_release((C_VOID *)fdd->connect.pAddressMarks);
    if (fdd != LIB_NULL) fdd->connect.pAddressMarks = (type_virtual_address)LIB_NULL;
}

C_VOID vm_machine_fdd_create_for(t_fdd *fdd)
{
    if (fdd != LIB_NULL && fdd->connect.medium != LIB_NULL &&
        fdd->connect.pAddressMarks != (type_virtual_address)LIB_NULL) {
        fdd->connect.flagDiskExist = LIB_TRUE;
        fdd->connect.media_generation++;
    }
}

static C_INT vm_machine_fdd_insert_medium_for(t_fdd *fdd, const C_CHAR *file_name,
    lib_storage_medium_mode mode)
{
    lib_size image_size;
    lib_storage_medium *candidate = LIB_NULL;
    type_virtual_address marks = (type_virtual_address)LIB_NULL;

    if (fdd == LIB_NULL || file_name == LIB_NULL ||
        lib_storage_medium_open(file_name, mode, &candidate) != LIB_STATUS_OK) return LIB_TRUE;
    image_size = vm_machine_fdd_image_size(fdd);
    if (lib_storage_medium_byte_count(candidate) != image_size ||
        (marks = (type_virtual_address)lib_allocate_zero((lib_size)fdd->data.ncyl *
            fdd->data.nhead * fdd->data.nsector, sizeof(lib_u8))) ==
            (type_virtual_address)LIB_NULL) {
        lib_storage_medium_destroy(&candidate);
        return LIB_TRUE;
    }
    vm_machine_fdd_install_medium(fdd, candidate, marks);
    fdd->connect.flagReadOnly = mode == LIB_STORAGE_MEDIUM_READONLY;
    return LIB_FALSE;
}

C_INT vm_machine_fdd_insert_for(t_fdd *fdd, const C_CHAR *file_name,
    lib_storage_medium_mode mode)
{
    return mode <= LIB_STORAGE_MEDIUM_OVERLAY ?
        vm_machine_fdd_insert_medium_for(fdd, file_name, mode) : LIB_TRUE;
}

C_INT vm_machine_fdd_remove_for(t_fdd *fdd)
{
    if (fdd == LIB_NULL) return LIB_TRUE;
    lib_storage_medium_destroy(&fdd->connect.medium);
    fdd->connect.flagDiskExist = LIB_FALSE;
    fdd->connect.flagReadOnly = LIB_FALSE;
    fdd->connect.media_generation++;
    if (fdd->connect.pAddressMarks != (type_virtual_address)LIB_NULL) {
        lib_memory_set((C_VOID *)fdd->connect.pAddressMarks, TYPE_ZERO_8,
            (lib_size)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector);
    }
    return LIB_FALSE;
}
