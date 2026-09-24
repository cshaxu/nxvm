#include "lib/types/types_interface.h"

#include "app-nxvm/product/ini_interface.h"
#include "lib/storage/file_interface.h"

#define VM_APP_INI_MAX_BYTES (64u * 1024u)

static lib_bool vm_app_ini_space(lib_u8 value)
{ return value == ' ' || value == '\t' || value == '\r' || value == '\f'; }

static lib_bool vm_app_ini_text_equal(const lib_u8 *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        if (*left++ != (lib_u8)*right++) return LIB_FALSE;
    }
    return *left == '\0' && *right == '\0';
}

static lib_u8 *vm_app_ini_trim(lib_u8 *text)
{
    lib_u8 *end;
    while (*text != '\0' && vm_app_ini_space(*text)) ++text;
    end = text + lib_text_length(text);
    while (end != text && vm_app_ini_space(end[-1])) --end;
    *end = '\0';
    return text;
}

static lib_i32 vm_app_ini_copy(lib_u8 *destination, lib_size capacity,
    const lib_u8 *source)
{
    lib_size length;
    if (destination == LIB_NULL || source == LIB_NULL) return 0;
    length = lib_text_length(source);
    if (length >= capacity) return 0;
    lib_memory_copy(destination, source, length + 1u);
    return 1;
}

static lib_i32 vm_app_ini_absolute(const lib_u8 *path)
{
    return path != LIB_NULL && (path[0] == '/' || path[0] == '\\' ||
        (path[0] != '\0' && path[1] == ':'));
}

static lib_u8 *vm_app_ini_last_separator(lib_u8 *path)
{
    lib_u8 *last = LIB_NULL;
    if (path == LIB_NULL) return LIB_NULL;
    while (*path != '\0') {
        if (*path == '/' || *path == '\\') last = path;
        ++path;
    }
    return last;
}

static lib_u8 *vm_app_ini_find(lib_u8 *text, lib_u8 needle)
{
    if (text == LIB_NULL) return LIB_NULL;
    while (*text != '\0') {
        if (*text == needle) return text;
        ++text;
    }
    return LIB_NULL;
}

static lib_i32 vm_app_ini_path(lib_u8 *destination, lib_size capacity,
    const lib_u8 *directory, const lib_u8 *value)
{
    lib_size directory_length;
    lib_size value_length;
    if (destination == LIB_NULL || directory == LIB_NULL || value == LIB_NULL ||
        value[0] == '\0') return 0;
    if (vm_app_ini_absolute(value)) return vm_app_ini_copy(destination, capacity, value);
    directory_length = lib_text_length(directory);
    value_length = lib_text_length(value);
    if (directory_length + 1u + value_length >= capacity) return 0;
    lib_memory_copy(destination, directory, directory_length);
    if (directory_length != 0u && directory[directory_length - 1u] != '/' &&
        directory[directory_length - 1u] != '\\') destination[directory_length++] = '/';
    lib_memory_copy(destination + directory_length, value, value_length + 1u);
    return 1;
}

static lib_i32 vm_app_ini_mode(const lib_u8 *value, lib_storage_medium_mode *out_mode)
{
    if (value == LIB_NULL || out_mode == LIB_NULL) return 0;
    if (vm_app_ini_text_equal(value, "direct")) *out_mode = LIB_STORAGE_MEDIUM_DIRECT;
    else if (vm_app_ini_text_equal(value, "readonly")) *out_mode = LIB_STORAGE_MEDIUM_READONLY;
    else if (vm_app_ini_text_equal(value, "overlay")) *out_mode = LIB_STORAGE_MEDIUM_OVERLAY;
    else return 0;
    return 1;
}

static lib_i32 vm_app_ini_memory(const lib_u8 *value, lib_size *out_bytes)
{
    lib_u64 kib = 0u;
    const lib_u8 *cursor = value;
    if (value == LIB_NULL || out_bytes == LIB_NULL || *value == '\0') return 0;
    while (*cursor != '\0') {
        if (*cursor < '0' || *cursor > '9' || kib > ((lib_u64)-1 - 9u) / 10u)
            return 0;
        kib = kib * 10u + (lib_u64)(*cursor++ - '0');
    }
    if (kib > (lib_u64)(lib_size)-1 / 1024u) return 0;
    *out_bytes = (lib_size)kib * 1024u;
    return 1;
}

static lib_i32 vm_app_ini_medium(vm_session_request *request, lib_i32 floppy,
    lib_size slot, const lib_u8 *directory, lib_u8 *value)
{
    lib_u8 *separator;
    const lib_u8 *path;
    const lib_u8 *mode;
    lib_u8 (*paths)[VM_SESSION_REQUEST_PATH_MAX];
    lib_storage_medium_mode *modes;
    lib_size *count;

    if (request == LIB_NULL || value == LIB_NULL || slot >= VM_SESSION_REQUEST_MEDIA_SLOT_COUNT)
        return 0;
    separator = LIB_NULL;
    {
        lib_u8 *scan = value;
        while (*scan != '\0') { if (*scan == '|') separator = scan; ++scan; }
    }
    if (separator == LIB_NULL) return 0;
    *separator = '\0';
    path = vm_app_ini_trim(value);
    mode = vm_app_ini_trim(separator + 1u);
    paths = floppy ? request->floppy : request->fixed_disk;
    modes = floppy ? request->floppy_mode : request->fixed_disk_mode;
    count = floppy ? &request->floppy_count : &request->fixed_disk_count;
    if (slot != *count || paths[slot][0] != '\0' || !vm_app_ini_path(paths[slot],
            VM_SESSION_REQUEST_PATH_MAX, directory, path) || !vm_app_ini_mode(mode,
            &modes[slot])) return 0;
    *count = slot + 1u;
    return 1;
}

lib_status vm_app_ini_parse(const lib_u8 *directory, const lib_u8 *name,
    lib_u8 *document, vm_session_request *out_request)
{
    vm_session_request request = {0};
    lib_u8 *line;
    lib_u8 *cursor;
    lib_u8 section[16] = {0};
    lib_i32 memory_seen = 0;
    lib_i32 display_seen = 0;
    lib_i32 console_control_seen = 0;

    if (out_request == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_request = (vm_session_request) {0};
    if (directory == LIB_NULL || name == LIB_NULL || document == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (!vm_app_ini_copy(request.file_name, sizeof(request.file_name), name))
        return LIB_STATUS_INVALID_ARGUMENT;
    (void)vm_app_ini_copy(request.display, sizeof(request.display),
        (const lib_u8 *)"console");
    request.floppy_mode[0u] = LIB_STORAGE_MEDIUM_OVERLAY;
    request.floppy_mode[1u] = LIB_STORAGE_MEDIUM_OVERLAY;
    request.fixed_disk_mode[0u] = LIB_STORAGE_MEDIUM_OVERLAY;
    request.fixed_disk_mode[1u] = LIB_STORAGE_MEDIUM_OVERLAY;
    cursor = document;
    while (cursor != LIB_NULL && *cursor != '\0') {
        lib_u8 *equals;
        const lib_u8 *key;
        lib_u8 *value;
        line = cursor;
        cursor = vm_app_ini_find(cursor, '\n');
        if (cursor != LIB_NULL) *cursor++ = '\0';
        line = vm_app_ini_trim(line);
        if (*line == '\0' || *line == ';' || *line == '#') continue;
        if (line[0] == '[') {
            lib_u8 *close = vm_app_ini_find(line, ']');
            if (close == LIB_NULL || close[1] != '\0') return LIB_STATUS_INVALID_ARGUMENT;
            *close = '\0';
            if (!vm_app_ini_copy(section, sizeof(section), line + 1u))
                return LIB_STATUS_INVALID_ARGUMENT;
            continue;
        }
        equals = vm_app_ini_find(line, '=');
        if (equals == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
        *equals = '\0';
        key = vm_app_ini_trim(line);
        value = vm_app_ini_trim(equals + 1u);
        if (vm_app_ini_text_equal(section, "machine") &&
            vm_app_ini_text_equal(key, "memory_kib")) {
            if (memory_seen || !vm_app_ini_memory(value, &request.memory_bytes))
                return LIB_STATUS_INVALID_ARGUMENT;
            memory_seen = 1;
        } else if (vm_app_ini_text_equal(section, "presentation") &&
            vm_app_ini_text_equal(key, "display")) {
            if (display_seen || (!vm_app_ini_text_equal(value, "console") &&
                !vm_app_ini_text_equal(value, "window")) ||
                !vm_app_ini_copy(request.display, sizeof(request.display), value))
                return LIB_STATUS_INVALID_ARGUMENT;
            display_seen = 1;
        } else if (vm_app_ini_text_equal(section, "presentation") &&
            vm_app_ini_text_equal(key, "console_control")) {
            if (console_control_seen) return LIB_STATUS_INVALID_ARGUMENT;
            if (vm_app_ini_text_equal(value, "true")) request.console_control = 1;
            else if (vm_app_ini_text_equal(value, "false")) request.console_control = 0;
            else return LIB_STATUS_INVALID_ARGUMENT;
            console_control_seen = 1;
        } else if (vm_app_ini_text_equal(section, "media") && vm_app_ini_text_equal(key, "floppy0")) {
            if (!vm_app_ini_medium(&request, 1, 0u, directory, value)) return LIB_STATUS_INVALID_ARGUMENT;
        } else if (vm_app_ini_text_equal(section, "media") && vm_app_ini_text_equal(key, "floppy1")) {
            if (!vm_app_ini_medium(&request, 1, 1u, directory, value)) return LIB_STATUS_INVALID_ARGUMENT;
        } else if (vm_app_ini_text_equal(section, "media") && vm_app_ini_text_equal(key, "fixed_disk0")) {
            if (!vm_app_ini_medium(&request, 0, 0u, directory, value)) return LIB_STATUS_INVALID_ARGUMENT;
        } else if (vm_app_ini_text_equal(section, "media") && vm_app_ini_text_equal(key, "fixed_disk1")) {
            if (!vm_app_ini_medium(&request, 0, 1u, directory, value)) return LIB_STATUS_INVALID_ARGUMENT;
        } else return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (section[0] == '\0') return LIB_STATUS_INVALID_ARGUMENT;
    *out_request = request;
    return LIB_STATUS_OK;
}

lib_status vm_app_ini_load(const lib_u8 *path, vm_session_request *out_request)
{
    void *bytes = LIB_NULL;
    lib_size byte_count = 0u;
    lib_u8 *document;
    lib_u8 directory[VM_SESSION_REQUEST_PATH_MAX];
    lib_u8 *slash;
    lib_status status;

    lib_status load_status;

    if (out_request == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_request = (vm_session_request) {0};
    if (path == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    load_status = lib_storage_file_read_owned((const char *)path, VM_APP_INI_MAX_BYTES, &bytes,
        &byte_count);
    if (load_status == LIB_STATUS_NO_MEMORY) return LIB_STATUS_NO_MEMORY;
    if (load_status != LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
    document = lib_allocate(byte_count + 1u);
    if (document == LIB_NULL) {
        lib_release(bytes);
        return LIB_STATUS_NO_MEMORY;
    }
    lib_memory_copy(document, bytes, byte_count);
    document[byte_count] = '\0';
    lib_release(bytes);
    if (!vm_app_ini_copy(directory, sizeof(directory), path)) { lib_release(document); return LIB_STATUS_INVALID_ARGUMENT; }
    slash = vm_app_ini_last_separator(directory);
    if (slash == LIB_NULL) (void)vm_app_ini_copy(directory, sizeof(directory),
        (const lib_u8 *)".");
    else if (slash == directory) slash[1u] = '\0';
    else *slash = '\0';
    status = vm_app_ini_parse(directory, path, document, out_request);
    lib_release(document);
    return status;
}
