#include "type.h"

#include "lib/storage/commit.h"
#include "lib/storage/file.h"

static const C_CHAR lib_storage_commit_suffix[] = ".ntvdm64.tmp.000";
static const STD_SIZE_T lib_storage_commit_candidate_count = 1000u;

static C_INT lib_storage_commit_stage(const C_CHAR *path, const C_VOID *bytes,
    STD_SIZE_T byte_count, C_CHAR **out_temporary_path)
{
    const STD_SIZE_T path_length = path == STD_NULL ? 0u : STD_STRLEN(path);
    const STD_SIZE_T suffix_length = sizeof(lib_storage_commit_suffix);
    C_CHAR *temporary_path;
    STD_SIZE_T candidate;

    if (path == STD_NULL || out_temporary_path == STD_NULL ||
        (byte_count != 0u && bytes == STD_NULL) ||
        path_length > (STD_SIZE_T)-1 - suffix_length) return TYPE_TRUE;
    temporary_path = STD_MALLOC(path_length + suffix_length);
    if (temporary_path == STD_NULL) return TYPE_TRUE;
    STD_MEMCPY(temporary_path, path, path_length);
    STD_MEMCPY(temporary_path + path_length, lib_storage_commit_suffix, suffix_length);
    for (candidate = 0u; candidate < lib_storage_commit_candidate_count; ++candidate) {
        temporary_path[path_length + suffix_length - 4u] = (C_CHAR)('0' + candidate / 100u);
        temporary_path[path_length + suffix_length - 3u] = (C_CHAR)('0' + candidate / 10u % 10u);
        temporary_path[path_length + suffix_length - 2u] = (C_CHAR)('0' + candidate % 10u);
        if (lib_storage_file_write_exclusive(temporary_path, bytes, byte_count) ==
                LIB_STORAGE_FILE_WRITE_OK) {
            *out_temporary_path = temporary_path;
            return TYPE_FALSE;
        }
    }
    STD_FREE(temporary_path);
    return TYPE_TRUE;
}

static C_CHAR *lib_storage_commit_backup_path(const C_CHAR *path)
{
    static const C_CHAR suffix[] = ".ntvdm64.bak";
    const STD_SIZE_T length = path == STD_NULL ? 0u : STD_STRLEN(path);
    C_CHAR *result;

    if (path == STD_NULL || length > (STD_SIZE_T)-1 - sizeof(suffix)) return STD_NULL;
    result = STD_MALLOC(length + sizeof(suffix));
    if (result == STD_NULL) return STD_NULL;
    STD_MEMCPY(result, path, length);
    STD_MEMCPY(result + length, suffix, sizeof(suffix));
    if (lib_storage_file_exists(result)) {
        STD_FREE(result);
        return STD_NULL;
    }
    return result;
}

C_INT lib_storage_commit_atomically(const C_CHAR *path, const C_VOID *bytes,
    STD_SIZE_T byte_count)
{
    C_CHAR *temporary_path = STD_NULL;
    C_INT failed;

    if (lib_storage_commit_stage(path, bytes, byte_count, &temporary_path) != TYPE_FALSE) {
        return TYPE_TRUE;
    }
    failed = lib_storage_file_replace(temporary_path, path);
    if (failed) (C_VOID)lib_storage_file_remove(temporary_path);
    STD_FREE(temporary_path);
    return failed;
}

C_INT lib_storage_commit_pair_atomically(const C_CHAR *first_path,
    const C_VOID *first_bytes, STD_SIZE_T first_byte_count, const C_CHAR *second_path,
    const C_VOID *second_bytes, STD_SIZE_T second_byte_count)
{
    C_CHAR *first_temporary = STD_NULL;
    C_CHAR *second_temporary = STD_NULL;
    C_CHAR *first_backup = STD_NULL;
    C_CHAR *second_backup = STD_NULL;
    C_INT first_old;
    C_INT second_old;
    C_INT first_new = TYPE_FALSE;
    C_INT second_new = TYPE_FALSE;
    C_INT failed = TYPE_TRUE;

    if (first_path == STD_NULL || second_path == STD_NULL ||
        lib_storage_commit_stage(first_path, first_bytes, first_byte_count,
            &first_temporary) != TYPE_FALSE ||
        lib_storage_commit_stage(second_path, second_bytes, second_byte_count,
            &second_temporary) != TYPE_FALSE ||
        (first_backup = lib_storage_commit_backup_path(first_path)) == STD_NULL ||
        (second_backup = lib_storage_commit_backup_path(second_path)) == STD_NULL) goto done;
    first_old = lib_storage_file_exists(first_path);
    second_old = lib_storage_file_exists(second_path);
    if ((first_old && lib_storage_file_replace(first_path, first_backup) != TYPE_FALSE) ||
        (second_old && lib_storage_file_replace(second_path, second_backup) != TYPE_FALSE) ||
        lib_storage_file_replace(first_temporary, first_path) != TYPE_FALSE) goto rollback;
    first_new = TYPE_TRUE;
    if (lib_storage_file_replace(second_temporary, second_path) != TYPE_FALSE) goto rollback;
    second_new = TYPE_TRUE;
    if ((first_old && lib_storage_file_remove(first_backup) != TYPE_FALSE) ||
        (second_old && lib_storage_file_remove(second_backup) != TYPE_FALSE)) goto done;
    failed = TYPE_FALSE;
    goto done;
rollback:
    if (first_new) (C_VOID)lib_storage_file_remove(first_path);
    if (second_new) (C_VOID)lib_storage_file_remove(second_path);
    if (first_old) (C_VOID)lib_storage_file_replace(first_backup, first_path);
    if (second_old) (C_VOID)lib_storage_file_replace(second_backup, second_path);
done:
    if (first_temporary != STD_NULL) (C_VOID)lib_storage_file_remove(first_temporary);
    if (second_temporary != STD_NULL) (C_VOID)lib_storage_file_remove(second_temporary);
    if (!failed) {
        if (first_backup != STD_NULL) (C_VOID)lib_storage_file_remove(first_backup);
        if (second_backup != STD_NULL) (C_VOID)lib_storage_file_remove(second_backup);
    }
    STD_FREE(first_temporary);
    STD_FREE(second_temporary);
    STD_FREE(first_backup);
    STD_FREE(second_backup);
    return failed;
}
