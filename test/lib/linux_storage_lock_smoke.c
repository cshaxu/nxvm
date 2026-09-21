/* Real Linux open-file-description locking; all fixtures are temporary. */
#define _POSIX_C_SOURCE 200809L
#include "lib/storage/file.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    char path[] = "library-lock-XXXXXX";
    char alias[64];
    int descriptor = mkstemp(path);
    lib_storage_file first = {0}, second = {0}, denied = {0};
    assert(descriptor >= 0 && close(descriptor) == 0);
    assert(snprintf(alias,sizeof(alias),"%s-link",path) > 0);
    assert(link(path,alias) == 0);

    assert(storage_file_platform_open(path,LIB_TRUE,&first) == LIB_STATUS_OK);
    assert(storage_file_platform_open(alias,LIB_TRUE,&denied) == LIB_STATUS_IO_ERROR);
    assert(!denied.stream);
    assert(storage_file_platform_open(path,LIB_FALSE,&denied) == LIB_STATUS_IO_ERROR);
    assert(!denied.stream);
    /* An unrelated open/close must not drop the first descriptor's lock. */
    FILE *unlocked = fopen(path,"rb");
    assert(unlocked && fclose(unlocked) == 0);
    assert(storage_file_platform_open(alias,LIB_TRUE,&denied) == LIB_STATUS_IO_ERROR);
    assert(lib_storage_file_close(&first) == LIB_STATUS_OK);

    assert(storage_file_platform_open(path,LIB_FALSE,&first) == LIB_STATUS_OK);
    assert(storage_file_platform_open(alias,LIB_FALSE,&second) == LIB_STATUS_OK);
    assert(storage_file_platform_open(path,LIB_TRUE,&denied) == LIB_STATUS_IO_ERROR);
    assert(lib_storage_file_close(&first) == LIB_STATUS_OK);
    assert(storage_file_platform_open(path,LIB_TRUE,&denied) == LIB_STATUS_IO_ERROR);
    assert(lib_storage_file_close(&second) == LIB_STATUS_OK);
    assert(storage_file_platform_open(alias,LIB_TRUE,&first) == LIB_STATUS_OK);
    assert(lib_storage_file_close(&first) == LIB_STATUS_OK);
    assert(unlink(alias) == 0 && unlink(path) == 0);
    return 0;
}
