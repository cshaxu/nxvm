#define _POSIX_C_SOURCE 200809L
#include "lib/types/file.h"
#include "lib/types/types_interface.h"

#include "lib/types/linux/file.h"

#include "lib/storage/file.h"

lib_status storage_file_platform_open(const char *path, lib_bool readwrite,
    lib_storage_file *file)
{
    file->stream = lib_c_fopen(path,
        readwrite != LIB_FALSE ? "rb+" : "rb");
    if (file->stream == LIB_NULL) {
        return LIB_STATUS_IO_ERROR;
    }
    /* flock belongs to this open file description, not to the process. */
    if (lib_linux_flock(lib_linux_fileno(file->stream), LIB_LINUX_LOCK_NB |
            (readwrite != LIB_FALSE ? LIB_LINUX_LOCK_EX : LIB_LINUX_LOCK_SH)) != 0) {
        (void)lib_c_fclose(file->stream);
        file->stream = LIB_NULL;
        return LIB_STATUS_IO_ERROR;
    }
    return LIB_STATUS_OK;
}

lib_status storage_file_platform_seek(const lib_storage_file *file, lib_i64 offset,
    int origin)
{ return lib_linux_fseeko(file->stream, (lib_linux_off_t)offset, origin) == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

lib_i64 storage_file_platform_tell(const lib_storage_file *file)
{ return (lib_i64)lib_linux_ftello(file->stream); }
