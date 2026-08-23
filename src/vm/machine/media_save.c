#include "vm/machine/media_save.h"

static const C_CHAR vm_machine_media_save_suffix[] = ".ntvdm64.tmp.000";
static const STD_SIZE_T vm_machine_media_save_candidate_count = 1000u;

static C_INT vm_machine_media_stage(const C_CHAR *file_name, const C_VOID *bytes,
    STD_SIZE_T byte_count, C_CHAR **out_temporary_name)
{
    STD_SIZE_T file_name_length;
    STD_SIZE_T suffix_length = sizeof(vm_machine_media_save_suffix);
    C_CHAR *temporary_name;
    STD_FILE *image;
    STD_SIZE_T candidate_index;
    C_INT write_failed;
    C_INT close_failed;

    if (file_name == STD_NULL || out_temporary_name == STD_NULL ||
        (byte_count != 0u && bytes == STD_NULL)) return TYPE_TRUE;
    file_name_length = STD_STRLEN(file_name);
    if (file_name_length > (STD_SIZE_T)-1 - suffix_length) return TYPE_TRUE;
    temporary_name = (C_CHAR *)STD_MALLOC(file_name_length + suffix_length);
    if (temporary_name == STD_NULL) return TYPE_TRUE;
    STD_MEMCPY(temporary_name, file_name, file_name_length);
    STD_MEMCPY(temporary_name + file_name_length, vm_machine_media_save_suffix,
        suffix_length);
    image = STD_NULL;
    for (candidate_index = 0u; candidate_index < vm_machine_media_save_candidate_count;
         ++candidate_index) {
        temporary_name[file_name_length + suffix_length - 4u] =
            (C_CHAR)('0' + (candidate_index / 100u));
        temporary_name[file_name_length + suffix_length - 3u] =
            (C_CHAR)('0' + ((candidate_index / 10u) % 10u));
        temporary_name[file_name_length + suffix_length - 2u] =
            (C_CHAR)('0' + (candidate_index % 10u));
        image = STD_FOPEN_EXCLUSIVE_WRITE(temporary_name);
        if (image != STD_NULL) break;
    }
    if (image == STD_NULL) {
        STD_FREE(temporary_name);
        return TYPE_TRUE;
    }
    write_failed = byte_count != 0u &&
        STD_FWRITE(bytes, sizeof(type_unsigned_8), byte_count, image) != byte_count;
    close_failed = STD_FCLOSE(image) != 0;
    if (write_failed || close_failed) {
        (C_VOID)STD_REMOVE(temporary_name);
        STD_FREE(temporary_name);
        return TYPE_TRUE;
    }
    *out_temporary_name = temporary_name;
    return TYPE_FALSE;
}

static C_INT vm_machine_media_exists(const C_CHAR *file_name)
{
    STD_FILE *file = STD_FOPEN(file_name, "rb");

    if (file == STD_NULL) return TYPE_FALSE;
    return STD_FCLOSE(file) == 0 ? TYPE_TRUE : TYPE_FALSE;
}

static C_CHAR *vm_machine_media_backup_name(const C_CHAR *file_name)
{
    static const C_CHAR suffix[] = ".ntvdm64.bak";
    STD_SIZE_T length;
    C_CHAR *result;

    if (file_name == STD_NULL || (length = STD_STRLEN(file_name)) >
        (STD_SIZE_T)-1 - sizeof(suffix)) return STD_NULL;
    result = (C_CHAR *)STD_MALLOC(length + sizeof(suffix));
    if (result == STD_NULL) return STD_NULL;
    STD_MEMCPY(result, file_name, length);
    STD_MEMCPY(result + length, suffix, sizeof(suffix));
    if (vm_machine_media_exists(result)) {
        STD_FREE(result);
        return STD_NULL;
    }
    return result;
}

C_INT vm_machine_media_save_atomically(const C_CHAR *file_name,
    const C_VOID *bytes, STD_SIZE_T byte_count)
{
    C_CHAR *temporary_name = STD_NULL;

    if (vm_machine_media_stage(file_name, bytes, byte_count, &temporary_name) !=
        TYPE_FALSE) return TYPE_TRUE;
    if (STD_RENAME_REPLACE(temporary_name, file_name) != 0) {
        (C_VOID)STD_REMOVE(temporary_name);
        STD_FREE(temporary_name);
        return TYPE_TRUE;
    }
    STD_FREE(temporary_name);
    return TYPE_FALSE;
}

C_INT vm_machine_media_save_pair_atomically(const C_CHAR *first_name,
    const C_VOID *first_bytes, STD_SIZE_T first_byte_count,
    const C_CHAR *second_name, const C_VOID *second_bytes,
    STD_SIZE_T second_byte_count)
{
    C_CHAR *first_temporary = STD_NULL;
    C_CHAR *second_temporary = STD_NULL;
    C_CHAR *first_backup = STD_NULL;
    C_CHAR *second_backup = STD_NULL;
    C_INT first_old = TYPE_FALSE;
    C_INT second_old = TYPE_FALSE;
    C_INT first_new = TYPE_FALSE;
    C_INT second_new = TYPE_FALSE;
    C_INT failed = TYPE_TRUE;

    if (first_name == STD_NULL || second_name == STD_NULL ||
        vm_machine_media_stage(first_name, first_bytes, first_byte_count,
            &first_temporary) != TYPE_FALSE ||
        vm_machine_media_stage(second_name, second_bytes, second_byte_count,
            &second_temporary) != TYPE_FALSE ||
        (first_backup = vm_machine_media_backup_name(first_name)) == STD_NULL ||
        (second_backup = vm_machine_media_backup_name(second_name)) == STD_NULL) goto done;
    first_old = vm_machine_media_exists(first_name);
    second_old = vm_machine_media_exists(second_name);
    if ((first_old && STD_RENAME_REPLACE(first_name, first_backup) != 0) ||
        (second_old && STD_RENAME_REPLACE(second_name, second_backup) != 0) ||
        STD_RENAME_REPLACE(first_temporary, first_name) != 0) goto rollback;
    first_new = TYPE_TRUE;
    if (STD_RENAME_REPLACE(second_temporary, second_name) != 0) goto rollback;
    second_new = TYPE_TRUE;
    if ((first_old && STD_REMOVE(first_backup) != 0) ||
        (second_old && STD_REMOVE(second_backup) != 0)) goto done;
    failed = TYPE_FALSE;
    goto done;
rollback:
    if (first_new) (C_VOID)STD_REMOVE(first_name);
    if (second_new) (C_VOID)STD_REMOVE(second_name);
    if (first_old) (C_VOID)STD_RENAME_REPLACE(first_backup, first_name);
    if (second_old) (C_VOID)STD_RENAME_REPLACE(second_backup, second_name);
done:
    if (first_temporary != STD_NULL) (C_VOID)STD_REMOVE(first_temporary);
    if (second_temporary != STD_NULL) (C_VOID)STD_REMOVE(second_temporary);
    if (!failed) {
        if (first_backup != STD_NULL) (C_VOID)STD_REMOVE(first_backup);
        if (second_backup != STD_NULL) (C_VOID)STD_REMOVE(second_backup);
    }
    STD_FREE(first_temporary);
    STD_FREE(second_temporary);
    STD_FREE(first_backup);
    STD_FREE(second_backup);
    return failed;
}
