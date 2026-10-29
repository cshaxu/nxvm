#include "lib/storage/file_interface.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "test/setup/console_process.h"
#include <windows.h>

static lib_status write_bytes(const char *path, const void *bytes, lib_size count)
{
    lib_storage_file_writer *writer = LIB_NULL;
    lib_status status = lib_storage_file_writer_open(path,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer);
    if (status == LIB_STATUS_OK) status = lib_storage_file_writer_write(writer, bytes, count);
    if (writer != LIB_NULL) {
        lib_status close_status = lib_storage_file_writer_close(writer);
        if (status == LIB_STATUS_OK) status = close_status;
    }
    return status;
}

int main(int argc, char **argv)
{
    static const char ini[] = "[presentation]\ndisplay=window\nconsole_control=1\n[media]\n";
    static const char *const commands[] = {"reset\r", "exit\r"};
    static const char *const markers[] = {"Machine reset and paused.", ""};
    static const char *const lifecycle_commands[] = {
        "start\r", "pause\r", "resume\r", "reset\r", "resume\r", "stop\r", "exit\r"};
    static const char *const lifecycle_markers[] = {
        "Machine started.", "Machine paused.", "Machine resumed.",
        "Machine reset and paused.", "Machine resumed.", "Machine stopped.", ""};
    char directory[MAX_PATH] = {0}, executable[MAX_PATH] = {0};
    char ini_path[MAX_PATH] = {0}, cmos_path[MAX_PATH] = {0};
    void *bytes = LIB_NULL;
    lib_size count = 0u;
    int result = 1;

    if (argc != 4) return 2;
    if (!GetTempFileNameA(argv[3], "pcm", 0u, directory) ||
        !DeleteFileA(directory) || !CreateDirectoryA(directory, LIB_NULL)) return 1;
    if (lib_c_snprintf(executable, sizeof(executable), "%s/program.exe", directory) >=
            (lib_i32)sizeof(executable) ||
        lib_c_snprintf(ini_path, sizeof(ini_path), "%s/mypc110.ini", directory) >=
            (lib_i32)sizeof(ini_path) ||
        lib_c_snprintf(cmos_path, sizeof(cmos_path), "%s/mypc110-%s.cmos", directory, argv[2]) >=
            (lib_i32)sizeof(cmos_path) ||
        !CopyFileA(argv[1], executable, TRUE) ||
        write_bytes(ini_path, ini, sizeof(ini) - 1u) != LIB_STATUS_OK) goto done;
    if (!emulator_console_process_run(executable, directory, commands, markers, 2u) ||
        lib_storage_file_read_owned(cmos_path, 128u, &bytes, &count) != LIB_STATUS_OK ||
        count != 128u) goto done;
    /* Legacy files have only configuration RAM; their clock bytes are zero. */
    lib_memory_set(bytes, 0, 0x0eu);
    ((lib_u8 *)bytes)[0x3bu] = 0x5au;
    if (write_bytes(cmos_path, bytes, count) != LIB_STATUS_OK) goto done;
    lib_release(bytes);
    bytes = LIB_NULL;
    if (!emulator_console_process_run(executable, directory, commands, markers, 2u) ||
        lib_storage_file_read_owned(cmos_path, 128u, &bytes, &count) != LIB_STATUS_OK ||
        count != 128u || ((lib_u8 *)bytes)[0x09u] != 0u ||
        ((lib_u8 *)bytes)[0x0du] != 0x80u || ((lib_u8 *)bytes)[0x3bu] != 0x5au)
        goto done;
    /* Restore a valid RTC image through the real product configuration path. */
    ((lib_u8 *)bytes)[0x09u] = 0x24u;
    ((lib_u8 *)bytes)[0x0bu] = 0x02u;
    ((lib_u8 *)bytes)[0x0du] = 0x80u;
    ((lib_u8 *)bytes)[0x3bu] = 0x5au;
    if (write_bytes(cmos_path, bytes, count) != LIB_STATUS_OK) goto done;
    lib_release(bytes);
    bytes = LIB_NULL;
    if (!emulator_console_process_run(executable, directory, commands, markers, 2u) ||
        lib_storage_file_read_owned(cmos_path, 128u, &bytes, &count) != LIB_STATUS_OK ||
        count != 128u || ((lib_u8 *)bytes)[0x0du] != 0x80u ||
        ((lib_u8 *)bytes)[0x09u] != 0x24u ||
        ((lib_u8 *)bytes)[0x3bu] != 0x5au) goto done;
    if (!emulator_console_process_run(executable, directory, lifecycle_commands,
            lifecycle_markers, sizeof(lifecycle_commands) / sizeof(lifecycle_commands[0])))
        goto done;
    result = 0;
done:
    lib_release(bytes);
    if (cmos_path[0] != '\0') (void)DeleteFileA(cmos_path);
    if (ini_path[0] != '\0') (void)DeleteFileA(ini_path);
    if (executable[0] != '\0') (void)DeleteFileA(executable);
    (void)RemoveDirectoryA(directory);
    if (result == 0) lib_c_printf("PC110:CMOS-PRODUCT:OK\n");
    return result;
}
