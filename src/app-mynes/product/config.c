#include "product/config.h"

#include "lib/storage/file_interface.h"
#include "lib/types/types_interface.h"

#include <windows.h>

static lib_bool app_config_space(char value)
{ return value == ' ' || value == '\t' || value == '\r'; }

static const char *app_config_trim_left(const char *start, const char *end)
{
    while (start != end && app_config_space(*start)) ++start;
    return start;
}

static const char *app_config_trim_right(const char *start, const char *end)
{
    while (end != start && app_config_space(end[-1])) --end;
    return end;
}

static lib_bool app_config_equals(const char *start, const char *end,
    const char *literal)
{
    while (start != end && *literal != '\0') {
        if (*start++ != *literal++) return LIB_FALSE;
    }
    return start == end && *literal == '\0';
}

static int app_config_copy(char *target, lib_size target_capacity,
    const char *start, const char *end)
{
    lib_size length = (lib_size)(end - start);
    if (length == 0u || length >= target_capacity) return 0;
    lib_memory_copy(target, start, length);
    target[length] = '\0';
    return 1;
}

int app_config_load_text(const char *text, lib_size text_length,
    app_startup_config *out_config)
{
    const char *cursor;
    const char *limit;
    app_startup_config candidate = { 0 };

    if (text == LIB_NULL || out_config == LIB_NULL) return 0;
    cursor = text;
    limit = text + text_length;
    while (cursor != limit) {
        const char *line_end = cursor;
        const char *key_start;
        const char *key_end;
        const char *value_start;
        const char *value_end;
        const char *equals;
        const char *comment;
        const char *next_line;

        while (line_end != limit && *line_end != '\n') ++line_end;
        next_line = line_end;
        /* Match SoftPC's fixed-INI behavior: strip either comment marker
         * before interpreting an assignment, and quietly ignore an ordinary
         * non-assignment record. */
        comment = cursor;
        while (comment != line_end && *comment != ';' && *comment != '#') ++comment;
        line_end = comment;
        key_start = app_config_trim_left(cursor, line_end);
        key_end = app_config_trim_right(key_start, line_end);
        if (key_start != key_end) {
            equals = key_start;
            while (equals != key_end && *equals != '=') ++equals;
            if (equals != key_end) {
                value_start = app_config_trim_left(equals + 1, key_end);
                value_end = app_config_trim_right(value_start, key_end);
                key_end = app_config_trim_right(key_start, equals);
                if (value_end > value_start + 1u && *value_start == '"' &&
                    value_end[-1] == '"') {
                    ++value_start;
                    --value_end;
                }
                if (app_config_equals(key_start, key_end, "rom")) {
                    if (!app_config_copy(candidate.rom_path,
                            sizeof(candidate.rom_path), value_start, value_end)) return 0;
                } else if (app_config_equals(key_start, key_end, "display")) {
                    if (app_config_equals(value_start, value_end, "window"))
                        candidate.text_output = LIB_FALSE;
                    else if (app_config_equals(value_start, value_end, "console"))
                        candidate.text_output = LIB_TRUE;
                    else return 0;
                } else return 0;
            }
        }
        cursor = next_line == limit ? limit : next_line + 1;
    }
    *out_config = candidate;
    return 1;
}

static int app_config_path(char *path)
{
    DWORD length = GetModuleFileNameA(LIB_NULL, path, APP_CONFIG_PATH_CAPACITY);
    char *separator = path;

    if (length == 0u || length >= APP_CONFIG_PATH_CAPACITY) return 0;
    for (; *separator != '\0'; ++separator) { }
    while (separator != path && separator[-1] != '\\' && separator[-1] != '/') --separator;
    if (separator == path || (lib_size)(separator - path) + sizeof("mynes.ini") >
            APP_CONFIG_PATH_CAPACITY) return 0;
    lib_memory_copy(separator, "mynes.ini", sizeof("mynes.ini"));
    return 1;
}

int app_config_parse(int argc, char **argv, app_startup_config *out_config)
{
    char path[APP_CONFIG_PATH_CAPACITY];
    void *bytes = LIB_NULL;
    lib_size byte_count;
    lib_status status;

    if (out_config == LIB_NULL || argc != 1 || argv == LIB_NULL || !app_config_path(path))
        return 0;
    status = lib_storage_file_read_owned(path, 16384u, &bytes, &byte_count);
    if (status != LIB_STATUS_OK) return 0;
    {
        int result = app_config_load_text(bytes, byte_count, out_config);
        lib_release(bytes);
        return result;
    }
}
