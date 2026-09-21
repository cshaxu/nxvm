#ifndef LIB_TYPES_LINUX_FILE_H
#define LIB_TYPES_LINUX_FILE_H

#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include "lib/types/file.h"

typedef off_t lib_linux_off_t;
#define lib_linux_flock flock
#define lib_linux_fileno fileno
#define lib_linux_fseeko fseeko
#define lib_linux_ftello ftello
#define LIB_LINUX_LOCK_EX LOCK_EX
#define LIB_LINUX_LOCK_SH LOCK_SH
#define LIB_LINUX_LOCK_NB LOCK_NB

#endif
