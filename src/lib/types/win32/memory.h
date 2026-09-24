#ifndef LIB_TYPES_WIN32_MEMORY_H
#define LIB_TYPES_WIN32_MEMORY_H

#include "lib/types/win32/scalar.h"

#define lib_win32_get_system_info GetSystemInfo
#define lib_win32_virtual_alloc VirtualAlloc
#define lib_win32_virtual_free VirtualFree
#define lib_win32_virtual_protect VirtualProtect
#define LIB_WIN32_MEM_COMMIT MEM_COMMIT
#define LIB_WIN32_MEM_RELEASE MEM_RELEASE
#define LIB_WIN32_MEM_RESERVE MEM_RESERVE
#define LIB_WIN32_PAGE_NOACCESS PAGE_NOACCESS
#define LIB_WIN32_PAGE_READWRITE PAGE_READWRITE

#endif
