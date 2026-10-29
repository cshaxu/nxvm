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

static lib_bool run_variant(const char *executable, const char *directory,
    const char *variant, const char *const *commands, const char *const *markers,
    lib_size count)
{
    const char *selected_commands[32];
    const char *selected_markers[32];
    if (count + 1u > 32u) return LIB_FALSE;
    selected_commands[0] = lib_text_compare(variant, "tft") == 0 ?
        "vdisplay=tft\r" : "vdisplay=dstn\r";
    selected_markers[0] = lib_text_compare(variant, "tft") == 0 ?
        "Display: tft (next start)." : "Display: dstn (next start).";
    for (lib_size index = 0u; index < count; ++index) {
        selected_commands[index + 1u] = commands[index];
        selected_markers[index + 1u] = markers[index];
    }
    return emulator_console_process_run(executable, directory, selected_commands,
        selected_markers, count + 1u);
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
    static const char *const set_rtc_commands[] = {
        "reset\r", "debug\r", "o 70 09\r", "o 71 26\r", "q\r", "exit\r"};
    static const char *const set_rtc_markers[] = {
        "Machine reset and paused.", "-", "-", "-", "> ", ""};
    static const char *const read_rtc_commands[] = {
        "reset\r", "debug\r", "o 70 09\r", "i 71\r", "q\r", "exit\r"};
    static const char *const read_rtc_markers[] = {
        "Machine reset and paused.", "-", "-", "00000026", "> ", ""};
    static const char *const switch_commands[] = {
        "vdisplay=tft\r", "reset\r", "stop\r", "vdisplay=dstn\r",
        "reset\r", "stop\r", "vdisplay=tft\r", "reset\r", "exit\r"};
    static const char *const switch_markers[] = {
        "Display: tft (next start).", "Machine reset and paused.", "Machine stopped.",
        "Display: dstn (next start).", "Machine reset and paused.", "Machine stopped.",
        "Display: tft (next start).", "Machine reset and paused.", ""};
    static const char *const isolation_commands[] = {
        "reset\r", "debug\r", "o 70 3b\r", "o 71 5a\r", "q\r", "stop\r",
        "vdisplay=tft\r", "reset\r", "debug\r", "o 70 3b\r", "o 71 a5\r", "q\r", "stop\r",
        "vdisplay=dstn\r", "reset\r", "debug\r", "o 70 3b\r", "i 71\r", "q\r", "exit\r"};
    static const char *const isolation_markers[] = {
        "Machine reset and paused.", "-", "-", "-", "> ", "Machine stopped.",
        "Display: tft (next start).", "Machine reset and paused.", "-", "-", "-", "> ",
        "Machine stopped.", "Display: dstn (next start).", "Machine reset and paused.",
        "-", "-", "0000005A", "> ", ""};
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
    if (!run_variant(executable, directory, argv[2], commands, markers, 2u) ||
        lib_storage_file_read_owned(cmos_path, 128u, &bytes, &count) != LIB_STATUS_OK ||
        count != 128u) goto done;
    /* Legacy files have only configuration RAM; their clock bytes are zero. */
    lib_memory_set(bytes, 0, 0x0eu);
    ((lib_u8 *)bytes)[0x3bu] = 0x5au;
    if (write_bytes(cmos_path, bytes, count) != LIB_STATUS_OK) goto done;
    lib_release(bytes);
    bytes = LIB_NULL;
    if (!run_variant(executable, directory, argv[2], commands, markers, 2u) ||
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
    if (!run_variant(executable, directory, argv[2], commands, markers, 2u) ||
        lib_storage_file_read_owned(cmos_path, 128u, &bytes, &count) != LIB_STATUS_OK ||
        count != 128u || ((lib_u8 *)bytes)[0x0du] != 0x80u ||
        ((lib_u8 *)bytes)[0x09u] != 0x24u ||
        ((lib_u8 *)bytes)[0x3bu] != 0x5au) goto done;
    if (!run_variant(executable, directory, argv[2], lifecycle_commands,
            lifecycle_markers, sizeof(lifecycle_commands) / sizeof(lifecycle_commands[0])))
        goto done;
    if (!run_variant(executable, directory, argv[2], set_rtc_commands,
            set_rtc_markers, sizeof(set_rtc_commands) / sizeof(set_rtc_commands[0])) ||
        !run_variant(executable, directory, argv[2], read_rtc_commands,
            read_rtc_markers, sizeof(read_rtc_commands) / sizeof(read_rtc_commands[0])))
        goto done;
    if (!emulator_console_process_run(executable, directory, switch_commands,
            switch_markers, sizeof(switch_commands) / sizeof(switch_commands[0])))
        goto done;
    if (!emulator_console_process_run(executable, directory, isolation_commands,
            isolation_markers, sizeof(isolation_commands) / sizeof(isolation_commands[0])))
        goto done;
    lib_release(bytes);
    bytes = LIB_NULL;
    for (lib_size index = 0u; index < 2u; ++index) {
        char isolated_path[MAX_PATH];
        if (lib_c_snprintf(isolated_path, sizeof(isolated_path), "%s/mypc110-%s.cmos",
                directory, index == 0u ? "dstn" : "tft") >= (lib_i32)sizeof(isolated_path) ||
            lib_storage_file_read_owned(isolated_path, 128u, &bytes, &count) != LIB_STATUS_OK ||
            count != 128u || ((lib_u8 *)bytes)[0x3bu] != (index == 0u ? 0x5au : 0xa5u))
            goto done;
        lib_release(bytes);
        bytes = LIB_NULL;
    }
    result = 0;
done:
    lib_release(bytes);
    if (cmos_path[0] != '\0') (void)DeleteFileA(cmos_path);
    if (directory[0] != '\0') {
        char other[MAX_PATH];
        if (lib_c_snprintf(other, sizeof(other), "%s/mypc110-%s.cmos", directory,
                lib_text_compare(argv[2], "tft") == 0 ? "dstn" : "tft") < (lib_i32)sizeof(other))
            (void)DeleteFileA(other);
    }
    if (ini_path[0] != '\0') (void)DeleteFileA(ini_path);
    if (executable[0] != '\0') (void)DeleteFileA(executable);
    (void)RemoveDirectoryA(directory);
    if (result == 0) lib_c_printf("PC110:CMOS-PRODUCT:OK\n");
    return result;
}
