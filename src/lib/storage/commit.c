#include "lib/base/base.h"
#include "lib/storage/commit.h"
#include "lib/storage/file.h"

static const char suffix[] = ".tmp.000";

static char *backup_path(const char *path)
{
    static const char backup_suffix[] = ".bak";
    size_t length;
    char *result;

    if (path == LIB_NULL) return LIB_NULL;
    length = strlen(path);
    if (length > (size_t)-1 - sizeof(backup_suffix)) return LIB_NULL;
    result = malloc(length + sizeof(backup_suffix));
    if (result == LIB_NULL) return LIB_NULL;
    memcpy(result, path, length);
    memcpy(result + length, backup_suffix, sizeof(backup_suffix));
    if (lib_storage_file_exists(result)) {
        free(result);
        return LIB_NULL;
    }
    return result;
}

static int stage(const char *path, const void *bytes, size_t count, char **out)
{
    size_t length, index;
    char *candidate;

    if (path == LIB_NULL || out == LIB_NULL || (count != 0u && bytes == LIB_NULL))
        return LIB_TRUE;
    length = strlen(path);
    if (length > (size_t)-1 - sizeof(suffix)) return LIB_TRUE;
    candidate = malloc(length + sizeof(suffix));
    if (candidate == LIB_NULL) return LIB_TRUE;
    memcpy(candidate, path, length);
    memcpy(candidate + length, suffix, sizeof(suffix));
    for (index = 0u; index < 1000u; ++index) {
        candidate[length + sizeof(suffix) - 4u] = (char)('0' + index / 100u);
        candidate[length + sizeof(suffix) - 3u] = (char)('0' + index / 10u % 10u);
        candidate[length + sizeof(suffix) - 2u] = (char)('0' + index % 10u);
        if (lib_storage_file_write_exclusive(candidate, bytes, count) ==
            LIB_STORAGE_FILE_WRITE_OK) {
            *out = candidate;
            return LIB_FALSE;
        }
    }
    free(candidate);
    return LIB_TRUE;
}

int lib_storage_commit_atomically(const char *path, const void *bytes, size_t count)
{
    char *temporary = LIB_NULL;
    int failed;

    if (stage(path, bytes, count, &temporary) != LIB_FALSE) return LIB_TRUE;
    failed = lib_storage_file_replace(temporary, path);
    if (failed) (void)lib_storage_file_remove(temporary);
    free(temporary);
    return failed;
}

int lib_storage_commit_pair_atomically(const char *first_path,
    const void *first_bytes, size_t first_count, const char *second_path,
    const void *second_bytes, size_t second_count)
{
    char *first = LIB_NULL, *second = LIB_NULL;
    char *first_backup = LIB_NULL, *second_backup = LIB_NULL;
    int first_old, second_old, first_new = LIB_FALSE, second_new = LIB_FALSE;
    int failed = LIB_TRUE;

    if (stage(first_path, first_bytes, first_count, &first) != LIB_FALSE ||
        stage(second_path, second_bytes, second_count, &second) != LIB_FALSE ||
        (first_backup = backup_path(first_path)) == LIB_NULL ||
        (second_backup = backup_path(second_path)) == LIB_NULL) goto done;
    first_old = lib_storage_file_exists(first_path);
    second_old = lib_storage_file_exists(second_path);
    if ((first_old && lib_storage_file_replace(first_path, first_backup)) ||
        (second_old && lib_storage_file_replace(second_path, second_backup)) ||
        lib_storage_file_replace(first, first_path)) goto rollback;
    first_new = LIB_TRUE;
    if (lib_storage_file_replace(second, second_path)) goto rollback;
    second_new = LIB_TRUE;
    if ((first_old && lib_storage_file_remove(first_backup)) ||
        (second_old && lib_storage_file_remove(second_backup))) goto done;
    failed = LIB_FALSE;
    goto done;
rollback:
    if (first_new) (void)lib_storage_file_remove(first_path);
    if (second_new) (void)lib_storage_file_remove(second_path);
    if (first_old) (void)lib_storage_file_replace(first_backup, first_path);
    if (second_old) (void)lib_storage_file_replace(second_backup, second_path);
done:
    if (first != LIB_NULL) (void)lib_storage_file_remove(first);
    if (second != LIB_NULL) (void)lib_storage_file_remove(second);
    if (!failed) {
        if (first_backup != LIB_NULL) (void)lib_storage_file_remove(first_backup);
        if (second_backup != LIB_NULL) (void)lib_storage_file_remove(second_backup);
    }
    free(first); free(second); free(first_backup); free(second_backup);
    return failed;
}
