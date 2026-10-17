#include "lib/types/test.h"
#include "lib/base/process_interface.h"

static const char *fixture_path;
static lib_bool reject_path;

/* Exercise the actual POSIX leaf even on a Windows build. */
#define LIB_TYPES_LINUX_PROCESS_H
typedef lib_iptr lib_linux_ssize_t;
#define LIB_LINUX_SSIZE_MAX LIB_INT32_MAX
static lib_linux_ssize_t process_readlink(const char *name, char *output, lib_size capacity)
{
    lib_size length = lib_text_length(fixture_path);
    lib_test_assert(lib_text_compare(name, "/proc/self/exe") == 0);
    if (reject_path) return -1;
    if (length > capacity) length = capacity;
    lib_memory_copy(output, fixture_path, length);
    return (lib_linux_ssize_t)length;
}
#define lib_linux_readlink process_readlink
#define base_process_platform_executable_directory process_linux_directory
#include "lib/base/linux/process.c"
#define base_process_executable_directory process_linux_public
#include "lib/base/process.c"
#undef base_process_executable_directory
#undef base_process_platform_executable_directory

#ifdef _WIN32
#include "lib/types/win32/process.h"
#include "lib/types/win32/text.h"
_Static_assert(sizeof(lib_win32_wchar)==2u, "Windows path allocation uses two-byte units");
static lib_bool malformed_native;
static lib_win32_dword process_module_path(lib_win32_hinstance instance,
    lib_win32_wchar *output, lib_win32_dword capacity)
{
    lib_i32 count;
    (void)instance;
    if (reject_path) return 0u;
    if (malformed_native) {
        lib_test_assert(capacity >= 2u);
        output[0] = 0xd800u; output[1] = 0u;
        return 1u;
    }
    count = lib_win32_multi_byte_to_wide_char(LIB_WIN32_CP_UTF8,
        LIB_WIN32_MB_ERR_INVALID_CHARS, fixture_path, -1, LIB_NULL, 0);
    lib_test_assert(count > 0);
    if ((lib_win32_dword)count > capacity) return capacity;
    lib_test_assert(lib_win32_multi_byte_to_wide_char(LIB_WIN32_CP_UTF8,
        LIB_WIN32_MB_ERR_INVALID_CHARS, fixture_path, -1, output, count) == count);
    return (lib_win32_dword)count - 1u;
}
#undef lib_win32_get_module_file_name_w
#define lib_win32_get_module_file_name_w process_module_path
#define base_process_platform_executable_directory process_windows_directory
#include "lib/base/win32/process.c"
#define base_process_executable_directory process_windows_public
#include "lib/base/process.c"
#undef base_process_executable_directory
#undef base_process_platform_executable_directory
#endif

static void check_path(lib_status (*directory)(char *, lib_size),
    const char *path, const char *expected)
{
    char result[1024] = "unchanged";
    fixture_path = path;
    reject_path = LIB_FALSE;
    lib_test_assert(directory(result, lib_text_length(path) + 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_text_compare(result, expected) == 0);
    lib_memory_copy(result, "unchanged", sizeof("unchanged"));
    lib_test_assert(directory(result, lib_text_length(path)) == LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(lib_text_compare(result, "unchanged") == 0);
    reject_path = LIB_TRUE;
    lib_test_assert(directory(result, sizeof(result)) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_text_compare(result, "unchanged") == 0);
    reject_path = LIB_FALSE;
}

int main(void)
{
    char directory[4096];
    char unchanged[] = "unchanged";
    lib_test_assert(base_process_executable_directory(directory, sizeof(directory)) == LIB_STATUS_OK);
    lib_test_assert(directory[0] != '\0');
    lib_test_assert(base_process_executable_directory(unchanged, 1u) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_text_compare(unchanged, "unchanged") == 0);
    check_path(process_linux_public, "/opt/demo", "/opt");
    check_path(process_linux_public, "/demo", "/");
    check_path(process_linux_public, "/opt/a\\b/demo", "/opt/a\\b");
    check_path(process_linux_public, "/opt/a\\b", "/opt");
    check_path(process_linux_public, "/opt/\xe6\xb5\x8b/\xf0\x9f\x8e\xb5", "/opt/\xe6\xb5\x8b");
    fixture_path = "/opt/\xff/demo";
    lib_test_assert(process_linux_public(unchanged, sizeof(unchanged)) == LIB_STATUS_LIMIT_EXCEEDED);
    lib_memory_copy(directory, "unchanged", sizeof("unchanged"));
    lib_test_assert(process_linux_public(directory, sizeof(directory)) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(lib_text_compare(directory, "unchanged") == 0);
#ifdef _WIN32
    static const struct { const char *path; const char *directory; } cases[] = {
        {"C:\\apps\\demo.exe", "C:\\apps"}, {"C:/apps/demo.exe", "C:/apps"},
        {"C:\\demo.exe", "C:\\"}, {"C:/demo.exe", "C:/"},
        {"\\\\server\\share\\demo.exe", "\\\\server\\share\\"},
        {"\\\\server\\share\\apps\\demo.exe", "\\\\server\\share\\apps"},
        {"//server/share/demo", "//server/share/"},
        {"\\\\?\\C:\\demo.exe", "\\\\?\\C:\\"},
        {"\\\\?\\UNC\\server\\share\\demo.exe", "\\\\?\\UNC\\server\\share\\"},
        {"\\\\?\\UNC\\server\\share\\apps\\demo.exe", "\\\\?\\UNC\\server\\share\\apps"},
        {"\\\\?\\uNc\\server\\share\\demo.exe", "\\\\?\\uNc\\server\\share\\"},
        {"C:\\\xe6\xb5\x8b\\\xf0\x9f\x8e\xb5.exe", "C:\\\xe6\xb5\x8b"}};
    for (lib_size i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i)
        check_path(process_windows_public, cases[i].path, cases[i].directory);
    malformed_native = LIB_TRUE;
    lib_test_assert(process_windows_public(unchanged, sizeof(unchanged)) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(lib_text_compare(unchanged, "unchanged") == 0);
#endif
    return 0;
}
