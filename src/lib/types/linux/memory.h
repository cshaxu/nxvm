#ifndef LIB_TYPES_LINUX_MEMORY_H
#define LIB_TYPES_LINUX_MEMORY_H

#include <sys/mman.h>
#include <unistd.h>

#define lib_linux_mmap mmap
#define lib_linux_mprotect mprotect
#define lib_linux_munmap munmap
#define lib_linux_sysconf sysconf
#define LIB_LINUX_MAP_ANONYMOUS MAP_ANONYMOUS
#define LIB_LINUX_MAP_FAILED MAP_FAILED
#define LIB_LINUX_MAP_PRIVATE MAP_PRIVATE
#define LIB_LINUX_PROT_NONE PROT_NONE
#define LIB_LINUX_PROT_READ PROT_READ
#define LIB_LINUX_PROT_WRITE PROT_WRITE
#define LIB_LINUX_SC_PAGESIZE _SC_PAGESIZE

#endif
