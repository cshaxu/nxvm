#ifndef SOFTPC_TEST_CLEANUP_H
#define SOFTPC_TEST_CLEANUP_H

/* Windows may retain a just-closed image briefly while an external filter
 * observes it.  Tests call this only after their machine/runtime has been
 * destroyed: a persistent SoftPC handle still fails after the bounded wait. */
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
static int softpc_test_remove_image(const char *path)
{
    DWORD deadline = GetTickCount() + 1000u;
    do {
        if (remove(path) == 0) return 1;
        Sleep(10u);
    } while ((LONG)(GetTickCount() - deadline) < 0);
    return 0;
}
#else
static int softpc_test_remove_image(const char *path)
{
    return remove(path) == 0;
}
#endif

#endif
