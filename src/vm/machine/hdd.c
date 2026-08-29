/* Copyright 2012-2014 Neko. */

/* VHDD implements Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63 */

#include "type.h"



#include "vm/machine/hdd_private.h"
#include "vm/machine/media_save.h"

#include "core/platform/file.h"

static core_machine_media_result vm_machine_hdd_media_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    const t_hdd *hdd = (const t_hdd *)context;

    if (hdd == STD_NULL || out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    out_info->generation = hdd->connect.media_generation;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE |
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

static core_machine_media_result vm_machine_hdd_media_read(C_VOID *context,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    const t_hdd *hdd = (const t_hdd *)context;
    STD_SIZE_T image_size;

    if (hdd == STD_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    image_size = hdd->connect.virtual_byte_count;
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    if (byte_count == 0u) return CORE_MACHINE_MEDIA_RESULT_OK;
    if (buffer == STD_NULL || hdd->connect.pImgBase == (type_virtual_address)STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    STD_MEMCPY(buffer, (const C_VOID *)(hdd->connect.pImgBase + (STD_SIZE_T)offset), byte_count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_hdd_media_write(C_VOID *context,
    type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 byte_count)
{
    t_hdd *hdd = (t_hdd *)context;
    STD_SIZE_T image_size;

    if (hdd == STD_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (hdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    image_size = hdd->connect.virtual_byte_count;
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    if (byte_count == 0u) return CORE_MACHINE_MEDIA_RESULT_OK;
    if (buffer == STD_NULL || hdd->connect.pImgBase == (type_virtual_address)STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    STD_MEMCPY((C_VOID *)(hdd->connect.pImgBase + (STD_SIZE_T)offset), buffer, byte_count);
    if (offset + byte_count > hdd->connect.raw_byte_count) {
        hdd->connect.flagPaddingWritten = TYPE_TRUE;
    }
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_hdd_media_format(C_VOID *context,
    type_unsigned_64 logical_sector, type_unsigned_32 sector_count, type_unsigned_8 fill)
{
    t_hdd *hdd = (t_hdd *)context;
    type_unsigned_64 sector_total;

    if (hdd == STD_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (hdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (hdd->connect.pImgBase == (type_virtual_address)STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = hdd->connect.virtual_byte_count / hdd->data.nbyte;
    if (logical_sector >= sector_total || sector_count > sector_total - logical_sector)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMSET((C_VOID *)(hdd->connect.pImgBase + (STD_SIZE_T)(logical_sector * hdd->data.nbyte)),
        fill, sector_count * hdd->data.nbyte);
    if ((logical_sector + sector_count) * hdd->data.nbyte >
        hdd->connect.raw_byte_count) {
        hdd->connect.flagPaddingWritten = TYPE_TRUE;
    }
    ++hdd->connect.media_generation;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

const core_machine_media_provider *vm_machine_hdd_media_provider(C_VOID)
{
    static const core_machine_media_provider provider = {
        vm_machine_hdd_media_query,
        vm_machine_hdd_media_read,
        vm_machine_hdd_media_write,
        vm_machine_hdd_media_format,
        STD_NULL,
        STD_NULL,
        STD_NULL
    };
    return &provider;
}

STD_SIZE_T vm_machine_hdd_image_size(const t_hdd *hdd) {
    return hdd == STD_NULL ? 0u : hdd->connect.virtual_byte_count;
}

STD_SIZE_T vm_machine_hdd_raw_byte_count(const t_hdd *hdd) {
    return hdd == STD_NULL ? 0u : hdd->connect.raw_byte_count;
}

type_unsigned_32 vm_machine_hdd_cylinders(const t_hdd *hdd) {
    return hdd == STD_NULL ? 0u : hdd->data.ncyl;
}

C_INT vm_machine_hdd_has_media(const t_hdd *hdd) {
    return hdd != STD_NULL && hdd->connect.flagDiskExist;
}

static C_INT vm_machine_hdd_capacity_from_raw(STD_SIZE_T raw_byte_count,
    STD_SIZE_T *out_virtual_byte_count, type_unsigned_32 *out_cylinders)
{
    const STD_SIZE_T sector_size = 512u;
    const STD_SIZE_T sectors_per_cylinder = 16u * 63u;
    const STD_SIZE_T lba28_sector_limit = 0x10000000u;
    STD_SIZE_T virtual_byte_count;
    STD_SIZE_T sectors;
    STD_SIZE_T cylinders;

    if (out_virtual_byte_count == STD_NULL || out_cylinders == STD_NULL ||
        raw_byte_count > ((STD_SIZE_T)-1) - (sector_size - 1u)) {
        return TYPE_TRUE;
    }
    virtual_byte_count = raw_byte_count == 0u ? 0u :
        ((raw_byte_count + sector_size - 1u) / sector_size) * sector_size;
    sectors = virtual_byte_count / sector_size;
    cylinders = sectors == 0u ? 0u :
        (sectors + sectors_per_cylinder - 1u) / sectors_per_cylinder;
    if (sectors > lba28_sector_limit || cylinders > UINT32_MAX) {
        return TYPE_TRUE;
    }
    *out_virtual_byte_count = virtual_byte_count;
    *out_cylinders = (type_unsigned_32)cylinders;
    return TYPE_FALSE;
}

static type_virtual_address vm_machine_hdd_allocate_candidate(STD_SIZE_T byte_count)
{
    type_virtual_address image;

    if (byte_count == 0u) {
        return (type_virtual_address)STD_NULL;
    }
    image = (type_virtual_address)STD_MALLOC(byte_count);
    if (image != (type_virtual_address)STD_NULL) {
        STD_MEMSET((C_VOID *)image, TYPE_ZERO_8, byte_count);
    }
    return image;
}

static C_VOID vm_machine_hdd_commit_candidate(t_hdd *hdd,
    type_virtual_address candidate, STD_SIZE_T raw_byte_count,
    STD_SIZE_T virtual_byte_count, type_unsigned_32 cylinders)
{
    type_virtual_address old_image = hdd->connect.pImgBase;

    hdd->connect.pImgBase = candidate;
    hdd->connect.raw_byte_count = raw_byte_count;
    hdd->connect.virtual_byte_count = virtual_byte_count;
    hdd->connect.flagPaddingWritten = TYPE_FALSE;
    hdd->connect.geometry_cylinders = 0u;
    hdd->connect.geometry_heads = 16u;
    hdd->connect.geometry_sectors_per_track = 63u;
    hdd->data.ncyl = cylinders;
    hdd->data.cyl = 0u;
    hdd->data.head = 0u;
    hdd->data.sector = 1u;
    hdd->connect.flagDiskExist = TYPE_TRUE;
    ++hdd->connect.media_generation;
    if (old_image != (type_virtual_address)STD_NULL) {
        STD_FREE((C_VOID *)old_image);
    }
}

C_VOID vm_machine_hdd_initialize(t_hdd *hdd) {
    if (hdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)hdd, TYPE_ZERO_8, sizeof(*hdd));
    hdd->connect.geometry_heads = 16u;
    hdd->connect.geometry_sectors_per_track = 63u;
    hdd->data.nhead = hdd->connect.geometry_heads;
    hdd->data.nsector = hdd->connect.geometry_sectors_per_track;
    hdd->data.nbyte = 512u;
}
C_VOID vm_machine_hdd_reset(t_hdd *hdd) {
    if (hdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)&hdd->data, TYPE_ZERO_8, sizeof(hdd->data));
    hdd->data.nhead = hdd->connect.geometry_heads;
    hdd->data.nsector = hdd->connect.geometry_sectors_per_track;
    hdd->data.nbyte = 512u;
    hdd->data.ncyl = hdd->connect.geometry_cylinders;
    if (hdd->data.ncyl == 0u && hdd->connect.virtual_byte_count != 0u) {
        hdd->data.ncyl = (type_unsigned_32)((hdd->connect.virtual_byte_count / 512u +
            (16u * 63u) - 1u) / (16u * 63u));
    }
}
C_VOID vm_machine_hdd_finalize(t_hdd *hdd) {
    if (hdd != STD_NULL && hdd->connect.pImgBase) {
        STD_FREE((C_VOID *)hdd->connect.pImgBase);
    }
    if (hdd != STD_NULL) {
        hdd->connect.pImgBase = (type_virtual_address)STD_NULL;
        hdd->connect.raw_byte_count = 0u;
        hdd->connect.virtual_byte_count = 0u;
        hdd->connect.flagPaddingWritten = TYPE_FALSE;
    }
}

C_VOID vm_machine_hdd_create(t_hdd *hdd, type_unsigned_16 cylinders) {
    STD_SIZE_T virtual_byte_count;
    type_virtual_address candidate;

    if (hdd == STD_NULL) return;
    virtual_byte_count = (STD_SIZE_T)cylinders * 16u * 63u * 512u;
    candidate = vm_machine_hdd_allocate_candidate(virtual_byte_count);
    if (virtual_byte_count != 0u && candidate == (type_virtual_address)STD_NULL) {
        return;
    }
    vm_machine_hdd_commit_candidate(hdd, candidate, virtual_byte_count,
        virtual_byte_count, cylinders);
}
C_INT vm_machine_hdd_replace_bytes(t_hdd *hdd, const C_VOID *bytes,
    STD_SIZE_T raw_byte_count)
{
    STD_SIZE_T virtual_byte_count;
    type_unsigned_32 cylinders;
    type_virtual_address candidate;

    if (hdd == STD_NULL || (raw_byte_count != 0u && bytes == STD_NULL) ||
        vm_machine_hdd_capacity_from_raw(raw_byte_count, &virtual_byte_count,
            &cylinders)) {
        return TYPE_TRUE;
    }
    candidate = vm_machine_hdd_allocate_candidate(virtual_byte_count);
    if (virtual_byte_count != 0u && candidate == (type_virtual_address)STD_NULL) {
        return TYPE_TRUE;
    }
    if (raw_byte_count != 0u) {
        STD_MEMCPY((C_VOID *)candidate, bytes, raw_byte_count);
    }
    vm_machine_hdd_commit_candidate(hdd, candidate, raw_byte_count,
        virtual_byte_count, cylinders);
    return TYPE_FALSE;
}
C_INT vm_machine_hdd_insert(t_hdd *hdd, const C_CHAR *file_name) {
    STD_SIZE_T raw_byte_count;
    STD_SIZE_T virtual_byte_count;
    type_unsigned_32 cylinders;
    type_virtual_address candidate;
    C_VOID *loaded = STD_NULL;

    if (hdd == STD_NULL || file_name == STD_NULL ||
        core_platform_file_read_all(file_name, (STD_SIZE_T)-1, &loaded,
            &raw_byte_count) != TYPE_FALSE) {
        return TYPE_TRUE;
    }
    if (vm_machine_hdd_capacity_from_raw(raw_byte_count, &virtual_byte_count,
            &cylinders) ||
        (virtual_byte_count != 0u &&
            (candidate = vm_machine_hdd_allocate_candidate(virtual_byte_count)) ==
                (type_virtual_address)STD_NULL)) {
        STD_FREE(loaded);
        return TYPE_TRUE;
    }
    if (virtual_byte_count == 0u) {
        candidate = (type_virtual_address)STD_NULL;
    }
    if (raw_byte_count != 0u) {
        STD_MEMCPY((C_VOID *)candidate, loaded, raw_byte_count);
    }
    STD_FREE(loaded);
    vm_machine_hdd_commit_candidate(hdd, candidate, raw_byte_count,
        virtual_byte_count, cylinders);
    return TYPE_FALSE;
}
C_INT vm_machine_hdd_remove(t_hdd *hdd, const C_CHAR *file_name) {
    STD_SIZE_T persistence_byte_count;
    if (hdd == STD_NULL) return TYPE_TRUE;
    if (file_name) {
        persistence_byte_count = hdd->connect.flagPaddingWritten ?
            hdd->connect.virtual_byte_count : hdd->connect.raw_byte_count;
        if (!hdd->connect.flagReadOnly && vm_machine_media_save_atomically(file_name,
                (const C_VOID *)hdd->connect.pImgBase,
                persistence_byte_count) != TYPE_FALSE) {
            return TYPE_TRUE;
        }
        if (hdd->connect.flagPaddingWritten) {
            hdd->connect.raw_byte_count = hdd->connect.virtual_byte_count;
            hdd->connect.flagPaddingWritten = TYPE_FALSE;
        }
    }
    hdd->connect.flagDiskExist = TYPE_FALSE;
    ++hdd->connect.media_generation;
    if (hdd->connect.pImgBase != (type_virtual_address)STD_NULL) {
        STD_MEMSET((C_VOID *)hdd->connect.pImgBase, TYPE_ZERO_8,
            hdd->connect.virtual_byte_count);
    }
    return TYPE_FALSE;
}
C_INT vm_machine_hdd_set_geometry(t_hdd *hdd, type_unsigned_32 cylinders,
    type_unsigned_16 heads, type_unsigned_16 sectors_per_track)
{
    type_unsigned_64 expected_bytes;

    if (hdd == STD_NULL || !hdd->connect.flagDiskExist || cylinders == 0u || heads == 0u ||
        sectors_per_track == 0u) return TYPE_TRUE;
    expected_bytes = (type_unsigned_64)cylinders * heads * sectors_per_track * 512u;
    if (expected_bytes != hdd->connect.virtual_byte_count) return TYPE_TRUE;
    hdd->connect.geometry_cylinders = cylinders;
    hdd->connect.geometry_heads = heads;
    hdd->connect.geometry_sectors_per_track = sectors_per_track;
    hdd->data.ncyl = cylinders;
    hdd->data.nhead = heads;
    hdd->data.nsector = sectors_per_track;
    hdd->data.nbyte = 512u;
    return TYPE_FALSE;
}
