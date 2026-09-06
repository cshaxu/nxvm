/* Copyright 2012-2014 Neko. */

/* VFDD implements profile-configured floppy media. */

#include "type.h"

#include "vm/machine/fdd_private.h"

static core_machine_media_result vm_machine_fdd_media_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    const t_fdd *fdd = (const t_fdd *)context;

    if (fdd == STD_NULL || out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    out_info->generation = fdd->connect.media_generation;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS;
    if (fdd->connect.flagReadOnly)
        out_info->capabilities |= CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY;
    out_info->present = fdd->connect.flagDiskExist;
    out_info->geometry.logical_sector_count = (type_unsigned_64)fdd->data.ncyl *
        fdd->data.nhead * fdd->data.nsector;
    out_info->geometry.bytes_per_sector = fdd->data.nbyte;
    out_info->geometry.cylinders = fdd->data.ncyl;
    out_info->geometry.heads = fdd->data.nhead;
    out_info->geometry.sectors_per_track = fdd->data.nsector;
    return out_info->present ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result vm_machine_fdd_media_read(C_VOID *context,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    const t_fdd *fdd = (const t_fdd *)context;
    STD_SIZE_T image_size;

    if (fdd == STD_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (buffer == STD_NULL || fdd->connect.medium == STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    image_size = vm_machine_fdd_image_size(fdd);
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    return lib_storage_medium_read_at(fdd->connect.medium, (STD_SIZE_T)offset,
        buffer, byte_count) == LIB_STATUS_OK ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_PERMANENT;
}

static core_machine_media_result vm_machine_fdd_media_write(C_VOID *context,
    type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 byte_count)
{
    t_fdd *fdd = (t_fdd *)context;
    STD_SIZE_T image_size;

    if (fdd == STD_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (buffer == STD_NULL || fdd->connect.medium == STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    image_size = vm_machine_fdd_image_size(fdd);
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    return lib_storage_medium_write_at(fdd->connect.medium, (STD_SIZE_T)offset,
        buffer, byte_count) == LIB_STATUS_OK ? CORE_MACHINE_MEDIA_RESULT_OK :
        CORE_MACHINE_MEDIA_RESULT_PERMANENT;
}

static core_machine_media_result vm_machine_fdd_media_format(C_VOID *context,
    type_unsigned_64 logical_sector, type_unsigned_32 sector_count, type_unsigned_8 fill)
{
    t_fdd *fdd = (t_fdd *)context;
    type_unsigned_64 sector_total;

    if (fdd == STD_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (fdd->connect.medium == STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (type_unsigned_64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total || sector_count > sector_total - logical_sector)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    if (lib_storage_medium_fill_at(fdd->connect.medium,
        (STD_SIZE_T)(logical_sector * fdd->data.nbyte),
        (STD_SIZE_T)sector_count * fdd->data.nbyte, fill) != LIB_STATUS_OK)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    ++fdd->connect.media_generation;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_fdd_media_get_address_mark(
    C_VOID *context, type_unsigned_64 logical_sector,
    core_machine_media_address_mark *out_mark)
{
    const t_fdd *fdd = (const t_fdd *)context;
    type_unsigned_64 sector_total;

    if (fdd == STD_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (out_mark == STD_NULL || fdd->connect.pAddressMarks == (type_virtual_address)STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (type_unsigned_64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *out_mark = TYPE_DEREFERENCE_UNSIGNED_8(fdd->connect.pAddressMarks + (STD_SIZE_T)logical_sector) ?
        CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA : CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_fdd_media_set_address_mark(
    C_VOID *context, type_unsigned_64 logical_sector,
    core_machine_media_address_mark mark)
{
    t_fdd *fdd = (t_fdd *)context;
    type_unsigned_64 sector_total;

    if (fdd == STD_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (fdd->connect.pAddressMarks == (type_virtual_address)STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (type_unsigned_64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total || (mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA))
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    TYPE_DEREFERENCE_UNSIGNED_8(fdd->connect.pAddressMarks + (STD_SIZE_T)logical_sector) =
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
        STD_NULL,
        vm_machine_fdd_media_get_address_mark,
        vm_machine_fdd_media_set_address_mark
    };
    return &provider;
}

static const core_machine_media_geometry vm_machine_fdd_default_geometry = {
    2880u, 512u, 80u, 2u, 18u
};

static C_INT vm_machine_fdd_geometry_is_valid(
    const core_machine_media_geometry *geometry)
{
    STD_SIZE_T sector_count;

    if (geometry == STD_NULL || geometry->cylinders == 0u ||
        geometry->heads == 0u || geometry->sectors_per_track == 0u ||
        geometry->bytes_per_sector == 0u) return TYPE_FALSE;
    if (geometry->cylinders > (STD_SIZE_T)-1 / geometry->heads) {
        return TYPE_FALSE;
    }
    sector_count = (STD_SIZE_T)geometry->cylinders * geometry->heads;
    if (geometry->sectors_per_track > (STD_SIZE_T)-1 / sector_count) {
        return TYPE_FALSE;
    }
    sector_count *= geometry->sectors_per_track;
    return geometry->logical_sector_count == sector_count &&
        geometry->bytes_per_sector <= (STD_SIZE_T)-1 / sector_count;
}

static C_VOID vm_machine_fdd_apply_geometry(t_fdd *fdd)
{
    fdd->data.ncyl = fdd->geometry.cylinders;
    fdd->data.nhead = fdd->geometry.heads;
    fdd->data.nsector = fdd->geometry.sectors_per_track;
    fdd->data.nbyte = fdd->geometry.bytes_per_sector;
}
STD_SIZE_T vm_machine_fdd_image_size(const t_fdd *fdd)
{
    return (STD_SIZE_T)fdd->data.nbyte * fdd->data.nsector * fdd->data.nhead *
        fdd->data.ncyl;
}

C_INT vm_machine_fdd_has_media(const t_fdd *fdd)
{
    return fdd != STD_NULL && fdd->connect.flagDiskExist;
}

static C_VOID vm_machine_fdd_install_medium(t_fdd *fdd,
    lib_storage_medium *candidate, type_virtual_address marks)
{
    lib_storage_medium *old_medium = STD_NULL;
    type_virtual_address old_marks = fdd->connect.pAddressMarks;

    if (lib_storage_medium_replace(&fdd->connect.medium, candidate, &old_medium) !=
        LIB_STATUS_OK) return;
    fdd->connect.pAddressMarks = marks;
    fdd->connect.flagDiskExist = TYPE_TRUE;
    ++fdd->connect.media_generation;
    lib_storage_medium_destroy(old_medium);
    if (old_marks != (type_virtual_address)STD_NULL) {
        STD_FREE((C_VOID *)old_marks);
    }
}

C_INT vm_machine_fdd_replace_bytes(t_fdd *fdd, const C_VOID *bytes,
    STD_SIZE_T byte_count)
{
    STD_SIZE_T image_size;
    lib_storage_medium *candidate = STD_NULL;
    type_virtual_address marks = (type_virtual_address)STD_NULL;

    if (fdd == STD_NULL || bytes == STD_NULL ||
        byte_count != (image_size = vm_machine_fdd_image_size(fdd)) ||
        lib_storage_medium_create_overlay(bytes, image_size, &candidate) !=
            TYPE_STATUS_OK ||
        (marks = (type_virtual_address)STD_CALLOC((STD_SIZE_T)fdd->data.ncyl *
            fdd->data.nhead * fdd->data.nsector, sizeof(type_unsigned_8))) ==
            (type_virtual_address)STD_NULL) {
        lib_storage_medium_destroy(candidate);
        return TYPE_TRUE;
    }
    vm_machine_fdd_install_medium(fdd, candidate, marks);
    return TYPE_FALSE;
}

static STD_SIZE_T vm_machine_fdd_byte_offset(const t_fdd *fdd,
    type_unsigned_16 cylinder, type_unsigned_16 head,
    type_unsigned_16 sector, type_unsigned_16 offset)
{
    return (((cylinder * fdd->data.nhead + head) *
        fdd->data.nsector + (sector - 1u)) * fdd->data.nbyte) + offset;
}

C_INT vm_machine_fdd_chs_valid(const t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 bytes)
{
    return fdd != STD_NULL && fdd->connect.flagDiskExist &&
        fdd->connect.medium != STD_NULL &&
        cylinder < fdd->data.ncyl && head < fdd->data.nhead && sector > 0u &&
        sector <= fdd->data.nsector && bytes == fdd->data.nbyte;
}

C_INT vm_machine_fdd_read_byte(const t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 offset,
    type_unsigned_8 *out_byte)
{
    if (out_byte == STD_NULL || !vm_machine_fdd_chs_valid(fdd, cylinder, head,
        sector, fdd != STD_NULL ? fdd->data.nbyte : 0u) ||
        offset >= fdd->data.nbyte) return TYPE_TRUE;
    return lib_storage_medium_read_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, offset), out_byte,
        1u) == LIB_STATUS_OK ? TYPE_FALSE : TYPE_TRUE;
}

C_INT vm_machine_fdd_write_byte(t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 offset,
    type_unsigned_8 value)
{
    if (fdd == STD_NULL || fdd->connect.flagReadOnly || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte) || offset >= fdd->data.nbyte) {
        return TYPE_TRUE;
    }
    return lib_storage_medium_write_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, offset), &value,
        1u) == LIB_STATUS_OK ? TYPE_FALSE : TYPE_TRUE;
}

C_INT vm_machine_fdd_format_sector(t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_8 fill_byte)
{
    if (fdd == STD_NULL || fdd->connect.flagReadOnly || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte)) return TYPE_TRUE;
    if (lib_storage_medium_fill_at(fdd->connect.medium,
        vm_machine_fdd_byte_offset(fdd, cylinder, head, sector, 0u),
        fdd->data.nbyte, fill_byte) != LIB_STATUS_OK) return TYPE_TRUE;
    ++fdd->connect.media_generation;
    return TYPE_FALSE;
}

C_VOID vm_machine_fdd_initialize(t_fdd *fdd)
{
    (C_VOID)vm_machine_fdd_initialize_with_geometry(fdd,
        &vm_machine_fdd_default_geometry);
}

C_INT vm_machine_fdd_initialize_with_geometry(t_fdd *fdd,
    const core_machine_media_geometry *geometry)
{
    STD_SIZE_T sector_count;

    if (fdd == STD_NULL || !vm_machine_fdd_geometry_is_valid(geometry)) {
        return TYPE_TRUE;
    }
    STD_MEMSET((C_VOID *)fdd, TYPE_ZERO_8, sizeof(*fdd));
    fdd->geometry = *geometry;
    vm_machine_fdd_reset(fdd);
    sector_count = (STD_SIZE_T)fdd->data.ncyl * fdd->data.nhead *
        fdd->data.nsector;
    if (lib_storage_medium_create_zero_overlay(vm_machine_fdd_image_size(fdd),
            &fdd->connect.medium) != TYPE_STATUS_OK) {
        fdd->connect.medium = STD_NULL;
    }
    fdd->connect.pAddressMarks = (type_virtual_address)STD_CALLOC(sector_count,
        sizeof(type_unsigned_8));
    if (fdd->connect.medium == STD_NULL || fdd->connect.pAddressMarks ==
        (type_virtual_address)STD_NULL) {
        vm_machine_fdd_finalize(fdd);
        return TYPE_TRUE;
    }
    return TYPE_FALSE;
}

C_VOID vm_machine_fdd_reset(t_fdd *fdd)
{
    if (fdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)&fdd->data, TYPE_ZERO_8, sizeof(fdd->data));
    vm_machine_fdd_apply_geometry(fdd);
}


C_VOID vm_machine_fdd_finalize(t_fdd *fdd)
{
    if (fdd != STD_NULL) lib_storage_medium_destroy(fdd->connect.medium);
    if (fdd != STD_NULL && fdd->connect.pAddressMarks)
        STD_FREE((C_VOID *)fdd->connect.pAddressMarks);
    if (fdd != STD_NULL) fdd->connect.medium = STD_NULL;
    if (fdd != STD_NULL) fdd->connect.pAddressMarks = (type_virtual_address)STD_NULL;
}

C_VOID vm_machine_fdd_create_for(t_fdd *fdd)
{
    if (fdd != STD_NULL && fdd->connect.medium != STD_NULL &&
        fdd->connect.pAddressMarks != (type_virtual_address)STD_NULL) {
        fdd->connect.flagDiskExist = TYPE_TRUE;
        fdd->connect.media_generation++;
    }
}

static C_INT vm_machine_fdd_insert_medium_for(t_fdd *fdd, const C_CHAR *file_name,
    lib_storage_medium_mode mode)
{
    STD_SIZE_T image_size;
    lib_storage_medium *candidate = STD_NULL;
    type_virtual_address marks = (type_virtual_address)STD_NULL;

    if (fdd == STD_NULL || file_name == STD_NULL ||
        lib_storage_medium_open(file_name, mode, &candidate) != LIB_STATUS_OK) return TYPE_TRUE;
    image_size = vm_machine_fdd_image_size(fdd);
    if (lib_storage_medium_byte_count(candidate) != image_size ||
        (marks = (type_virtual_address)STD_CALLOC((STD_SIZE_T)fdd->data.ncyl *
            fdd->data.nhead * fdd->data.nsector, sizeof(type_unsigned_8))) ==
            (type_virtual_address)STD_NULL) {
        lib_storage_medium_destroy(candidate);
        return TYPE_TRUE;
    }
    vm_machine_fdd_install_medium(fdd, candidate, marks);
    fdd->connect.flagReadOnly = mode == LIB_STORAGE_MEDIUM_READONLY;
    return TYPE_FALSE;
}

C_INT vm_machine_fdd_insert_for(t_fdd *fdd, const C_CHAR *file_name)
{ return vm_machine_fdd_insert_medium_for(fdd, file_name, LIB_STORAGE_MEDIUM_OVERLAY); }

C_INT vm_machine_fdd_insert_readonly_for(t_fdd *fdd, const C_CHAR *file_name)
{ return vm_machine_fdd_insert_medium_for(fdd, file_name, LIB_STORAGE_MEDIUM_READONLY); }

C_INT vm_machine_fdd_insert_direct_for(t_fdd *fdd, const C_CHAR *file_name)
{ return vm_machine_fdd_insert_medium_for(fdd, file_name, LIB_STORAGE_MEDIUM_DIRECT); }

C_INT vm_machine_fdd_remove_for(t_fdd *fdd, const C_CHAR *file_name)
{
    (C_VOID)file_name;
    if (fdd == STD_NULL) return TYPE_TRUE;
    lib_storage_medium_discard(&fdd->connect.medium);
    fdd->connect.flagDiskExist = TYPE_FALSE;
    fdd->connect.flagReadOnly = TYPE_FALSE;
    fdd->connect.media_generation++;
    if (fdd->connect.pAddressMarks != (type_virtual_address)STD_NULL) {
        STD_MEMSET((C_VOID *)fdd->connect.pAddressMarks, TYPE_ZERO_8,
            (STD_SIZE_T)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector);
    }
    return TYPE_FALSE;
}

C_VOID vm_machine_fdd_print(const t_fdd *fdd) {
    if (fdd == STD_NULL) return;
    STD_PRINTF("FDD INFO\n========\n");
    STD_PRINTF("cyl = %x, head = %x, sector = %x, gpl = %x\n",
           fdd->data.cyl, fdd->data.head, fdd->data.sector, fdd->data.gpl);
    STD_PRINTF("nsector = %x, nbyte = %x, ncyl = %x, nhead = %x\n",
           fdd->data.nsector, fdd->data.nbyte, fdd->data.ncyl,
           fdd->data.nhead);
    STD_PRINTF("ReadOnly = %x, Exist = %x\n",
           fdd->connect.flagReadOnly, fdd->connect.flagDiskExist);
    STD_PRINTF("media generation = %u\n", fdd->connect.media_generation);
}
