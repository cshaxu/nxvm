#include "type.h"

#include <windows.h>

#include "vm/app/catalog.h"
#include "test/integration/support/nxvm_console_process.h"

#define NXVM_CONSOLE_WAIT_MILLISECONDS 5000u

static C_INT nxvm_console_profile_choice(const C_CHAR *directory,
    const C_CHAR *profile_file)
{
    vm_app_session_catalog *catalog = STD_NULL;
    STD_SIZE_T index;
    C_INT choice = 0;

    if (directory == STD_NULL || profile_file == STD_NULL ||
        vm_app_session_catalog_create(directory, &catalog) != TYPE_STATUS_OK)
        return 0;
    for (index = 0u; index < vm_app_session_catalog_count(catalog); ++index) {
        vm_session_request request;

        if (vm_app_session_catalog_get_request(catalog, index, &request) ==
                TYPE_STATUS_OK &&
            !STD_STRCMP(request.file_name, profile_file)) {
            choice = (C_INT)index + 1;
            break;
        }
    }
    vm_app_session_catalog_destroy(catalog);
    return choice;
}

static C_INT nxvm_console_send_key(HANDLE input, WORD virtual_key,
    CHAR character, C_INT pressed)
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

static C_INT nxvm_console_send_text(HANDLE input, const C_CHAR *text)
{
    STD_SIZE_T index;

    if (input == INVALID_HANDLE_VALUE || text == STD_NULL) return 0;
    for (index = 0u; text[index] != '\0'; ++index) {
        CHAR character = text[index];
        WORD virtual_key = character == '\r' ? VK_RETURN : 0u;

        if (!nxvm_console_send_key(input, virtual_key, character, TYPE_TRUE) ||
            !nxvm_console_send_key(input, virtual_key, 0, TYPE_FALSE)) return 0;
    }
    return 1;
}

static C_INT nxvm_console_has_text(HANDLE output, const C_CHAR *text)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    C_CHAR buffer[4096];
    COORD origin = {0};
    DWORD read = 0u;
    DWORD count;
    STD_SIZE_T index;
    STD_SIZE_T length;

    if (output == INVALID_HANDLE_VALUE || text == STD_NULL ||
        !GetConsoleScreenBufferInfo(output, &info)) return 0;
    origin.X = info.srWindow.Left;
    origin.Y = info.srWindow.Top;
    count = (DWORD)(info.srWindow.Right - info.srWindow.Left + 1) *
        (DWORD)(info.srWindow.Bottom - info.srWindow.Top + 1);
    if (count >= sizeof(buffer)) count = sizeof(buffer) - 1u;
    if (!ReadConsoleOutputCharacterA(output, buffer, count, origin, &read)) return 0;
    buffer[read] = '\0';
    length = STD_STRLEN(text);
    for (index = 0u; index + length <= read; ++index) {
        if (STD_MEMCMP(buffer + index, text, length) == 0) return 1;
    }
    return 0;
}

static C_INT nxvm_console_wait_for_text(HANDLE output, const C_CHAR *text,
    DWORD timeout)
{
    DWORD deadline = GetTickCount() + timeout;

    do {
        if (nxvm_console_has_text(output, text)) return 1;
        Sleep(20u);
    } while ((LONG)(GetTickCount() - deadline) < 0);
    return 0;
}

C_INT nxvm_console_process_run(const C_CHAR *executable,
    const C_CHAR *session_directory, const C_CHAR *profile_file,
    const C_CHAR *const *commands, const C_CHAR *const *markers,
    STD_SIZE_T command_count)
{
    STARTUPINFOA startup = {0};
    PROCESS_INFORMATION process = {0};
    HANDLE input = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    C_CHAR choice_text[16];
    C_INT choice;
    STD_SIZE_T index;
    C_INT result = 0;
    const C_CHAR *stage = "validation";

    startup.cb = sizeof(startup);
    if (executable == STD_NULL || session_directory == STD_NULL ||
        profile_file == STD_NULL || (command_count != 0u &&
        (commands == STD_NULL || markers == STD_NULL)) ||
        (choice = nxvm_console_profile_choice(session_directory, profile_file)) == 0 ||
        STD_SNPRINTF(choice_text, sizeof(choice_text), "%d\r", choice) < 0) return 0;
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    stage = "child creation";
    if (!CreateProcessA(executable, STD_NULL, STD_NULL, STD_NULL, FALSE,
            CREATE_NEW_CONSOLE, STD_NULL, session_directory, &startup, &process))
        return 0;
    (C_VOID)FreeConsole();
    stage = "console attachment";
    for (index = 0u; index < NXVM_CONSOLE_WAIT_MILLISECONDS / 20u; ++index) {
        if (AttachConsole(process.dwProcessId)) break;
        Sleep(20u);
    }
    input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, STD_NULL, OPEN_EXISTING, 0u, STD_NULL);
    output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, STD_NULL, OPEN_EXISTING, 0u, STD_NULL);
    if (input == INVALID_HANDLE_VALUE || output == INVALID_HANDLE_VALUE) {
        stage = "console handles";
        goto done;
    }
    if (!nxvm_console_wait_for_text(output, "Select profile", NXVM_CONSOLE_WAIT_MILLISECONDS)) {
        stage = "profile prompt";
        goto done;
    }
    if (!nxvm_console_send_text(input, choice_text)) {
        stage = "profile input";
        goto done;
    }
    if (!nxvm_console_wait_for_text(output, "Console>", NXVM_CONSOLE_WAIT_MILLISECONDS)) {
        stage = "monitor prompt";
        goto done;
    }
    for (index = 0u; index < command_count; ++index) {
        if (!nxvm_console_send_text(input, commands[index])) {
            stage = "command input";
            goto done;
        }
        if (markers[index][0] != '\0' && !nxvm_console_wait_for_text(output,
                markers[index], NXVM_CONSOLE_WAIT_MILLISECONDS)) {
            stage = "command result";
            goto done;
        }
    }
    result = 1;
done:
    if (result && command_count != 0u &&
        WaitForSingleObject(process.hProcess, NXVM_CONSOLE_WAIT_MILLISECONDS) !=
            WAIT_OBJECT_0) result = 0;
    if (!result) {
        DWORD exit_code = STILL_ACTIVE;

        (C_VOID)GetExitCodeProcess(process.hProcess, &exit_code);
        STD_FPRINTF(STD_STDERR, "NXVM Console integration failed at %s "
            "(error=%lu exit=%lu).\n", stage, (unsigned long)GetLastError(),
            (unsigned long)exit_code);
        (C_VOID)TerminateProcess(process.hProcess, 1u);
        (C_VOID)WaitForSingleObject(process.hProcess, NXVM_CONSOLE_WAIT_MILLISECONDS);
    }
    if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
    if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
    (C_VOID)FreeConsole();
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return result;
}
