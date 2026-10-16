#include "lib/types/test.h"
#include "lib/base/process_interface.h"

static const char *fixture_path;
static lib_status fixture_status;

static lib_status process_test_path(char *output, lib_size capacity)
{
    lib_size length = lib_text_length(fixture_path);
    if (fixture_status != LIB_STATUS_OK) return fixture_status;
    if (length >= capacity) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(output, fixture_path, length + 1u);
    return LIB_STATUS_OK;
}

#define base_process_platform_executable_path process_test_path
#define base_process_executable_directory process_test_directory
#include "lib/base/process.c"
#undef base_process_executable_directory
#undef base_process_platform_executable_path

int main(void)
{
    char directory[1024];
    char unchanged[] = "unchanged";
    static const struct { const char *path; const char *directory; } cases[] = {
        {"C:\\apps\\demo.exe", "C:\\apps"},
        {"C:/apps/demo.exe", "C:/apps"},
        {"/opt/demo", "/opt"}, {"/demo", "/"},
        {"C:\\demo.exe", "C:\\"}, {"C:/demo.exe", "C:/"},
        {"\\\\server\\share\\demo.exe", "\\\\server\\share\\"},
        {"\\\\server\\share\\apps\\demo.exe", "\\\\server\\share\\apps"},
        {"//server/share/demo", "//server/share/"},
        {"\\\\?\\C:\\demo.exe", "\\\\?\\C:\\"},
        {"\\\\?\\UNC\\server\\share\\demo.exe", "\\\\?\\UNC\\server\\share\\"},
        {"\\\\?\\UNC\\server\\share\\apps\\demo.exe", "\\\\?\\UNC\\server\\share\\apps"}};
    lib_size index;

    if (base_process_executable_directory(directory, sizeof(directory)) != LIB_STATUS_OK ||
        directory[0] == '\0') return 1;
    if (base_process_executable_directory(unchanged, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_text_compare(unchanged, "unchanged") != 0) return 1;
    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        fixture_path = cases[index].path;
        fixture_status = LIB_STATUS_OK;
        lib_test_assert(process_test_directory(directory, sizeof(directory)) == LIB_STATUS_OK);
        lib_test_assert(lib_text_compare(directory, cases[index].directory) == 0);
        lib_test_assert(process_test_directory(directory, lib_text_length(fixture_path) + 1u) == LIB_STATUS_OK);
        lib_test_assert(lib_text_compare(directory, cases[index].directory) == 0);
        lib_memory_copy(directory, "unchanged", sizeof("unchanged"));
        lib_test_assert(process_test_directory(directory, lib_text_length(fixture_path)) == LIB_STATUS_IO_ERROR);
        lib_test_assert(lib_text_compare(directory, "unchanged") == 0);
    }
    fixture_path = "C:\\apps\\demo.exe";
    lib_test_assert(process_test_directory(unchanged, sizeof(unchanged)) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_text_compare(unchanged, "unchanged") == 0);
    fixture_path = "demo";
    lib_test_assert(process_test_directory(unchanged, sizeof(unchanged)) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_text_compare(unchanged, "unchanged") == 0);
    fixture_status = LIB_STATUS_UNSUPPORTED;
    lib_test_assert(process_test_directory(unchanged, sizeof(unchanged)) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(lib_text_compare(unchanged, "unchanged") == 0);
    return 0;
}
