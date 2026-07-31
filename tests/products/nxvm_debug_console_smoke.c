#include <windows.h>

#include <stdio.h>
#include <string.h>

#define OUTPUT_CAPACITY 32768u

static int write_all(HANDLE handle, const char *text)
{
    DWORD written;
    size_t length = strlen(text);

    return WriteFile(handle, text, (DWORD)length, &written, NULL) &&
           written == (DWORD)length;
}

static void collect_available(HANDLE output, char *text, size_t *length)
{
    DWORD available;
    DWORD read;

    while (*length + 1u < OUTPUT_CAPACITY &&
           PeekNamedPipe(output, NULL, 0u, NULL, &available, NULL) && available != 0u) {
        DWORD capacity = (DWORD)(OUTPUT_CAPACITY - *length - 1u);
        if (available < capacity) capacity = available;
        if (!ReadFile(output, text + *length, capacity, &read, NULL) || read == 0u) break;
        *length += (size_t)read;
        text[*length] = '\0';
    }
}

int main(int argc, char **argv)
{
    SECURITY_ATTRIBUTES attributes = { sizeof(attributes), NULL, TRUE };
    STARTUPINFOA startup = { 0 };
    PROCESS_INFORMATION process = { 0 };
    HANDLE input_read = NULL;
    HANDLE input_write = NULL;
    HANDLE output_read = NULL;
    HANDLE output_write = NULL;
    char command[2048];
    char output[OUTPUT_CAPACITY] = { 0 };
    size_t output_length = 0u;
    DWORD exit_code;
    unsigned int waited;

    startup.cb = sizeof(startup);
    if (argc != 3 ||
        !CreatePipe(&input_read, &input_write, &attributes, 0u) ||
        !SetHandleInformation(input_write, HANDLE_FLAG_INHERIT, 0u) ||
        !CreatePipe(&output_read, &output_write, &attributes, 0u) ||
        !SetHandleInformation(output_read, HANDLE_FLAG_INHERIT, 0u) ||
        snprintf(command, sizeof(command), "\"%s\"", argv[1]) < 0) return 1;

    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input_read;
    startup.hStdOutput = output_write;
    startup.hStdError = output_write;
    if (!CreateProcessA(NULL, command, NULL, NULL, TRUE, CREATE_NO_WINDOW,
                        NULL, NULL, &startup, &process)) return 1;
    CloseHandle(input_read);
    input_read = NULL;
    CloseHandle(output_write);
    output_write = NULL;

    if (!write_all(input_write, "device fdd insert ") ||
        !write_all(input_write, argv[2]) ||
        !write_all(input_write, "\r\nreset\r\ndebug\r\n")) goto fail;
    for (waited = 0u; waited < 5000u; waited += 25u) {
        collect_available(output_read, output, &output_length);
        if (strstr(output, "Entering NXVM hardware debugger.") != NULL) break;
        Sleep(25u);
    }
    if (strstr(output, "Entering NXVM hardware debugger.") == NULL ||
        !write_all(input_write, "q\r\ninfo\r\nexit\r\n")) goto fail;
    CloseHandle(input_write);
    input_write = NULL;
    if (WaitForSingleObject(process.hProcess, 10000u) != WAIT_OBJECT_0 ||
        !GetExitCodeProcess(process.hProcess, &exit_code) || exit_code != 0u) goto fail;
    collect_available(output_read, output, &output_length);
    CloseHandle(output_read);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (strstr(output, "lifecycle: debugger-paused") == NULL) return 1;
    puts("M5:T7:S2:NXVM-DEBUG-CONSOLE:OK");
    return 0;

fail:
    if (input_read != NULL) CloseHandle(input_read);
    if (input_write != NULL) CloseHandle(input_write);
    if (output_read != NULL) CloseHandle(output_read);
    if (output_write != NULL) CloseHandle(output_write);
    if (process.hThread != NULL) CloseHandle(process.hThread);
    if (process.hProcess != NULL) {
        TerminateProcess(process.hProcess, 1u);
        CloseHandle(process.hProcess);
    }
    return 1;
}
