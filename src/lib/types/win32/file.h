#ifndef LIB_TYPES_WIN32_FILE_H
#define LIB_TYPES_WIN32_FILE_H

#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include "lib/types/file.h"

#define lib_win32_create_file_a CreateFileA
#define lib_win32_open_osfhandle _open_osfhandle
#define lib_win32_fdopen _fdopen
#define lib_win32_close _close
#define lib_win32_fseeki64 _fseeki64
#define lib_win32_ftelli64 _ftelli64
#define LIB_WIN32_FILE_ATTRIBUTE_NORMAL FILE_ATTRIBUTE_NORMAL
#define LIB_WIN32_FILE_SHARE_READ FILE_SHARE_READ
#define LIB_WIN32_FILE_SHARE_WRITE FILE_SHARE_WRITE
#define LIB_WIN32_GENERIC_READ GENERIC_READ
#define LIB_WIN32_GENERIC_WRITE GENERIC_WRITE
#define LIB_WIN32_OPEN_EXISTING OPEN_EXISTING
#define LIB_WIN32_INVALID_HANDLE_VALUE INVALID_HANDLE_VALUE
#define LIB_WIN32_O_RDWR _O_RDWR
#define LIB_WIN32_O_BINARY _O_BINARY
#define LIB_WIN32_O_RDONLY _O_RDONLY

#endif
