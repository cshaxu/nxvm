#include "lib/types/test.h"
#include "lib/types/file.h"
/* Real Linux open-file-description locking; all fixtures are temporary. */
#define _POSIX_C_SOURCE 200809L
#include "lib/storage/file.h"
#include "lib/types/linux/test.h"

int main(void)
{
    char path[] = "library-lock-XXXXXX";
    char alias[64];
    lib_i32 descriptor = mkstemp(path);
    lib_storage_file first = {0}, second = {0}, denied = {0};
    lib_test_assert(descriptor >= 0 && close(descriptor) == 0);
    lib_test_assert(lib_c_snprintf(alias,sizeof(alias),"%s-link",path) > 0);
    lib_test_assert(link(path,alias) == 0);

    lib_test_assert(storage_file_platform_open(path,LIB_TRUE,&first) == LIB_STATUS_OK);
    lib_test_assert(storage_file_platform_open(alias,LIB_TRUE,&denied) == LIB_STATUS_IO_ERROR);
    lib_test_assert(!denied.stream);
    lib_test_assert(storage_file_platform_open(path,LIB_FALSE,&denied) == LIB_STATUS_IO_ERROR);
    lib_test_assert(!denied.stream);
    /* An unrelated open/close must not drop the first descriptor's lock. */
    lib_c_file *unlocked = lib_c_fopen(path,"rb");
    lib_test_assert(unlocked && lib_c_fclose(unlocked) == 0);
    lib_test_assert(storage_file_platform_open(alias,LIB_TRUE,&denied) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_storage_file_close(&first) == LIB_STATUS_OK);

    lib_test_assert(storage_file_platform_open(path,LIB_FALSE,&first) == LIB_STATUS_OK);
    lib_test_assert(storage_file_platform_open(alias,LIB_FALSE,&second) == LIB_STATUS_OK);
    lib_test_assert(storage_file_platform_open(path,LIB_TRUE,&denied) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_storage_file_close(&first) == LIB_STATUS_OK);
    lib_test_assert(storage_file_platform_open(path,LIB_TRUE,&denied) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_storage_file_close(&second) == LIB_STATUS_OK);
    lib_test_assert(storage_file_platform_open(alias,LIB_TRUE,&first) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_close(&first) == LIB_STATUS_OK);
    lib_test_assert(unlink(alias) == 0 && unlink(path) == 0);
    return 0;
}
