/* Copyright 2012-2014 Neko. */

/* VHDD implements Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63 */

#include "type.h"



#include "core/machine/dma.h"


#include "vm/machine/hdd.h"

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
    out_info->geometry.logical_sector_count = (uint64_t)hdd->data.ncyl *
        hdd->data.nhead * hdd->data.nsector;
    out_info->geometry.bytes_per_sector = hdd->data.nbyte;
    out_info->geometry.cylinders = hdd->data.ncyl;
    out_info->geometry.heads = hdd->data.nhead;
    out_info->geometry.sectors_per_track = hdd->data.nsector;
    return out_info->present ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result vm_machine_hdd_media_read(C_VOID *context,
    uint64_t offset, C_VOID *buffer, uint32_t byte_count)
{
    const t_hdd *hdd = (const t_hdd *)context;
    STD_SIZE_T image_size;

    if (hdd == STD_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    image_size = vm_machine_hdd_image_size(hdd);
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMCPY(buffer, (const C_VOID *)(hdd->connect.pImgBase + offset), byte_count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_hdd_media_write(C_VOID *context,
    uint64_t offset, const C_VOID *buffer, uint32_t byte_count)
{
    t_hdd *hdd = (t_hdd *)context;
    STD_SIZE_T image_size;

    if (hdd == STD_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (hdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    image_size = vm_machine_hdd_image_size(hdd);
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMCPY((C_VOID *)(hdd->connect.pImgBase + offset), buffer, byte_count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_hdd_media_format(C_VOID *context,
    uint64_t logical_sector, uint32_t sector_count, uint8_t fill)
{
    t_hdd *hdd = (t_hdd *)context;
    uint64_t sector_total;

    if (hdd == STD_NULL || !hdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (hdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    sector_total = (uint64_t)hdd->data.ncyl * hdd->data.nhead * hdd->data.nsector;
    if (logical_sector >= sector_total || sector_count > sector_total - logical_sector)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMSET((C_VOID *)(hdd->connect.pImgBase + logical_sector * hdd->data.nbyte),
        fill, sector_count * hdd->data.nbyte);
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
        STD_NULL
    };
    return &provider;
}

static C_INT vm_machine_hdd_track_end(const t_hdd *hdd) {
    return hdd->data.sector >= hdd->data.nsector + 1;
}

static C_INT vm_machine_hdd_cylinder_end(const t_hdd *hdd) {
    return hdd->data.head == hdd->data.nhead - 1 &&
        vm_machine_hdd_track_end(hdd);
}

STD_SIZE_T vm_machine_hdd_image_size(const t_hdd *hdd) {
    return (STD_SIZE_T)hdd->data.nbyte * hdd->data.nsector * hdd->data.nhead *
        hdd->data.ncyl;
}

C_VOID vm_machine_hdd_set_pointer(t_hdd *hdd) {
    hdd->connect.pCurrByte = hdd->connect.pImgBase +
        ((hdd->data.cyl * hdd->data.nhead + hdd->data.head) *
        hdd->data.nsector + (hdd->data.sector - 1)) * hdd->data.nbyte;
}

static C_VOID vm_machine_hdd_allocate(t_hdd *hdd) {
    STD_SIZE_T image_size = vm_machine_hdd_image_size(hdd);
    if (hdd->connect.pImgBase) {
        STD_FREE((C_VOID *)hdd->connect.pImgBase);
    }
    hdd->connect.pImgBase = (type_virtual_address)STD_MALLOC(image_size);
    STD_MEMSET((C_VOID *)hdd->connect.pImgBase, TYPE_ZERO_8, image_size);
}

C_VOID vm_machine_hdd_transfer_read(t_hdd *hdd, t_latch *latch) {
    if (hdd == STD_NULL || latch == STD_NULL || vm_machine_hdd_cylinder_end(hdd)) {
        return;
    }
    latch->data.byte = TYPE_DEREFERENCE_UNSIGNED_8(hdd->connect.pCurrByte);
    hdd->connect.pCurrByte++;
    hdd->connect.transCount++;
    if (!(hdd->connect.transCount % hdd->data.nbyte)) {
        hdd->data.sector++;
        if (vm_machine_hdd_track_end(hdd)) {
            hdd->data.sector = 1;
            hdd->data.head++;
        }
        vm_machine_hdd_set_pointer(hdd);
    }
}
C_VOID vm_machine_hdd_transfer_write(t_hdd *hdd, t_latch *latch) {
    if (hdd == STD_NULL || latch == STD_NULL || vm_machine_hdd_cylinder_end(hdd)) {
        return;
    }
    TYPE_DEREFERENCE_UNSIGNED_8(hdd->connect.pCurrByte) = latch->data.byte;
    hdd->connect.pCurrByte++;
    hdd->connect.transCount++;
    if (!(hdd->connect.transCount % hdd->data.nbyte)) {
        hdd->data.sector++;
        if (vm_machine_hdd_track_end(hdd)) {
            hdd->data.sector = 1;
            hdd->data.head++;
        }
        vm_machine_hdd_set_pointer(hdd);
    }
}
C_VOID vm_machine_hdd_format_track(t_hdd *hdd, type_unsigned_8 fill_byte) {
    type_native_unsigned i;
    if (hdd == STD_NULL || hdd->data.cyl >= hdd->data.ncyl) {
        return;
    }
    for (i = 0; i < hdd->data.nhead; ++i) {
        hdd->data.head = TYPE_MASK_UNSIGNED_16(i);
        hdd->data.sector = 1;
        vm_machine_hdd_set_pointer(hdd);
        STD_MEMSET((C_VOID *)hdd->connect.pCurrByte, fill_byte,
            hdd->data.nsector * hdd->data.nbyte);
        hdd->data.sector = hdd->data.nsector;
    }
    ++hdd->connect.media_generation;
}

C_VOID vm_machine_hdd_initialize(t_hdd *hdd) {
    if (hdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)hdd, TYPE_ZERO_8, sizeof(*hdd));
    hdd->data.ncyl = 0;
    hdd->data.nhead = 16;
    hdd->data.nsector = 63;
    hdd->data.nbyte = 512;
}
C_VOID vm_machine_hdd_reset(t_hdd *hdd) {
    type_unsigned_16 old_cylinders;
    if (hdd == STD_NULL) return;
    old_cylinders = hdd->data.ncyl;
    STD_MEMSET((C_VOID *)&hdd->data, TYPE_ZERO_8, sizeof(hdd->data));
    hdd->data.ncyl = old_cylinders;
    hdd->data.nhead = 16;
    hdd->data.nsector = 63;
    hdd->data.nbyte = 512;
}
C_VOID vm_machine_hdd_refresh(t_hdd *hdd) { (C_VOID)hdd; }
C_VOID vm_machine_hdd_finalize(t_hdd *hdd) {
    if (hdd != STD_NULL && hdd->connect.pImgBase) {
        STD_FREE((C_VOID *)hdd->connect.pImgBase);
    }
    if (hdd != STD_NULL) hdd->connect.pImgBase = (type_virtual_address)STD_NULL;
}

C_VOID vm_machine_hdd_create(t_hdd *hdd, uint16_t cylinders) {
    if (hdd == STD_NULL) return;
    hdd->data.ncyl = cylinders;
    vm_machine_hdd_allocate(hdd);
    hdd->connect.flagDiskExist = TYPE_TRUE;
    ++hdd->connect.media_generation;
}
C_INT vm_machine_hdd_insert(t_hdd *hdd, const C_CHAR *file_name) {
    type_native_unsigned count;
    STD_FILE *image = STD_FOPEN(file_name, "rb");
    if (hdd != STD_NULL && image) {
        STD_FSEEK(image, TYPE_ZERO_32, STD_SEEK_END);
        count = STD_FTELL(image);
        hdd->data.ncyl = (type_unsigned_16)(count / hdd->data.nhead /
            hdd->data.nsector / hdd->data.nbyte);
        STD_FSEEK(image, TYPE_ZERO_32, STD_SEEK_SET);
        vm_machine_hdd_allocate(hdd);
        count = STD_FREAD((C_VOID *)hdd->connect.pImgBase, sizeof(type_unsigned_8),
            vm_machine_hdd_image_size(hdd), image);
        hdd->connect.flagDiskExist = TYPE_TRUE;
        ++hdd->connect.media_generation;
        STD_FCLOSE(image);
        return TYPE_FALSE;
    } else {
        return TYPE_TRUE;
    }
}
C_INT vm_machine_hdd_remove(t_hdd *hdd, const C_CHAR *file_name) {
    type_native_unsigned count;
    STD_FILE *image;
    if (hdd == STD_NULL) return TYPE_TRUE;
    if (file_name) {
        image = STD_FOPEN(file_name, "wb");
        if (image) {
            if (!hdd->connect.flagReadOnly)
                count = STD_FWRITE((C_VOID *)hdd->connect.pImgBase, sizeof(type_unsigned_8),
                    vm_machine_hdd_image_size(hdd), image);
            hdd->connect.flagDiskExist = TYPE_FALSE;
            STD_FCLOSE(image);
        } else {
            return TYPE_TRUE;
        }
    }
    hdd->connect.flagDiskExist = TYPE_FALSE;
    ++hdd->connect.media_generation;
    STD_MEMSET((C_VOID *)hdd->connect.pImgBase, TYPE_ZERO_8, vm_machine_hdd_image_size(hdd));
    return TYPE_FALSE;
}
