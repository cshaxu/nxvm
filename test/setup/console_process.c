#include "lib/types/types_interface.h"
#include <stdio.h>

#include <windows.h>

#include "test/setup/console_process.h"

#define EMULATOR_CONSOLE_WAIT_MILLISECONDS 5000u

static lib_i32 emulator_console_send_key(HANDLE input, WORD virtual_key,
    CHAR character, lib_i32 pressed)
{
    INPUT_RECORD record = {0};
    DWORD written = 0u;

    record.EventType = KEY_EVENT;
    record.Event.KeyEvent.bKeyDown = pressed ? TRUE : FALSE;
    record.Event.KeyEvent.wRepeatCount = 1u;
    record.Event.KeyEvent.wVirtualKeyCode = virtual_key;
    record.Event.KeyEvent.uChar.AsciiChar = pressed ? character : 0;
    return WriteConsoleInputA(input, &record, 1u, &written) && written == 1u;
}

static lib_i32 emulator_console_send_text(HANDLE input, const char *text)
{
    lib_size index;

    if (input == INVALID_HANDLE_VALUE || text == LIB_NULL) return 0;
    for (index = 0u; text[index] != '\0'; ++index) {
        CHAR character = text[index];
        WORD virtual_key = character == '\r' ? VK_RETURN : 0u;

        if (!emulator_console_send_key(input, virtual_key, character, LIB_TRUE) ||
            !emulator_console_send_key(input, virtual_key, 0, LIB_FALSE)) return 0;
    }
    return 1;
}

static lib_i32 emulator_console_has_text(HANDLE output, const char *text)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    char buffer[4096];
    COORD origin = {0};
    DWORD read = 0u;
    DWORD count;
    DWORD offset = 0u;
    DWORD limit;
    lib_size index;
    lib_size length;

    if (output == INVALID_HANDLE_VALUE || text == LIB_NULL ||
        !GetConsoleScreenBufferInfo(output, &info)) return 0;
    length = lib_text_length(text);
    if (length == 0u) return 1;
    if (length >= sizeof(buffer)) return 0;
    /* Monitor output may already have scrolled outside the visible viewport. */
    limit = (DWORD)info.dwSize.X * ((DWORD)info.dwCursorPosition.Y + 1u);
    while (offset < limit) {
        origin.X = (SHORT)(offset % (DWORD)info.dwSize.X);
        origin.Y = (SHORT)(offset / (DWORD)info.dwSize.X);
        count = limit - offset;
        if (count >= sizeof(buffer)) count = sizeof(buffer) - 1u;
        if (!ReadConsoleOutputCharacterA(output, buffer, count, origin, &read)) return 0;
        for (index = 0u; index + length <= read; ++index) {
            if (lib_memory_compare(buffer + index, text, length) == 0) return 1;
        }
        if (read < length) return 0;
        offset += read - (DWORD)length + 1u;
    }
    return 0;
}

static lib_i32 emulator_console_wait_for_text(HANDLE output, const char *text,
    DWORD timeout)
{
    DWORD deadline = GetTickCount() + timeout;

    do {
        if (emulator_console_has_text(output, text)) return 1;
        Sleep(20u);
    } while ((LONG)(GetTickCount() - deadline) < 0);
    return 0;
}

lib_i32 emulator_console_process_run(const char *executable,
    const char *session_directory,
    const char *const *commands, const char *const *markers,
    lib_size command_count)
{
    STARTUPINFOA startup = {0};
    PROCESS_INFORMATION process = {0};
    HANDLE input = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    const DWORD standard_ids[] = {STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
    HANDLE standard_handles[3];
    lib_size index;
    lib_i32 result = 0;
    const char *stage = "validation";

    startup.cb = sizeof(startup);
    if (executable == LIB_NULL || session_directory == LIB_NULL ||
        (command_count != 0u && (commands == LIB_NULL || markers == LIB_NULL))) return 0;
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    stage = "child creation";
    if (!CreateProcessA(executable, LIB_NULL, LIB_NULL, LIB_NULL, FALSE,
            CREATE_NEW_CONSOLE, LIB_NULL, session_directory, &startup, &process))
        return 0;
    for (index = 0u; index < 3u; ++index) {
        DWORD mode;
        standard_handles[index] = GetStdHandle(standard_ids[index]);
        if (standard_handles[index] == INVALID_HANDLE_VALUE ||
            GetConsoleMode(standard_handles[index], &mode))
            standard_handles[index] = LIB_NULL;
    }
    (void)FreeConsole();
    stage = "console attachment";
    for (index = 0u; index < EMULATOR_CONSOLE_WAIT_MILLISECONDS / 20u; ++index) {
        if (AttachConsole(process.dwProcessId)) break;
        Sleep(20u);
    }
    input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, LIB_NULL, OPEN_EXISTING, 0u, LIB_NULL);
    output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, LIB_NULL, OPEN_EXISTING, 0u, LIB_NULL);
    if (input == INVALID_HANDLE_VALUE || output == INVALID_HANDLE_VALUE) {
        stage = "console handles";
        goto done;
    }
    if (!emulator_console_wait_for_text(output, "> ", EMULATOR_CONSOLE_WAIT_MILLISECONDS)) {
        stage = "monitor prompt";
        goto done;
    }
    for (index = 0u; index < command_count; ++index) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        DWORD cleared = 0u;
        DWORD count;

        /* This private test Console must not satisfy a command with old output.
         * Leave its cursor, geometry and input stream untouched. */
        stage = "old output removal";
        if (!GetConsoleScreenBufferInfo(output, &info)) goto done;
        count = (DWORD)info.dwSize.X * ((DWORD)info.dwCursorPosition.Y + 1u);
        if (!FillConsoleOutputCharacterA(output, ' ', count, (COORD){0}, &cleared) ||
            cleared != count) goto done;
        if (!emulator_console_send_text(input, commands[index])) {
            stage = "command input";
            goto done;
        }
        if (markers[index][0] != '\0' && !emulator_console_wait_for_text(output,
                markers[index], EMULATOR_CONSOLE_WAIT_MILLISECONDS)) {
            fprintf(stderr, "PC Console expected result %zu: %s.\n",
                index, markers[index]);
            stage = "command result";
            goto done;
        }
    }
    result = 1;
done:
    if (result && command_count != 0u &&
        WaitForSingleObject(process.hProcess, EMULATOR_CONSOLE_WAIT_MILLISECONDS) !=
            WAIT_OBJECT_0) result = 0;
    if (result && command_count != 0u) {
        DWORD exit_code;
        result = GetExitCodeProcess(process.hProcess, &exit_code) && exit_code == 0u;
    }
    if (!result) {
        DWORD exit_code = STILL_ACTIVE;

        (void)GetExitCodeProcess(process.hProcess, &exit_code);
        fprintf(stderr, "PC Console integration failed at %s "
            "(error=%lu exit=%lu).\n", stage, (unsigned long)GetLastError(),
            (unsigned long)exit_code);
        (void)TerminateProcess(process.hProcess, 1u);
        (void)WaitForSingleObject(process.hProcess, EMULATOR_CONSOLE_WAIT_MILLISECONDS);
    }
    if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
    if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
    (void)FreeConsole();
    /* The borrowed Console invalidated its standard handles; preserve only
     * the caller's independent file/pipe handles for subsequent CRT cleanup. */
    for (index = 0u; index < 3u; ++index)
        if (!SetStdHandle(standard_ids[index], standard_handles[index])) result = 0;
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return result;
}
