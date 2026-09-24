#include "lib/types/test.h"
#include "lib/types/file.h"
#ifndef SHARED_TEST_CLEANUP_H
#define SHARED_TEST_CLEANUP_H

/* Windows may retain a just-closed image briefly while an external filter
 * observes it.  Tests call this only after their machine/runtime has been
 * destroyed: a persistent file handle still fails after the bounded wait. */

#ifdef _WIN32
#include "lib/types/win32/test.h"
static lib_i32 shared_test_remove_file(const char *path)
{
    lib_win32_dword deadline = lib_win32_get_tick_count() + 1000u;
    do {
        if (lib_c_remove(path) == 0) return 1;
        lib_win32_sleep(10u);
    } while ((lib_win32_long)(lib_win32_get_tick_count() - deadline) < 0);
    return 0;
}
#else
static lib_i32 shared_test_remove_file(const char *path)
{
    return lib_c_remove(path) == 0;
}
#endif

#endif
