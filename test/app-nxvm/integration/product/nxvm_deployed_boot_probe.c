/* Bounded observation of the deployed EXE and its unchanged adjacent INI.
 * Captures are diagnostic evidence, never an automatic boot-success verdict. */
#include "lib/types/types_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef struct boot_window {
    DWORD process;
    HWND window;
} boot_window;

static BOOL CALLBACK boot_find_window(HWND window, LPARAM opaque)
{
    boot_window *found = (boot_window *)opaque;
    DWORD process = 0;
    char title[128];

    GetWindowThreadProcessId(window, &process);
    GetWindowTextA(window, title, sizeof(title));
    if (process == found->process &&
        lib_text_compare_n(title, "NXVM (", 6) == 0) {
        found->window = window;
        return FALSE;
    }
    return TRUE;
}

static lib_bool boot_capture_window(HWND window, const char *path)
{
    RECT size;
    BITMAPINFO info = {0};
    BITMAPFILEHEADER header = {0};
    HDC source = GetDC(window);
    HDC memory = CreateCompatibleDC(source);
    void *pixels = LIB_NULL;
    HBITMAP bitmap = LIB_NULL;
    HGDIOBJ previous = LIB_NULL;
    FILE *file = LIB_NULL;
    lib_bool result = LIB_FALSE;

    ShowWindow(window, SW_SHOWNOACTIVATE);
    RedrawWindow(window, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    if (!source || !memory || !GetClientRect(window, &size)) goto done;
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = size.right;
    info.bmiHeader.biHeight = -size.bottom;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biSizeImage = (DWORD)size.right * (DWORD)size.bottom * 4u;
    bitmap = CreateDIBSection(source, &info, DIB_RGB_COLORS, &pixels, NULL, 0);
    if (!bitmap) goto done;
    previous = SelectObject(memory, bitmap);
    if (!BitBlt(memory, 0, 0, size.right, size.bottom, source, 0, 0, SRCCOPY)) goto done;
    header.bfType = 0x4d42;
    header.bfOffBits = sizeof(header) + sizeof(info.bmiHeader);
    header.bfSize = header.bfOffBits + info.bmiHeader.biSizeImage;
    file = fopen(path, "wb");
    if (!file) goto done;
    result = fwrite(&header, sizeof(header), 1, file) == 1 &&
        fwrite(&info.bmiHeader, sizeof(info.bmiHeader), 1, file) == 1 &&
        fwrite(pixels, info.bmiHeader.biSizeImage, 1, file) == 1;
done:
    if (file && fclose(file)) result = LIB_FALSE;
    if (previous) SelectObject(memory, previous);
    if (bitmap) DeleteObject(bitmap);
    if (memory) DeleteDC(memory);
    if (source) ReleaseDC(window, source);
    return result;
}

static lib_bool boot_read_console(HANDLE output, char *text, DWORD capacity)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD origin = {0};
    DWORD count = 0;

    if (!GetConsoleScreenBufferInfo(output, &info)) return LIB_FALSE;
    origin.Y = info.srWindow.Top;
    if (!ReadConsoleOutputCharacterA(output, text, capacity - 1,
            origin, &count)) return LIB_FALSE;
    text[count] = 0;
    return LIB_TRUE;
}

static lib_bool boot_send_text(HANDLE input, const char *command, DWORD return_release_ms)
{
    INPUT_RECORD record = {0};
    DWORD written;

    record.EventType = KEY_EVENT;
    record.Event.KeyEvent.wRepeatCount = 1;
    while (*command) {
        record.Event.KeyEvent.uChar.AsciiChar = *command;
        record.Event.KeyEvent.wVirtualKeyCode = *command == '\r' ? VK_RETURN : 0;
        record.Event.KeyEvent.wVirtualScanCode = *command == '\r' ? 0x1cu : 0u;
        record.Event.KeyEvent.bKeyDown = TRUE;
        if (!WriteConsoleInputA(input, &record, 1, &written) || written != 1)
            return LIB_FALSE;
        if (*command == '\r' && return_release_ms != 0u) Sleep(return_release_ms);
        record.Event.KeyEvent.bKeyDown = FALSE;
        if (!WriteConsoleInputA(input, &record, 1, &written) || written != 1)
            return LIB_FALSE;
        ++command;
    }
    return LIB_TRUE;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    STARTUPINFOA startup = {0};
    PROCESS_INFORMATION process = {0};
    HANDLE input = INVALID_HANDLE_VALUE, output = INVALID_HANDLE_VALUE;
    char text[8192] = {0};
    char previous[8192] = {0};
    DWORD begin, code = STILL_ACTIVE;
    lib_bool started = LIB_FALSE;
    lib_bool information_requested = LIB_FALSE;
    FILE *log;
    lib_i32 result = 1;
    boot_window window = {0};
    char *duration_end;
    unsigned long duration;
    unsigned long return_release_ms = 0u;

    if (argc != 6 && argc != 7) {
        fprintf(stderr, "usage: probe EXE DIRECTORY SECONDS LOG BMP [START_RETURN_RELEASE_MS]\n");
        return 2;
    }
    duration = strtoul(argv[3], &duration_end, 10);
    if (*duration_end != '\0' || duration == 0 || duration > 300) return 2;
    if (argc == 7) {
        return_release_ms = strtoul(argv[6], &duration_end, 10);
        if (*duration_end != '\0' || return_release_ms > 1000u) return 2;
    }
    log = fopen(argv[4], "wb");
    if (!log) return 2;
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    if (!CreateProcessA(argv[1], NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
            NULL, argv[2], &startup, &process)) goto done;
    window.process = process.dwProcessId;
    FreeConsole();
    begin = GetTickCount();
    while (!AttachConsole(process.dwProcessId) && GetTickCount() - begin < 5000)
        Sleep(20);
    input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (input == INVALID_HANDLE_VALUE || output == INVALID_HANDLE_VALUE) goto done;
    begin = GetTickCount();
    do {
        GetExitCodeProcess(process.hProcess, &code);
        if (code != STILL_ACTIVE) break;
        /* STARTF_USESHOWWINDOW hides the first native Window as well as the
         * Console. Let an existing guest Window paint normally, without
         * moving keyboard focus to the diagnostic run. */
        window.window = NULL;
        EnumWindows(boot_find_window, (LPARAM)&window);
        if (window.window && !IsWindowVisible(window.window))
            ShowWindow(window.window, SW_SHOWNOACTIVATE);
        /* A raw presenter activates another screen buffer. Reopen CONOUT$
         * instead of continuing to observe the inactive monitor buffer. */
        CloseHandle(output);
        output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (boot_read_console(output, text, sizeof(text))) {
            if (lib_text_compare(text, previous)) {
                fprintf(log, "\n[%lu ms console]\n%s\n",
                    (unsigned long)(GetTickCount() - begin), text);
                lib_memory_copy(previous, text, sizeof(previous));
                fflush(log);
            }
            if (!information_requested && strstr(text, "Console>")) {
                information_requested = boot_send_text(input, "info\r", 0u);
            } else if (!started && information_requested && strstr(text, "Machine:")) {
                started = boot_send_text(input, "start\r", (DWORD)return_release_ms);
                fprintf(log, "START_SENT=%d\n", started);
            }
        }
        Sleep(100);
    } while (GetTickCount() - begin < (DWORD)duration * 1000u);
    window.window = NULL;
    EnumWindows(boot_find_window, (LPARAM)&window);
    if (window.window) fprintf(log, "WINDOW_CAPTURE=%d\n",
        boot_capture_window(window.window, argv[5]));
    if (started && code == STILL_ACTIVE) {
        INPUT_RECORD pause[2] = {0};
        DWORD written;
        if (window.window) PostMessageA(window.window, WM_CLOSE, 0, 0);
        else {
            pause[0].EventType = KEY_EVENT;
            pause[0].Event.KeyEvent.bKeyDown = TRUE;
            pause[0].Event.KeyEvent.wRepeatCount = 1;
            pause[0].Event.KeyEvent.wVirtualKeyCode = 'P';
            pause[0].Event.KeyEvent.wVirtualScanCode = 0x19;
            pause[0].Event.KeyEvent.dwControlKeyState = LEFT_CTRL_PRESSED | LEFT_ALT_PRESSED;
            pause[1] = pause[0];
            pause[1].Event.KeyEvent.bKeyDown = FALSE;
            (void)WriteConsoleInputA(input, pause, 2, &written);
        }
        Sleep(1000);
        CloseHandle(output);
        output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (boot_read_console(output, text, sizeof(text)))
            fprintf(log, "\n[after pause request]\n%s\n", text);
        (void)boot_send_text(input, "debug\r", 0u);
        Sleep(300);
        (void)boot_send_text(input, "r\r", 0u);
        Sleep(300);
        if (boot_read_console(output, text, sizeof(text)))
            fprintf(log, "\n[diagnostic registers]\n%s\n", text);
        (void)boot_send_text(input, "d b800:a0\r", 0u);
        Sleep(300);
        if (boot_read_console(output, text, sizeof(text)))
            fprintf(log, "\n[diagnostic text memory]\n%s\n", text);
    }
    result = 0;
done:
    fprintf(log, "OBSERVATION_ONLY start=%d exit=%lu last_native_error=%lu\n", started,
        (unsigned long)code, (unsigned long)GetLastError());
    fclose(log);
    if (process.hProcess) {
        if (WaitForSingleObject(process.hProcess, 0) != WAIT_OBJECT_0) {
            TerminateProcess(process.hProcess, 1);
            WaitForSingleObject(process.hProcess, 5000);
        }
        CloseHandle(process.hProcess);
        CloseHandle(process.hThread);
    }
    if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
    if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
    FreeConsole();
    return result;
}
