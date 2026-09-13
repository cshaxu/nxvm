#include "lib/types/file.h"
#include "lib/types/types_interface.h"

#include "lib/types/linux/file.h"

#include "lib/storage/file.h"

static lib_status storage_file_platform_open(const char *path, lib_bool readwrite,
    lib_storage_file *file)
{
    lib_linux_file_lock lock = { 0 };

    file->stream = lib_c_fopen(path,
        readwrite != LIB_FALSE ? "rb+" : "rb");
    if (file->stream == LIB_NULL) {
        return LIB_STATUS_IO_ERROR;
    }
    lock.l_type = readwrite != LIB_FALSE ? LIB_LINUX_F_WRLCK : LIB_LINUX_F_RDLCK;
    lock.l_whence = LIB_SEEK_SET;
    if (lib_linux_fcntl(lib_linux_fileno(file->stream), LIB_LINUX_F_SETLK, &lock) != 0) {
        (void)lib_c_fclose(file->stream);
        return LIB_STATUS_IO_ERROR;
    }
    return LIB_STATUS_OK;
}

lib_status storage_file_platform_open_readonly(const char *path,
    lib_storage_file *file)
{ return storage_file_platform_open(path, LIB_FALSE, file); }

lib_status storage_file_platform_open_readwrite(const char *path,
    lib_storage_file *file)
{ return storage_file_platform_open(path, LIB_TRUE, file); }

lib_status storage_file_platform_seek_absolute(lib_storage_file *file, lib_i64 offset)
{ return lib_linux_fseeko(file->stream, (lib_linux_off_t)offset, LIB_SEEK_SET) == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

lib_status storage_file_platform_byte_count(lib_storage_file *file, lib_i64 *out_byte_count)
{
    lib_linux_off_t offset = lib_linux_ftello(file->stream);
    lib_linux_off_t length;

    if (offset < 0 || lib_linux_fseeko(file->stream, 0, LIB_SEEK_END) != 0)
        return LIB_STATUS_IO_ERROR;
    length = lib_linux_ftello(file->stream);
    if (length < 0 || lib_linux_fseeko(file->stream, offset, LIB_SEEK_SET) != 0)
        return LIB_STATUS_IO_ERROR;
    *out_byte_count = (lib_i64)length;
    return LIB_STATUS_OK;
}
