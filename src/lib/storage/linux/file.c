#include "lib/base/base_interface.h"

#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "lib/storage/file_backend.h"

FILE *lib_storage_file_backend_open(const char *path, lib_storage_medium_mode mode)
{
    FILE *file = fopen(path, mode == LIB_STORAGE_MEDIUM_DIRECT ? "rb+" : "rb");
    struct flock lock = { 0 };

    if (file == LIB_NULL) return LIB_NULL;
    lock.l_type = mode == LIB_STORAGE_MEDIUM_DIRECT ? F_WRLCK : F_RDLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(fileno(file), F_SETLK, &lock) != 0) {
        (void)fclose(file);
        return LIB_NULL;
    }
    return file;
}

int lib_storage_file_backend_seek_64(FILE *file, lib_i64 offset, int origin)
{ return fseeko(file, (off_t)offset, origin); }

lib_i64 lib_storage_file_backend_tell_64(FILE *file)
{ return (lib_i64)ftello(file); }
