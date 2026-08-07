#include "vm/machine/media_save.h"

static const C_CHAR vm_machine_media_save_suffix[] = ".ntvdm64.tmp.000";
static const STD_SIZE_T vm_machine_media_save_candidate_count = 1000u;

C_INT vm_machine_media_save_atomically(const C_CHAR *file_name,
    const C_VOID *bytes, STD_SIZE_T byte_count)
{
    STD_SIZE_T file_name_length;
    STD_SIZE_T suffix_length = sizeof(vm_machine_media_save_suffix);
    C_CHAR *temporary_name;
    STD_FILE *image;
    STD_SIZE_T candidate_index;
    C_INT write_failed;
    C_INT close_failed;

    if (file_name == STD_NULL || (byte_count != 0u && bytes == STD_NULL)) {
        return TYPE_TRUE;
    }
    file_name_length = STD_STRLEN(file_name);
    if (file_name_length > (STD_SIZE_T)-1 - suffix_length) return TYPE_TRUE;
    temporary_name = (C_CHAR *)STD_MALLOC(file_name_length + suffix_length);
    if (temporary_name == STD_NULL) return TYPE_TRUE;
    STD_MEMCPY(temporary_name, file_name, file_name_length);
    STD_MEMCPY(temporary_name + file_name_length, vm_machine_media_save_suffix,
        suffix_length);

    image = STD_NULL;
    for (candidate_index = 0u;
         candidate_index < vm_machine_media_save_candidate_count;
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
    write_failed = byte_count != 0u && STD_FWRITE(bytes, sizeof(type_unsigned_8),
        byte_count, image) != byte_count;
    close_failed = STD_FCLOSE(image) != 0;
    if (write_failed || close_failed) {
        (C_VOID)STD_REMOVE(temporary_name);
        STD_FREE(temporary_name);
        return TYPE_TRUE;
    }
    if (STD_RENAME_REPLACE(temporary_name, file_name) != 0) {
        (C_VOID)STD_REMOVE(temporary_name);
        STD_FREE(temporary_name);
        return TYPE_TRUE;
    }
    STD_FREE(temporary_name);
    return TYPE_FALSE;
}
