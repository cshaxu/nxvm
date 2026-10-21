#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-nxvm/product/ini_interface.h"
#include <windows.h>

#include "test/emulator/setup/console_process.h"

static lib_bool write_ini(const char *path, const char *text)
{
    FILE *file = lib_c_fopen(path, "wb");
    lib_size length = lib_text_length(text);
    lib_bool written;

    if (file == LIB_NULL) return LIB_FALSE;
    written = lib_c_fwrite(text, 1u, length, file) == length;
    return lib_c_fclose(file) == 0 && written;
}

static lib_bool rejects_missing_ini(const char *executable, const char *directory)
{
    STARTUPINFOA startup = {.cb = sizeof(startup)};
    PROCESS_INFORMATION process = {0};
    DWORD code = STILL_ACTIVE;
    lib_bool result;

    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    if (!CreateProcessA(executable, LIB_NULL, LIB_NULL, LIB_NULL, FALSE,
            CREATE_NEW_CONSOLE, LIB_NULL, directory, &startup, &process))
        return LIB_FALSE;
    result = WaitForSingleObject(process.hProcess, 5000u) == WAIT_OBJECT_0 &&
        GetExitCodeProcess(process.hProcess, &code) && code == 1u;
    if (!result) {
        (void)TerminateProcess(process.hProcess, 1u);
        (void)WaitForSingleObject(process.hProcess, 5000u);
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return result;
}

int main(int argc, char **argv)
{
    static const char *const names[] = {
        "my5160.ini", "my5170.ini", "mydeskpro386.ini", "nxvm.ini"};
    static const char valid[] =
        "[presentation]\ndisplay=window\nconsole_control=1\n[media]\n";
    static const char *const commands[] = {
        "info\r", "speed\r", "speed turbo\r", "speed standard\r", "help\r", "exit\r"};
    static const char *const markers[] = {
        "Device Info", "Speed:", "Speed: turbo", "Speed: standard",
        "Control your virtual machine:", ""};
    char directory[MAX_PATH] = {0}, executable[MAX_PATH] = {0};
    char cwd[MAX_PATH] = {0}, cwd_ini[MAX_PATH] = {0};
    char media[MAX_PATH] = {0};
    char paths[4][MAX_PATH] = {{0}};
    x86_nxvm_product_request request;
    lib_size selected = sizeof(names) / sizeof(names[0]);
    lib_i32 result = 1;

    if (argc != 4) return 2;
    for (lib_size i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (lib_text_compare(argv[2], names[i]) == 0) selected = i;
    if (selected == sizeof(names) / sizeof(names[0])) return 2;
    /* Every mutation is inside a new private directory under the CTest workspace.
     * Only the executable is copied: no owner INI, firmware or media is copied. */
    if (!GetTempFileNameA(argv[3], "ini", 0u, directory)) return 1;
    if (!DeleteFileA(directory) || !CreateDirectoryA(directory, LIB_NULL)) return 1;
    if (lib_c_snprintf(executable, sizeof(executable), "%s/program.exe", directory) >=
            (lib_i32)sizeof(executable) ||
        lib_c_snprintf(cwd, sizeof(cwd), "%s/cwd", directory) >= (lib_i32)sizeof(cwd) ||
        lib_c_snprintf(cwd_ini, sizeof(cwd_ini), "%s/%s", cwd, names[selected]) >=
            (lib_i32)sizeof(cwd_ini) ||
        lib_c_snprintf(media, sizeof(media), "%s/probe.img", directory) >=
            (lib_i32)sizeof(media)) goto done;
    if (!CopyFileA(argv[1], executable, TRUE) || !CreateDirectoryA(cwd, LIB_NULL))
        goto done;
    for (lib_size i = 0u; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (lib_c_snprintf(paths[i], sizeof(paths[i]), "%s/%s", directory, names[i]) >=
                (lib_i32)sizeof(paths[i]) ||
            !write_ini(paths[i], i == selected ? valid : "[invalid]\nkey=value\n"))
            goto done;
    }
    /* The file loader must resolve media against this INI, independently of CWD.
     * Parsing needs no disk image; the real process checks below are media-free. */
    if (!write_ini(paths[selected], "[media]\nfloppy0=probe.img|readonly\n") ||
        x86_nxvm_product_ini_load((const lib_u8 *)paths[selected], &request) != LIB_STATUS_OK ||
        lib_text_compare((const char *)request.floppy[0], media) != 0) goto done;
    if (!write_ini(paths[selected], valid)) goto done;
    /* A correct adjacent INI wins despite wrong product files and invalid CWD INI.
     * Renaming the test EXE also proves the name is not inferred from argv[0]. */
    if (!write_ini(cwd_ini, "[invalid]\nkey=value\n") ||
        !emulator_console_process_run(executable, cwd, commands, markers,
            sizeof(commands) / sizeof(commands[0]))) goto done;
    if (!DeleteFileA(paths[selected]) || !write_ini(cwd_ini, valid)) goto done;
    for (lib_size i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (i != selected && !write_ini(paths[i], valid)) goto done;
    /* Other products, the old NXVM.ini (case-insensitive Windows name), and a
     * correct name in CWD must not replace a missing executable-adjacent file. */
    if (!rejects_missing_ini(executable, cwd)) goto done;
    result = 0;
done:
    for (lib_size i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (paths[i][0] != '\0') (void)DeleteFileA(paths[i]);
    if (cwd_ini[0] != '\0') (void)DeleteFileA(cwd_ini);
    if (cwd[0] != '\0') (void)RemoveDirectoryA(cwd);
    if (executable[0] != '\0') (void)DeleteFileA(executable);
    (void)RemoveDirectoryA(directory);
    if (result == 0) lib_c_printf("PC:PRODUCT-INI-IDENTITY:OK\n");
    return result;
}
