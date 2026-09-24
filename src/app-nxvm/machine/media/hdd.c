/* Copyright 2012-2014 Neko. */

/* VHDD implements Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63 */
#include "lib/types/types_interface.h"




#include "app-nxvm/machine/media/hdd_private.h"

static core_machine_media_result vm_machine_hdd_media_query(void *context,
    core_machine_media_info *out_info)
{
    const t_hdd *hdd = (const t_hdd *)context;

    if (hdd == LIB_NULL || out_info == LIB_NULL) return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    out_info->generation = hdd->connect.media_generation;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE;
    if (hdd->connect.flagReadOnly)
        out_info->capabilities |= CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY;
    out_info->present = hdd->connect.flagDiskExist;
    out_info->geometry.logical_sector_count = hdd->connect.virtual_byte_count /
        hdd->data.nbyte;
    out_info->geometry.bytes_per_sector = hdd->data.nbyte;
    out_info->geometry.cylinders = hdd->data.ncyl;
    out_info->geometry.heads = hdd->data.nhead;
    out_info->geometry.sectors_per_track = hdd->data.nsector;
    return out_info->present ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result vm_machine_hdd_media_read(void *context,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
{
    const t_hdd *hdd = (const t_hdd *)context;
    lib_size image_size;

    if (hdd == LIB_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    image_size = hdd->connect.virtual_byte_count;
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    if (byte_count == 0u) return CORE_MACHINE_MEDIA_RESULT_OK;
    if (buffer == LIB_NULL || hdd->connect.medium == LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    return lib_storage_medium_read_at(hdd->connect.medium, (lib_size)offset,
        buffer, byte_count) == LIB_STATUS_OK ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_PERMANENT;
}

static core_machine_media_result vm_machine_hdd_media_write(void *context,
    lib_u64 offset, const void *buffer, lib_u32 byte_count)
{
    t_hdd *hdd = (t_hdd *)context;
    lib_size image_size;

    if (hdd == LIB_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (hdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    image_size = hdd->connect.virtual_byte_count;
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    if (byte_count == 0u) return CORE_MACHINE_MEDIA_RESULT_OK;
    if (buffer == LIB_NULL || hdd->connect.medium == LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    return lib_storage_medium_write_at(hdd->connect.medium, (lib_size)offset,
        buffer, byte_count) == LIB_STATUS_OK ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_PERMANENT;
}

static core_machine_media_result vm_machine_hdd_media_format(void *context,
    lib_u64 logical_sector, lib_u32 sector_count, lib_u8 fill)
{
    t_hdd *hdd = (t_hdd *)context;
    lib_u64 sector_total;

    if (hdd == LIB_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (hdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (hdd->connect.medium == LIB_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = hdd->connect.virtual_byte_count / hdd->data.nbyte;
    if (logical_sector >= sector_total || sector_count > sector_total - logical_sector)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    if (lib_storage_medium_fill_at(hdd->connect.medium,
        (lib_size)(logical_sector * hdd->data.nbyte),
        (lib_size)sector_count * hdd->data.nbyte, fill) != LIB_STATUS_OK)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    ++hdd->connect.media_generation;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

const core_machine_media_provider *vm_machine_hdd_media_provider(void)
{
    static const core_machine_media_provider provider = {
        vm_machine_hdd_media_query,
        vm_machine_hdd_media_read,
        vm_machine_hdd_media_write,
        vm_machine_hdd_media_format,
        LIB_NULL,
        LIB_NULL,
        LIB_NULL
    };
    return &provider;
}

lib_size vm_machine_hdd_image_size(const t_hdd *hdd) {
    return hdd == LIB_NULL ? 0u : hdd->connect.virtual_byte_count;
}

lib_size vm_machine_hdd_raw_byte_count(const t_hdd *hdd) {
    return hdd == LIB_NULL ? 0u : hdd->connect.raw_byte_count;
}

lib_u32 vm_machine_hdd_cylinders(const t_hdd *hdd) {
    return hdd == LIB_NULL ? 0u : hdd->data.ncyl;
}

lib_i32 vm_machine_hdd_has_media(const t_hdd *hdd) {
    return hdd != LIB_NULL && hdd->connect.flagDiskExist;
}

static lib_i32 vm_machine_hdd_capacity_from_raw(lib_size raw_byte_count,
    lib_size *out_virtual_byte_count, lib_u32 *out_cylinders)
{
    const lib_size sector_size = 512u;
    const lib_size sectors_per_cylinder = 16u * 63u;
    const lib_size lba28_sector_limit = 0x10000000u;
    lib_size virtual_byte_count;
    lib_size sectors;
    lib_size cylinders;

    if (out_virtual_byte_count == LIB_NULL || out_cylinders == LIB_NULL ||
        raw_byte_count > ((lib_size)-1) - (sector_size - 1u)) {
        return LIB_TRUE;
    }
    virtual_byte_count = raw_byte_count == 0u ? 0u :
        ((raw_byte_count + sector_size - 1u) / sector_size) * sector_size;
    sectors = virtual_byte_count / sector_size;
    cylinders = sectors == 0u ? 0u :
        (sectors + sectors_per_cylinder - 1u) / sectors_per_cylinder;
    if (sectors > lba28_sector_limit || cylinders > UINT32_MAX) {
        return LIB_TRUE;
    }
    *out_virtual_byte_count = virtual_byte_count;
    *out_cylinders = (lib_u32)cylinders;
    return LIB_FALSE;
}

static lib_storage_medium *vm_machine_hdd_allocate_candidate(
    lib_size byte_count)
{
    lib_storage_medium *medium = LIB_NULL;

    if (byte_count == 0u) return LIB_NULL;
    return lib_storage_medium_create_zero_overlay(byte_count, &medium) == LIB_STATUS_OK ?
        medium : LIB_NULL;
}

static void vm_machine_hdd_install_medium(t_hdd *hdd,
    lib_storage_medium *candidate, lib_size raw_byte_count,
    lib_size virtual_byte_count, lib_u32 cylinders)
{
    lib_storage_medium *old_medium = LIB_NULL;

    if (lib_storage_medium_replace(&hdd->connect.medium, candidate, &old_medium) !=
        LIB_STATUS_OK) return;
    hdd->connect.raw_byte_count = raw_byte_count;
    hdd->connect.virtual_byte_count = virtual_byte_count;
    hdd->connect.geometry_cylinders = 0u;
    hdd->connect.geometry_heads = 16u;
    hdd->connect.geometry_sectors_per_track = 63u;
    hdd->data.ncyl = cylinders;
    hdd->data.cyl = 0u;
    hdd->data.head = 0u;
    hdd->data.sector = 1u;
    hdd->connect.flagDiskExist = LIB_TRUE;
    ++hdd->connect.media_generation;
    lib_storage_medium_destroy(&old_medium);
}

void vm_machine_hdd_initialize(t_hdd *hdd) {
    if (hdd == LIB_NULL) return;
    lib_memory_set((void *)hdd, 0u, sizeof(*hdd));
    hdd->connect.geometry_heads = 16u;
    hdd->connect.geometry_sectors_per_track = 63u;
    hdd->data.nhead = hdd->connect.geometry_heads;
    hdd->data.nsector = hdd->connect.geometry_sectors_per_track;
    hdd->data.nbyte = 512u;
}
void vm_machine_hdd_reset(t_hdd *hdd) {
    if (hdd == LIB_NULL) return;
    lib_memory_set((void *)&hdd->data, 0u, sizeof(hdd->data));
    hdd->data.nhead = hdd->connect.geometry_heads;
    hdd->data.nsector = hdd->connect.geometry_sectors_per_track;
    hdd->data.nbyte = 512u;
    hdd->data.ncyl = hdd->connect.geometry_cylinders;
    if (hdd->data.ncyl == 0u && hdd->connect.virtual_byte_count != 0u) {
        hdd->data.ncyl = (lib_u32)((hdd->connect.virtual_byte_count / 512u +
            (16u * 63u) - 1u) / (16u * 63u));
    }
}
void vm_machine_hdd_finalize(t_hdd *hdd) {
    if (hdd != LIB_NULL) lib_storage_medium_destroy(&hdd->connect.medium);
    if (hdd != LIB_NULL) {
        hdd->connect.raw_byte_count = 0u;
        hdd->connect.virtual_byte_count = 0u;
    }
}

lib_i32 vm_machine_hdd_create(t_hdd *hdd, lib_u16 cylinders) {
    lib_size virtual_byte_count;
    lib_storage_medium *candidate;

    if (hdd == LIB_NULL || cylinders == 0u) return LIB_TRUE;
    virtual_byte_count = (lib_size)cylinders * 16u * 63u * 512u;
    candidate = vm_machine_hdd_allocate_candidate(virtual_byte_count);
    if (virtual_byte_count != 0u && candidate == LIB_NULL) {
        return LIB_TRUE;
    }
    vm_machine_hdd_install_medium(hdd, candidate, virtual_byte_count,
        virtual_byte_count, cylinders);
    return LIB_FALSE;
}
lib_i32 vm_machine_hdd_replace_bytes(t_hdd *hdd, const void *bytes,
    lib_size raw_byte_count)
{
    lib_size virtual_byte_count;
    lib_u32 cylinders;
    lib_storage_medium *candidate;

    if (hdd == LIB_NULL || (raw_byte_count != 0u && bytes == LIB_NULL) ||
        vm_machine_hdd_capacity_from_raw(raw_byte_count, &virtual_byte_count,
            &cylinders)) {
        return LIB_TRUE;
    }
    candidate = vm_machine_hdd_allocate_candidate(virtual_byte_count);
    if (virtual_byte_count != 0u && candidate == LIB_NULL) {
        return LIB_TRUE;
    }
    if (raw_byte_count != 0u) {
        if (lib_storage_medium_write_at(candidate, 0u, bytes, raw_byte_count) !=
            LIB_STATUS_OK) {
            lib_storage_medium_destroy(&candidate);
            return LIB_TRUE;
        }
    }
    vm_machine_hdd_install_medium(hdd, candidate, raw_byte_count,
        virtual_byte_count, cylinders);
    return LIB_FALSE;
}
static lib_i32 vm_machine_hdd_insert_medium(t_hdd *hdd, const char *file_name,
    lib_storage_medium_mode mode) {
    lib_size raw_byte_count;
    lib_size virtual_byte_count;
    lib_u32 cylinders;
    lib_storage_medium *candidate = LIB_NULL;

    if (hdd == LIB_NULL || file_name == LIB_NULL ||
        lib_storage_medium_open(file_name, mode, &candidate) != LIB_STATUS_OK) {
        return LIB_TRUE;
    }
    raw_byte_count = lib_storage_medium_byte_count(candidate);
    if (vm_machine_hdd_capacity_from_raw(raw_byte_count, &virtual_byte_count,
            &cylinders) ||
        raw_byte_count == 0u || raw_byte_count != virtual_byte_count) {
        lib_storage_medium_destroy(&candidate);
        return LIB_TRUE;
    }
    vm_machine_hdd_install_medium(hdd, candidate, raw_byte_count,
        virtual_byte_count, cylinders);
    hdd->connect.flagReadOnly = mode == LIB_STORAGE_MEDIUM_READONLY;
    return LIB_FALSE;
}
lib_i32 vm_machine_hdd_insert(t_hdd *hdd, const char *file_name,
    lib_storage_medium_mode mode)
{
    return mode <= LIB_STORAGE_MEDIUM_OVERLAY ?
        vm_machine_hdd_insert_medium(hdd, file_name, mode) : LIB_TRUE;
}
lib_i32 vm_machine_hdd_remove(t_hdd *hdd) {
    if (hdd == LIB_NULL) return LIB_TRUE;
    lib_storage_medium_destroy(&hdd->connect.medium);
    hdd->connect.flagDiskExist = LIB_FALSE;
    hdd->connect.flagReadOnly = LIB_FALSE;
    hdd->connect.raw_byte_count = 0u;
    hdd->connect.virtual_byte_count = 0u;
    hdd->connect.geometry_cylinders = 0u;
    ++hdd->connect.media_generation;
    return LIB_FALSE;
}

lib_i32 vm_machine_hdd_set_geometry(t_hdd *hdd, lib_u32 cylinders,
    lib_u16 heads, lib_u16 sectors_per_track)
{
    lib_u64 expected_bytes;

    if (hdd == LIB_NULL || !hdd->connect.flagDiskExist || cylinders == 0u || heads == 0u ||
        sectors_per_track == 0u) return LIB_TRUE;
    expected_bytes = (lib_u64)cylinders * heads * sectors_per_track * 512u;
    if (expected_bytes != hdd->connect.virtual_byte_count) return LIB_TRUE;
    hdd->connect.geometry_cylinders = cylinders;
    hdd->connect.geometry_heads = heads;
    hdd->connect.geometry_sectors_per_track = sectors_per_track;
    hdd->data.ncyl = cylinders;
    hdd->data.nhead = heads;
    hdd->data.nsector = sectors_per_track;
    hdd->data.nbyte = 512u;
    return LIB_FALSE;
}
