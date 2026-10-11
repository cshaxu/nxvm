/* Copyright 2012-2014 Neko. */

/* VFDD implements profile-configured floppy media. */
#include "lib/types/types_interface.h"
#include "core/machine/media/fdd.h"

static lib_bool vm_machine_fdd_geometry_is_valid(
    const core_machine_media_geometry *geometry);

lib_status vm_machine_fdd_allocate(const core_machine_media_geometry *geometry,
    t_fdd **out_fdd)
{
    t_fdd *candidate;

    if (out_fdd == LIB_NULL || *out_fdd != LIB_NULL ||
        !vm_machine_fdd_geometry_is_valid(geometry))
        return LIB_STATUS_INVALID_ARGUMENT;
    candidate = lib_allocate_zero(1u, sizeof(*candidate));
    if (candidate == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_status status = vm_machine_fdd_initialize_with_geometry(candidate, geometry);
    if (status != LIB_STATUS_OK) {
        lib_release(candidate);
        return status;
    }
    *out_fdd = candidate;
    return LIB_STATUS_OK;
}

void vm_machine_fdd_destroy(t_fdd **fdd)
{
    if (fdd == LIB_NULL || *fdd == LIB_NULL) return;
    vm_machine_fdd_finalize(*fdd);
    lib_release(*fdd);
    *fdd = LIB_NULL;
}

static core_machine_media_result vm_machine_fdd_media_query(void *context,
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

static core_machine_media_result vm_machine_fdd_media_read(void *context,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
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

static core_machine_media_result vm_machine_fdd_media_write(void *context,
    lib_u64 offset, const void *buffer, lib_u32 byte_count)
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

static core_machine_media_result vm_machine_fdd_media_format(void *context,
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
    void *context, lib_u64 logical_sector,
    core_machine_media_address_mark *out_mark)
{
    const t_fdd *fdd = (const t_fdd *)context;
    lib_u64 sector_total;

    if (fdd == LIB_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (out_mark == LIB_NULL || fdd->connect.address_marks == LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (lib_u64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *out_mark = fdd->connect.address_marks[logical_sector] ?
        CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA : CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_fdd_media_set_address_mark(
    void *context, lib_u64 logical_sector,
    core_machine_media_address_mark mark)
{
    t_fdd *fdd = (t_fdd *)context;
    lib_u64 sector_total;

    if (fdd == LIB_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (fdd->connect.address_marks == LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (lib_u64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total || (mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA))
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    fdd->connect.address_marks[logical_sector] =
        mark == CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

const core_machine_media_provider *vm_machine_fdd_media_provider(void)
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

static lib_bool vm_machine_fdd_geometry_is_valid(
    const core_machine_media_geometry *geometry)
{
    lib_size sector_count;

    if (geometry == LIB_NULL || geometry->cylinders == 0u ||
        geometry->heads == 0u || geometry->sectors_per_track == 0u ||
        geometry->bytes_per_sector == 0u) return LIB_FALSE;
    if (geometry->cylinders > LIB_SIZE_MAX / geometry->heads) {
        return LIB_FALSE;
    }
    sector_count = (lib_size)geometry->cylinders * geometry->heads;
    if (geometry->sectors_per_track > LIB_SIZE_MAX / sector_count) {
        return LIB_FALSE;
    }
    sector_count *= geometry->sectors_per_track;
    return geometry->logical_sector_count == sector_count &&
        geometry->bytes_per_sector <= LIB_SIZE_MAX / sector_count;
}

static void vm_machine_fdd_apply_geometry(t_fdd *fdd)
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

lib_bool vm_machine_fdd_has_media(const t_fdd *fdd)
{
    return fdd != LIB_NULL && fdd->connect.flagDiskExist;
}

static lib_status vm_machine_fdd_install_medium(t_fdd *fdd,
    lib_storage_medium *candidate, lib_u8 *marks, lib_bool readonly)
{
    lib_storage_medium *old_medium = LIB_NULL;
    lib_u8 *old_marks = fdd->connect.address_marks;
    lib_status status = lib_storage_medium_replace(&fdd->connect.medium,
        candidate, &old_medium);

    if (status != LIB_STATUS_OK) {
        lib_storage_medium_destroy(&candidate);
        lib_release(marks);
        return status;
    }
    fdd->connect.address_marks = marks;
    fdd->connect.flagReadOnly = readonly;
    fdd->connect.flagDiskExist = LIB_TRUE;
    ++fdd->connect.media_generation;
    status = lib_storage_medium_destroy(&old_medium);
    lib_release(old_marks);
    return status;
}

lib_status vm_machine_fdd_replace_bytes(t_fdd *fdd, const void *bytes,
    lib_size byte_count)
{
    lib_storage_medium *candidate = LIB_NULL;
    lib_u8 *marks;
    lib_status status;

    if (fdd == LIB_NULL || bytes == LIB_NULL ||
        byte_count != vm_machine_fdd_image_size(fdd))
        return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_medium_create_overlay(bytes, byte_count, &candidate);
    if (status != LIB_STATUS_OK) return status;
    marks = lib_allocate_zero((lib_size)fdd->data.ncyl * fdd->data.nhead *
        fdd->data.nsector, sizeof(*marks));
    if (marks == LIB_NULL) {
        lib_storage_medium_destroy(&candidate);
        return LIB_STATUS_NO_MEMORY;
    }
    return vm_machine_fdd_install_medium(fdd, candidate, marks, LIB_FALSE);
}

static lib_size vm_machine_fdd_byte_offset(const t_fdd *fdd,
    lib_u16 cylinder, lib_u16 head,
    lib_u16 sector, lib_u16 offset)
{
    return (((cylinder * fdd->data.nhead + head) *
        fdd->data.nsector + (sector - 1u)) * fdd->data.nbyte) + offset;
}

lib_bool vm_machine_fdd_chs_valid(const t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 bytes)
{
    return fdd != LIB_NULL && fdd->connect.flagDiskExist &&
        fdd->connect.medium != LIB_NULL &&
        cylinder < fdd->data.ncyl && head < fdd->data.nhead && sector > 0u &&
        sector <= fdd->data.nsector && bytes == fdd->data.nbyte;
}

lib_status vm_machine_fdd_read_byte(const t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 offset,
    lib_u8 *out_byte)
{
    if (out_byte == LIB_NULL || !vm_machine_fdd_chs_valid(fdd, cylinder, head,
        sector, fdd != LIB_NULL ? fdd->data.nbyte : 0u) ||
        offset >= fdd->data.nbyte) return LIB_STATUS_INVALID_ARGUMENT;
    return lib_storage_medium_read_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, offset), out_byte,
        1u);
}

lib_status vm_machine_fdd_write_byte(t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 offset,
    lib_u8 value)
{
    if (fdd == LIB_NULL || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte) || offset >= fdd->data.nbyte) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (fdd->connect.flagReadOnly) return LIB_STATUS_INVALID_STATE;
    return lib_storage_medium_write_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, offset), &value,
        1u);
}

lib_status vm_machine_fdd_format_sector(t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u8 fill_byte)
{
    if (fdd == LIB_NULL || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte)) return LIB_STATUS_INVALID_ARGUMENT;
    if (fdd->connect.flagReadOnly) return LIB_STATUS_INVALID_STATE;
    lib_status status = lib_storage_medium_fill_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, 0u),
        fdd->data.nbyte, fill_byte);
    if (status != LIB_STATUS_OK) return status;
    ++fdd->connect.media_generation;
    return LIB_STATUS_OK;
}

lib_status vm_machine_fdd_initialize_with_geometry(t_fdd *fdd,
    const core_machine_media_geometry *geometry)
{
    lib_size sector_count;
    lib_status status;

    if (fdd == LIB_NULL || !vm_machine_fdd_geometry_is_valid(geometry)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_set((void *)fdd, 0u, sizeof(*fdd));
    fdd->geometry = *geometry;
    vm_machine_fdd_reset(fdd);
    sector_count = (lib_size)fdd->data.ncyl * fdd->data.nhead *
        fdd->data.nsector;
    status = lib_storage_medium_create_zero_overlay(vm_machine_fdd_image_size(fdd),
        &fdd->connect.medium);
    if (status != LIB_STATUS_OK) return status;
    fdd->connect.address_marks = lib_allocate_zero(sector_count,
        sizeof(lib_u8));
    if (fdd->connect.medium == LIB_NULL || fdd->connect.address_marks ==
        LIB_NULL) {
        vm_machine_fdd_finalize(fdd);
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

void vm_machine_fdd_reset(t_fdd *fdd)
{
    if (fdd == LIB_NULL) return;
    lib_memory_set((void *)&fdd->data, 0u, sizeof(fdd->data));
    vm_machine_fdd_apply_geometry(fdd);
}


void vm_machine_fdd_finalize(t_fdd *fdd)
{
    if (fdd != LIB_NULL) lib_storage_medium_destroy(&fdd->connect.medium);
    if (fdd != LIB_NULL && fdd->connect.address_marks)
        lib_release(fdd->connect.address_marks);
    if (fdd != LIB_NULL) fdd->connect.address_marks = LIB_NULL;
}

void vm_machine_fdd_create_for(t_fdd *fdd)
{
    if (fdd != LIB_NULL && fdd->connect.medium != LIB_NULL &&
        fdd->connect.address_marks != LIB_NULL) {
        fdd->connect.flagDiskExist = LIB_TRUE;
        fdd->connect.media_generation++;
    }
}

lib_status vm_machine_fdd_insert_for(t_fdd *fdd, const char *file_name,
    lib_storage_medium_mode mode)
{
    lib_storage_medium *candidate = LIB_NULL;
    lib_u8 *marks;
    lib_status status;

    if (fdd == LIB_NULL || file_name == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_medium_open(file_name, mode, &candidate);
    if (status != LIB_STATUS_OK) return status;
    if (lib_storage_medium_byte_count(candidate) != vm_machine_fdd_image_size(fdd)) {
        lib_storage_medium_destroy(&candidate);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    marks = lib_allocate_zero((lib_size)fdd->data.ncyl * fdd->data.nhead *
        fdd->data.nsector, sizeof(*marks));
    if (marks == LIB_NULL) {
        lib_storage_medium_destroy(&candidate);
        return LIB_STATUS_NO_MEMORY;
    }
    return vm_machine_fdd_install_medium(fdd, candidate, marks,
        mode == LIB_STORAGE_MEDIUM_READONLY);
}

lib_status vm_machine_fdd_remove_for(t_fdd *fdd)
{
    lib_status status;

    if (fdd == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_medium_destroy(&fdd->connect.medium);
    fdd->connect.flagDiskExist = LIB_FALSE;
    fdd->connect.flagReadOnly = LIB_FALSE;
    fdd->connect.media_generation++;
    if (fdd->connect.address_marks != LIB_NULL) {
        lib_memory_set(fdd->connect.address_marks, 0u,
            (lib_size)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector);
    }
    return status;
}
