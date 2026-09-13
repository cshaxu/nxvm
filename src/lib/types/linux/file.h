#ifndef LIB_TYPES_LINUX_FILE_H
#define LIB_TYPES_LINUX_FILE_H

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "lib/types/file.h"

typedef off_t lib_linux_off_t;
#define lib_linux_fcntl fcntl
#define lib_linux_fileno fileno
#define lib_linux_fseeko fseeko
#define lib_linux_ftello ftello
#define LIB_LINUX_F_WRLCK F_WRLCK
#define LIB_LINUX_F_RDLCK F_RDLCK
#define LIB_LINUX_F_SETLK F_SETLK
typedef struct flock lib_linux_file_lock;

#endif
