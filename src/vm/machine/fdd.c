/* Copyright 2012-2014 Neko. */

/* VFDD implements Floppy Disk Drive: 3.5" 1.44MB. */

#include "type.h"



#include "core/machine/dma.h"


#include "vm/machine/fdd.h"
#include "vm/machine/media_save.h"

static C_CHAR *vm_machine_fdd_sidecar_name(const C_CHAR *image_name)
{
    static const C_CHAR suffix[] = ".json";
    STD_SIZE_T length;
    C_CHAR *result;

    if (image_name == STD_NULL || (length = STD_STRLEN(image_name)) >
        (STD_SIZE_T)-1 - sizeof(suffix)) return STD_NULL;
    result = (C_CHAR *)STD_MALLOC(length + sizeof(suffix));
    if (result == STD_NULL) return STD_NULL;
    STD_MEMCPY(result, image_name, length);
    STD_MEMCPY(result + length, suffix, sizeof(suffix));
    return result;
}

static type_unsigned_32 vm_machine_fdd_checksum(const C_VOID *bytes, STD_SIZE_T count)
{
    const type_unsigned_8 *cursor = (const type_unsigned_8 *)bytes;
    type_unsigned_32 result = 2166136261u;

    while (count-- != 0u) {
        result ^= *cursor++;
        result *= 16777619u;
    }
    return result;
}

static C_VOID vm_machine_fdd_json_skip(const C_CHAR **cursor)
{
    while (**cursor == ' ' || **cursor == '\t' || **cursor == '\r' || **cursor == '\n')
        ++*cursor;
}

static C_INT vm_machine_fdd_json_literal(const C_CHAR **cursor, const C_CHAR *literal)
{
    STD_SIZE_T length = STD_STRLEN(literal);

    vm_machine_fdd_json_skip(cursor);
    if (STD_MEMCMP(*cursor, literal, length) != 0) return TYPE_TRUE;
    *cursor += length;
    return TYPE_FALSE;
}

static C_INT vm_machine_fdd_json_unsigned(const C_CHAR **cursor,
    type_unsigned_32 *out_value)
{
    type_unsigned_64 value = 0u;
    C_INT found = 0;

    vm_machine_fdd_json_skip(cursor);
    while (**cursor >= '0' && **cursor <= '9') {
        found = 1;
        value = value * 10u + (type_unsigned_32)(**cursor - '0');
        if (value > TYPE_MAX_UNSIGNED_32) return TYPE_TRUE;
        ++*cursor;
    }
    if (!found) return TYPE_TRUE;
    *out_value = (type_unsigned_32)value;
    return TYPE_FALSE;
}

static C_INT vm_machine_fdd_sidecar_load(const t_fdd *fdd, const C_CHAR *image_name,
    const C_VOID *image_bytes, type_virtual_address marks)
{
    C_CHAR *sidecar_name;
    STD_FILE *file;
    type_signed_64 length;
    C_CHAR *text;
    const C_CHAR *cursor;
    type_unsigned_32 version;
    type_unsigned_32 raw_size;
    type_unsigned_32 checksum;
    type_unsigned_32 geometry[4];
    STD_SIZE_T sector_count;
    STD_SIZE_T index;
    C_INT result = TYPE_TRUE;

    sidecar_name = vm_machine_fdd_sidecar_name(image_name);
    if (sidecar_name == STD_NULL) return TYPE_TRUE;
    file = STD_FOPEN(sidecar_name, "rb");
    STD_FREE(sidecar_name);
    if (file == STD_NULL) return TYPE_FALSE;
    if (STD_FSEEK_64(file, 0, STD_SEEK_END) != 0 ||
        (length = STD_FTELL_64(file)) < 0 || (type_unsigned_64)length > 65536u ||
        STD_FSEEK_64(file, 0, STD_SEEK_SET) != 0 ||
        (text = (C_CHAR *)STD_MALLOC((STD_SIZE_T)length + 1u)) == STD_NULL) {
        (C_VOID)STD_FCLOSE(file);
        return TYPE_TRUE;
    }
    if (STD_FREAD(text, 1u, (STD_SIZE_T)length, file) != (STD_SIZE_T)length ||
        STD_FCLOSE(file) != 0) {
        STD_FREE(text);
        return TYPE_TRUE;
    }
    text[length] = '\0';
    cursor = text;
    sector_count = (STD_SIZE_T)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (vm_machine_fdd_json_literal(&cursor, "{") ||
        vm_machine_fdd_json_literal(&cursor, "\"version\"") ||
        vm_machine_fdd_json_literal(&cursor, ":") ||
        vm_machine_fdd_json_unsigned(&cursor, &version) || version != 1u ||
        vm_machine_fdd_json_literal(&cursor, ",") ||
        vm_machine_fdd_json_literal(&cursor, "\"raw_size\"") ||
        vm_machine_fdd_json_literal(&cursor, ":") ||
        vm_machine_fdd_json_unsigned(&cursor, &raw_size) ||
        raw_size != vm_machine_fdd_image_size(fdd) ||
        vm_machine_fdd_json_literal(&cursor, ",") ||
        vm_machine_fdd_json_literal(&cursor, "\"raw_checksum\"") ||
        vm_machine_fdd_json_literal(&cursor, ":") ||
        vm_machine_fdd_json_unsigned(&cursor, &checksum) ||
        checksum != vm_machine_fdd_checksum(image_bytes, vm_machine_fdd_image_size(fdd)) ||
        vm_machine_fdd_json_literal(&cursor, ",") ||
        vm_machine_fdd_json_literal(&cursor, "\"geometry\"") ||
        vm_machine_fdd_json_literal(&cursor, ":") ||
        vm_machine_fdd_json_literal(&cursor, "[") ||
        vm_machine_fdd_json_unsigned(&cursor, &geometry[0]) ||
        vm_machine_fdd_json_literal(&cursor, ",") ||
        vm_machine_fdd_json_unsigned(&cursor, &geometry[1]) ||
        vm_machine_fdd_json_literal(&cursor, ",") ||
        vm_machine_fdd_json_unsigned(&cursor, &geometry[2]) ||
        vm_machine_fdd_json_literal(&cursor, ",") ||
        vm_machine_fdd_json_unsigned(&cursor, &geometry[3]) ||
        vm_machine_fdd_json_literal(&cursor, "]") ||
        geometry[0] != fdd->data.ncyl || geometry[1] != fdd->data.nhead ||
        geometry[2] != fdd->data.nsector || geometry[3] != fdd->data.nbyte ||
        vm_machine_fdd_json_literal(&cursor, ",") ||
        vm_machine_fdd_json_literal(&cursor, "\"deleted\"") ||
        vm_machine_fdd_json_literal(&cursor, ":\"") ) goto done;
    for (index = 0u; index < sector_count; ++index) {
        if (*cursor != '0' && *cursor != '1') goto done;
        TYPE_DEREFERENCE_UNSIGNED_8(marks + index) = *cursor++ == '1';
    }
    if (vm_machine_fdd_json_literal(&cursor, "\"") ||
        vm_machine_fdd_json_literal(&cursor, "}") ) goto done;
    vm_machine_fdd_json_skip(&cursor);
    if (*cursor != '\0') goto done;
    result = TYPE_FALSE;
done:
    STD_FREE(text);
    return result;
}

static C_INT vm_machine_fdd_sidecar_serialize(const t_fdd *fdd,
    const C_VOID *image_bytes, C_VOID **out_bytes, STD_SIZE_T *out_count)
{
    static const C_CHAR prefix[] =
        "{\"version\":1,\"raw_size\":%u,\"raw_checksum\":%u,\"geometry\":[%u,%u,%u,%u],\"deleted\":\"";
    static const C_CHAR suffix[] = "\"}";
    STD_SIZE_T sector_count;
    STD_SIZE_T capacity;
    C_CHAR *text;
    C_INT written;
    STD_SIZE_T index;

    if (fdd == STD_NULL || image_bytes == STD_NULL || out_bytes == STD_NULL ||
        out_count == STD_NULL || fdd->connect.pAddressMarks == (type_virtual_address)STD_NULL)
        return TYPE_TRUE;
    sector_count = (STD_SIZE_T)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    capacity = sizeof(prefix) + 4u * 10u + sector_count + sizeof(suffix);
    text = (C_CHAR *)STD_MALLOC(capacity);
    if (text == STD_NULL) return TYPE_TRUE;
    written = STD_SNPRINTF(text, capacity, prefix, (type_unsigned_32)vm_machine_fdd_image_size(fdd),
        vm_machine_fdd_checksum(image_bytes, vm_machine_fdd_image_size(fdd)),
        (type_unsigned_32)fdd->data.ncyl, (type_unsigned_32)fdd->data.nhead,
        (type_unsigned_32)fdd->data.nsector, (type_unsigned_32)fdd->data.nbyte);
    if (written < 0 || (STD_SIZE_T)written >= capacity) {
        STD_FREE(text);
        return TYPE_TRUE;
    }
    for (index = 0u; index < sector_count; ++index) {
        text[written + index] = TYPE_DEREFERENCE_UNSIGNED_8(
            fdd->connect.pAddressMarks + index) ? '1' : '0';
    }
    text[written + sector_count] = '"';
    text[written + sector_count + 1u] = '}';
    *out_bytes = text;
    *out_count = (STD_SIZE_T)written + sector_count + 2u;
    return TYPE_FALSE;
}

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
    if (buffer == STD_NULL || fdd->connect.pImgBase == (type_virtual_address)STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    image_size = vm_machine_fdd_image_size(fdd);
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMCPY(buffer, (const C_VOID *)(fdd->connect.pImgBase + offset), byte_count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_fdd_media_write(C_VOID *context,
    type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 byte_count)
{
    t_fdd *fdd = (t_fdd *)context;
    STD_SIZE_T image_size;

    if (fdd == STD_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (buffer == STD_NULL || fdd->connect.pImgBase == (type_virtual_address)STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    image_size = vm_machine_fdd_image_size(fdd);
    if (offset > image_size || byte_count > image_size - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMCPY((C_VOID *)(fdd->connect.pImgBase + offset), buffer, byte_count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result vm_machine_fdd_media_format(C_VOID *context,
    type_unsigned_64 logical_sector, type_unsigned_32 sector_count, type_unsigned_8 fill)
{
    t_fdd *fdd = (t_fdd *)context;
    type_unsigned_64 sector_total;

    if (fdd == STD_NULL || !fdd->connect.flagDiskExist)
        return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fdd->connect.flagReadOnly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (fdd->connect.pImgBase == (type_virtual_address)STD_NULL)
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    sector_total = (type_unsigned_64)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector;
    if (logical_sector >= sector_total || sector_count > sector_total - logical_sector)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMSET((C_VOID *)(fdd->connect.pImgBase + logical_sector * fdd->data.nbyte),
        fill, sector_count * fdd->data.nbyte);
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
    *out_mark = TYPE_DEREFERENCE_UNSIGNED_8(fdd->connect.pAddressMarks + logical_sector) ?
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
    TYPE_DEREFERENCE_UNSIGNED_8(fdd->connect.pAddressMarks + logical_sector) =
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

STD_SIZE_T vm_machine_fdd_image_size(const t_fdd *fdd)
{
    return (STD_SIZE_T)fdd->data.nbyte * fdd->data.nsector * fdd->data.nhead *
        fdd->data.ncyl;
}

static C_VOID vm_machine_fdd_commit_candidate(t_fdd *fdd,
    type_virtual_address candidate, type_virtual_address marks)
{
    type_virtual_address old_image = fdd->connect.pImgBase;
    type_virtual_address old_marks = fdd->connect.pAddressMarks;

    fdd->connect.pImgBase = candidate;
    fdd->connect.pAddressMarks = marks;
    fdd->connect.flagDiskExist = TYPE_TRUE;
    ++fdd->connect.media_generation;
    if (old_image != (type_virtual_address)STD_NULL) {
        STD_FREE((C_VOID *)old_image);
    }
    if (old_marks != (type_virtual_address)STD_NULL) {
        STD_FREE((C_VOID *)old_marks);
    }
}

C_INT vm_machine_fdd_replace_bytes(t_fdd *fdd, const C_VOID *bytes,
    STD_SIZE_T byte_count)
{
    STD_SIZE_T image_size;
    type_virtual_address candidate = (type_virtual_address)STD_NULL;
    type_virtual_address marks = (type_virtual_address)STD_NULL;

    if (fdd == STD_NULL || bytes == STD_NULL ||
        byte_count != (image_size = vm_machine_fdd_image_size(fdd)) ||
        (candidate = (type_virtual_address)STD_MALLOC(image_size)) ==
            (type_virtual_address)STD_NULL ||
        (marks = (type_virtual_address)STD_CALLOC((STD_SIZE_T)fdd->data.ncyl *
            fdd->data.nhead * fdd->data.nsector, sizeof(type_unsigned_8))) ==
            (type_virtual_address)STD_NULL) {
        if (candidate != (type_virtual_address)STD_NULL) STD_FREE((C_VOID *)candidate);
        return TYPE_TRUE;
    }
    STD_MEMCPY((C_VOID *)candidate, bytes, image_size);
    vm_machine_fdd_commit_candidate(fdd, candidate, marks);
    return TYPE_FALSE;
}

static type_virtual_address vm_machine_fdd_byte_address(const t_fdd *fdd,
    type_unsigned_16 cylinder, type_unsigned_16 head,
    type_unsigned_16 sector, type_unsigned_16 offset)
{
    return fdd->connect.pImgBase + (((cylinder * fdd->data.nhead + head) *
        fdd->data.nsector + (sector - 1u)) * fdd->data.nbyte) + offset;
}

C_INT vm_machine_fdd_chs_valid(const t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 bytes)
{
    return fdd != STD_NULL && fdd->connect.flagDiskExist &&
        fdd->connect.pImgBase != (type_virtual_address)STD_NULL &&
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
    *out_byte = TYPE_DEREFERENCE_UNSIGNED_8(vm_machine_fdd_byte_address(fdd,
        cylinder, head, sector, offset));
    return TYPE_FALSE;
}

C_INT vm_machine_fdd_write_byte(t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 offset,
    type_unsigned_8 value)
{
    if (fdd == STD_NULL || fdd->connect.flagReadOnly || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte) || offset >= fdd->data.nbyte) {
        return TYPE_TRUE;
    }
    TYPE_DEREFERENCE_UNSIGNED_8(vm_machine_fdd_byte_address(fdd, cylinder, head,
        sector, offset)) = value;
    return TYPE_FALSE;
}

C_INT vm_machine_fdd_format_sector(t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_8 fill_byte)
{
    if (fdd == STD_NULL || fdd->connect.flagReadOnly || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte)) return TYPE_TRUE;
    STD_MEMSET((C_VOID *)vm_machine_fdd_byte_address(fdd, cylinder, head, sector,
        0u), fill_byte, fdd->data.nbyte);
    ++fdd->connect.media_generation;
    return TYPE_FALSE;
}

C_VOID vm_machine_fdd_initialize(t_fdd *fdd)
{
    if (fdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)fdd, TYPE_ZERO_8, sizeof(*fdd));
    fdd->data.ncyl = 0x0050;
    fdd->data.nhead = 0x0002;
    fdd->data.nsector = 0x0012;
    fdd->data.nbyte = 0x0200;
    fdd->connect.pImgBase = (type_virtual_address)STD_MALLOC(vm_machine_fdd_image_size(fdd));
    fdd->connect.pAddressMarks = (type_virtual_address)STD_CALLOC(
        (STD_SIZE_T)fdd->data.ncyl * fdd->data.nhead * fdd->data.nsector,
        sizeof(type_unsigned_8));
    if (fdd->connect.pImgBase != (type_virtual_address)STD_NULL) {
        STD_MEMSET((C_VOID *)fdd->connect.pImgBase, TYPE_ZERO_8,
            vm_machine_fdd_image_size(fdd));
    }
}

C_VOID vm_machine_fdd_reset(t_fdd *fdd)
{
    if (fdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)&fdd->data, TYPE_ZERO_8, sizeof(fdd->data));
    fdd->data.ncyl = 0x0050;
    fdd->data.nhead = 0x0002;
    fdd->data.nsector = 0x0012;
    fdd->data.nbyte = 0x0200;
}

C_VOID vm_machine_fdd_refresh(t_fdd *fdd) { (C_VOID)fdd; }

C_VOID vm_machine_fdd_finalize(t_fdd *fdd)
{
    if (fdd != STD_NULL && fdd->connect.pImgBase) STD_FREE((C_VOID *)fdd->connect.pImgBase);
    if (fdd != STD_NULL && fdd->connect.pAddressMarks)
        STD_FREE((C_VOID *)fdd->connect.pAddressMarks);
    if (fdd != STD_NULL) fdd->connect.pImgBase = (type_virtual_address)STD_NULL;
    if (fdd != STD_NULL) fdd->connect.pAddressMarks = (type_virtual_address)STD_NULL;
}

C_VOID vm_machine_fdd_create_for(t_fdd *fdd)
{
    if (fdd != STD_NULL && fdd->connect.pImgBase != (type_virtual_address)STD_NULL &&
        fdd->connect.pAddressMarks != (type_virtual_address)STD_NULL) {
        fdd->connect.flagDiskExist = TYPE_TRUE;
        fdd->connect.media_generation++;
    }
}

C_INT vm_machine_fdd_insert_for(t_fdd *fdd, const C_CHAR *file_name)
{
    type_signed_64 image_length;
    STD_SIZE_T image_size;
    type_virtual_address candidate = (type_virtual_address)STD_NULL;
    type_virtual_address marks = (type_virtual_address)STD_NULL;
    STD_FILE *image;

    if (fdd == STD_NULL || file_name == STD_NULL ||
        (image = STD_FOPEN(file_name, "rb")) == STD_NULL) return TYPE_TRUE;
    image_size = vm_machine_fdd_image_size(fdd);
    if (STD_FSEEK_64(image, 0, STD_SEEK_END) != 0 ||
        (image_length = STD_FTELL_64(image)) < 0 || (STD_SIZE_T)image_length != image_size ||
        STD_FSEEK_64(image, 0, STD_SEEK_SET) != 0 ||
        (candidate = (type_virtual_address)STD_MALLOC(image_size)) ==
            (type_virtual_address)STD_NULL ||
        (marks = (type_virtual_address)STD_CALLOC((STD_SIZE_T)fdd->data.ncyl *
            fdd->data.nhead * fdd->data.nsector, sizeof(type_unsigned_8))) ==
            (type_virtual_address)STD_NULL) {
        if (candidate != (type_virtual_address)STD_NULL) STD_FREE((C_VOID *)candidate);
        (C_VOID)STD_FCLOSE(image);
        return TYPE_TRUE;
    }
    if (STD_FREAD((C_VOID *)candidate, sizeof(type_unsigned_8), image_size, image) !=
            image_size || STD_FCLOSE(image) != 0) {
        STD_FREE((C_VOID *)candidate);
        STD_FREE((C_VOID *)marks);
        return TYPE_TRUE;
    }
    if (vm_machine_fdd_sidecar_load(fdd, file_name, (const C_VOID *)candidate,
        marks) != TYPE_FALSE) {
        STD_FREE((C_VOID *)candidate);
        STD_FREE((C_VOID *)marks);
        return TYPE_TRUE;
    }
    vm_machine_fdd_commit_candidate(fdd, candidate, marks);
    return TYPE_FALSE;
}

C_INT vm_machine_fdd_remove_for(t_fdd *fdd, const C_CHAR *file_name)
{
    C_CHAR *sidecar_name;
    C_VOID *sidecar_bytes;
    STD_SIZE_T sidecar_byte_count;

    if (fdd == STD_NULL) return TYPE_TRUE;
    if (file_name != STD_NULL && !fdd->connect.flagReadOnly) {
        sidecar_name = vm_machine_fdd_sidecar_name(file_name);
        if (sidecar_name == STD_NULL || vm_machine_fdd_sidecar_serialize(fdd,
            (const C_VOID *)fdd->connect.pImgBase, &sidecar_bytes,
            &sidecar_byte_count) != TYPE_FALSE ||
            vm_machine_media_save_pair_atomically(file_name,
                (const C_VOID *)fdd->connect.pImgBase, vm_machine_fdd_image_size(fdd),
                sidecar_name, sidecar_bytes, sidecar_byte_count) != TYPE_FALSE) {
            STD_FREE(sidecar_name);
            return TYPE_TRUE;
        }
        STD_FREE(sidecar_bytes);
        STD_FREE(sidecar_name);
    }
    fdd->connect.flagDiskExist = TYPE_FALSE;
    fdd->connect.media_generation++;
    if (fdd->connect.pImgBase != (type_virtual_address)STD_NULL) {
        STD_MEMSET((C_VOID *)fdd->connect.pImgBase, TYPE_ZERO_8,
            vm_machine_fdd_image_size(fdd));
    }
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
    STD_PRINTF("base = %x, media generation = %u\n",
           fdd->connect.pImgBase, fdd->connect.media_generation);
}
