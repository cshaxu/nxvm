#include "type.h"

#include "vm/platform/input_flush.h"

#ifdef _WIN32
#include <windows.h>
#endif

C_VOID vm_platform_input_flush_console_input(C_VOID)
{
#ifdef _WIN32
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    if (input != INVALID_HANDLE_VALUE && input != STD_NULL &&
        GetConsoleMode(input, &mode)) {
        FlushConsoleInputBuffer(input);
    }
#endif
}
