#ifndef LIB_BASE_H
#define LIB_BASE_H

#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

typedef uint8_t lib_u8;
typedef uint16_t lib_u16;
typedef uint32_t lib_u32;
typedef uint64_t lib_u64;
typedef int32_t lib_i32;
typedef int lib_bool;

typedef int lib_status;

enum {
    LIB_STATUS_OK = 0,
    LIB_STATUS_INVALID_ARGUMENT = 1,
    LIB_STATUS_INVALID_STATE = 2,
    LIB_STATUS_UNSUPPORTED = 3,
    LIB_STATUS_NO_MEMORY = 4,
    LIB_STATUS_IO_ERROR = 5
};

#define LIB_FALSE 0
#define LIB_TRUE 1
#define LIB_NULL NULL

static inline FILE *lib_base_fopen_exclusive_write(const char *path)
{
    int descriptor;
    FILE *file;

    if (path == NULL) return NULL;
#ifdef _WIN32
    descriptor = _open(path, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY,
        _S_IREAD | _S_IWRITE);
    if (descriptor < 0) return NULL;
    file = _fdopen(descriptor, "wb");
#else
    descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (descriptor < 0) return NULL;
    file = fdopen(descriptor, "wb");
#endif
    if (file != NULL) return file;
#ifdef _WIN32
    (void)_close(descriptor);
#else
    (void)close(descriptor);
#endif
    (void)remove(path);
    return NULL;
}

static inline int lib_base_fseek_64(FILE *file, int64_t offset, int origin)
{
#ifdef _WIN32
    return _fseeki64(file, offset, origin);
#else
    return fseeko(file, (off_t)offset, origin);
#endif
}

static inline int64_t lib_base_ftell_64(FILE *file)
{
#ifdef _WIN32
    return _ftelli64(file);
#else
    return (int64_t)ftello(file);
#endif
}

#endif
