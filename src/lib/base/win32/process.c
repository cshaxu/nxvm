#include "lib/base/process.h"
#include "lib/types/win32/process.h"
#include "lib/types/win32/text.h"

static lib_status base_process_directory(char *path)
{
    char *separator = path + lib_text_length(path);
    char *root_end = path + 1;
    while (separator != path && separator[-1] != '/' && separator[-1] != '\\') --separator;
    if (separator == path) return LIB_STATUS_IO_ERROR;
    if (path[1] == ':') root_end = path + 3;
    else if ((path[0] == '\\' && path[1] == '\\') ||
        (path[0] == '/' && path[1] == '/')) {
        lib_u32 parts = 0u;
        lib_bool extended_unc = lib_text_compare_n(path, "\\\\?\\", 4u) == 0 &&
            (path[4] == 'U' || path[4] == 'u') && (path[5] == 'N' || path[5] == 'n') &&
            (path[6] == 'C' || path[6] == 'c') && path[7] == '\\';
        root_end = path + (extended_unc ? 8 : 2);
        while (*root_end != '\0' && parts < 2u) {
            if (*root_end == '/' || *root_end == '\\') ++parts;
            ++root_end;
        }
    }
    if (separator > root_end) --separator;
    *separator = '\0';
    return LIB_STATUS_OK;
}

lib_status base_process_platform_executable_directory(char *output, lib_size capacity)
{
    lib_win32_dword length;
    lib_win32_wchar *path;
    lib_i32 bytes;
    lib_status status;

    if (output == LIB_NULL || capacity > LIB_INT32_MAX) return LIB_STATUS_INVALID_ARGUMENT;
    path = lib_allocate(capacity * sizeof(*path));
    if (path == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    length = lib_win32_get_module_file_name_w(LIB_NULL, path, (lib_win32_dword)capacity);
    if (length == 0u) status = LIB_STATUS_IO_ERROR;
    else if (length >= capacity) status = LIB_STATUS_LIMIT_EXCEEDED;
    else {
        bytes = lib_win32_wide_char_to_multi_byte(LIB_WIN32_CP_UTF8,
            LIB_WIN32_WC_ERR_INVALID_CHARS, path, -1, LIB_NULL, 0, LIB_NULL, LIB_NULL);
        if (bytes == 0) status = LIB_STATUS_UNSUPPORTED;
        else if ((lib_size)bytes > capacity) status = LIB_STATUS_LIMIT_EXCEEDED;
        else if (lib_win32_wide_char_to_multi_byte(LIB_WIN32_CP_UTF8,
            LIB_WIN32_WC_ERR_INVALID_CHARS, path, -1, output, bytes,
            LIB_NULL, LIB_NULL) != bytes) status = LIB_STATUS_IO_ERROR;
        else status = base_process_directory(output);
    }
    lib_release(path);
    return status;
}
