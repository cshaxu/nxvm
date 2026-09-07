#include "lib/ux/presenter.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <curses.h>
#endif

void ux_discard_console_input(void)
{
#ifdef _WIN32
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    if (input != INVALID_HANDLE_VALUE && input != NULL &&
        GetConsoleMode(input, &mode))
        (void)FlushConsoleInputBuffer(input);
#else
    (void)flushinp();
#endif
}
