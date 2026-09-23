#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/product/ini_interface.h"
#include "lib/storage/file_interface.h"

#define VM_APP_INI_MAX_BYTES (64u * 1024u)

static C_CHAR *vm_app_ini_trim(C_CHAR *text)
{
    C_CHAR *end;
    while (*text != '\0' && STD_ISSPACE(*text)) ++text;
    end = text + lib_text_length(text);
    while (end != text && STD_ISSPACE(end[-1])) --end;
    *end = '\0';
    return text;
}

static C_INT vm_app_ini_copy(C_CHAR *destination, lib_size capacity,
    const C_CHAR *source)
{
    lib_size length;
    if (destination == LIB_NULL || source == LIB_NULL) return 0;
    length = lib_text_length(source);
    if (length >= capacity) return 0;
    lib_memory_copy(destination, source, length + 1u);
    return 1;
}

static C_INT vm_app_ini_absolute(const C_CHAR *path)
{
    return path != LIB_NULL && (path[0] == '/' || path[0] == '\\' ||
        (path[0] != '\0' && path[1] == ':'));
}

static C_CHAR *vm_app_ini_last_separator(C_CHAR *path)
{
    C_CHAR *last = LIB_NULL;
    if (path == LIB_NULL) return LIB_NULL;
    while (*path != '\0') {
        if (*path == '/' || *path == '\\') last = path;
        ++path;
    }
    return last;
}

static C_CHAR *vm_app_ini_find(C_CHAR *text, C_CHAR needle)
{
    if (text == LIB_NULL) return LIB_NULL;
    while (*text != '\0') {
        if (*text == needle) return text;
        ++text;
    }
    return LIB_NULL;
}

static C_INT vm_app_ini_path(C_CHAR *destination, lib_size capacity,
    const C_CHAR *directory, const C_CHAR *value)
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

static C_INT vm_app_ini_mode(const C_CHAR *value, lib_storage_medium_mode *out_mode)
{
    if (value == LIB_NULL || out_mode == LIB_NULL) return 0;
    if (!lib_c_strcmp(value, "direct")) *out_mode = LIB_STORAGE_MEDIUM_DIRECT;
    else if (!lib_c_strcmp(value, "readonly")) *out_mode = LIB_STORAGE_MEDIUM_READONLY;
    else if (!lib_c_strcmp(value, "overlay")) *out_mode = LIB_STORAGE_MEDIUM_OVERLAY;
    else return 0;
    return 1;
}

static C_INT vm_app_ini_memory(const C_CHAR *value, lib_size *out_bytes)
{
    lib_u64 kib = 0u;
    const C_CHAR *cursor = value;
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

static C_INT vm_app_ini_medium(vm_session_request *request, C_INT floppy,
    lib_size slot, const C_CHAR *directory, C_CHAR *value)
{
    C_CHAR *separator;
    const C_CHAR *path;
    const C_CHAR *mode;
    C_CHAR (*paths)[VM_SESSION_REQUEST_PATH_MAX];
    lib_storage_medium_mode *modes;
    lib_size *count;

    if (request == LIB_NULL || value == LIB_NULL || slot >= VM_SESSION_REQUEST_MEDIA_SLOT_COUNT)
        return 0;
    separator = LIB_NULL;
    {
        C_CHAR *scan = value;
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

type_status vm_app_ini_parse(const C_CHAR *directory, const C_CHAR *name,
    C_CHAR *document, vm_session_request *out_request)
{
    vm_session_request request = {0};
    C_CHAR *line;
    C_CHAR *cursor;
    C_CHAR section[16] = {0};
    C_INT memory_seen = 0;
    C_INT display_seen = 0;
    C_INT console_control_seen = 0;

    if (out_request == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_request = (vm_session_request) {0};
    if (directory == LIB_NULL || name == LIB_NULL || document == LIB_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (!vm_app_ini_copy(request.file_name, sizeof(request.file_name), name))
        return TYPE_STATUS_INVALID_ARGUMENT;
    (C_VOID)vm_app_ini_copy(request.display, sizeof(request.display), "console");
    request.floppy_mode[0u] = LIB_STORAGE_MEDIUM_OVERLAY;
    request.floppy_mode[1u] = LIB_STORAGE_MEDIUM_OVERLAY;
    request.fixed_disk_mode[0u] = LIB_STORAGE_MEDIUM_OVERLAY;
    request.fixed_disk_mode[1u] = LIB_STORAGE_MEDIUM_OVERLAY;
    cursor = document;
    while (cursor != LIB_NULL && *cursor != '\0') {
        C_CHAR *equals;
        const C_CHAR *key;
        C_CHAR *value;
        line = cursor;
        cursor = vm_app_ini_find(cursor, '\n');
        if (cursor != LIB_NULL) *cursor++ = '\0';
        line = vm_app_ini_trim(line);
        if (*line == '\0' || *line == ';' || *line == '#') continue;
        if (line[0] == '[') {
            C_CHAR *close = vm_app_ini_find(line, ']');
            if (close == LIB_NULL || close[1] != '\0') return TYPE_STATUS_INVALID_ARGUMENT;
            *close = '\0';
            if (!vm_app_ini_copy(section, sizeof(section), line + 1u))
                return TYPE_STATUS_INVALID_ARGUMENT;
            continue;
        }
        equals = vm_app_ini_find(line, '=');
        if (equals == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
        *equals = '\0';
        key = vm_app_ini_trim(line);
        value = vm_app_ini_trim(equals + 1u);
        if (!lib_c_strcmp(section, "machine") && !lib_c_strcmp(key, "memory_kib")) {
            if (memory_seen || !vm_app_ini_memory(value, &request.memory_bytes))
                return TYPE_STATUS_INVALID_ARGUMENT;
            memory_seen = 1;
        } else if (!lib_c_strcmp(section, "presentation") && !lib_c_strcmp(key, "display")) {
            if (display_seen || (lib_c_strcmp(value, "console") && lib_c_strcmp(value, "window")) ||
                !vm_app_ini_copy(request.display, sizeof(request.display), value))
                return TYPE_STATUS_INVALID_ARGUMENT;
            display_seen = 1;
        } else if (!lib_c_strcmp(section, "presentation") && !lib_c_strcmp(key, "console_control")) {
            if (console_control_seen) return TYPE_STATUS_INVALID_ARGUMENT;
            if (!lib_c_strcmp(value, "true")) request.console_control = 1;
            else if (!lib_c_strcmp(value, "false")) request.console_control = 0;
            else return TYPE_STATUS_INVALID_ARGUMENT;
            console_control_seen = 1;
        } else if (!lib_c_strcmp(section, "media") && !lib_c_strcmp(key, "floppy0")) {
            if (!vm_app_ini_medium(&request, 1, 0u, directory, value)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else if (!lib_c_strcmp(section, "media") && !lib_c_strcmp(key, "floppy1")) {
            if (!vm_app_ini_medium(&request, 1, 1u, directory, value)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else if (!lib_c_strcmp(section, "media") && !lib_c_strcmp(key, "fixed_disk0")) {
            if (!vm_app_ini_medium(&request, 0, 0u, directory, value)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else if (!lib_c_strcmp(section, "media") && !lib_c_strcmp(key, "fixed_disk1")) {
            if (!vm_app_ini_medium(&request, 0, 1u, directory, value)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (section[0] == '\0') return TYPE_STATUS_INVALID_ARGUMENT;
    *out_request = request;
    return TYPE_STATUS_OK;
}

type_status vm_app_ini_load(const C_CHAR *path, vm_session_request *out_request)
{
    C_VOID *bytes = LIB_NULL;
    lib_size byte_count = 0u;
    C_CHAR *document;
    C_CHAR directory[VM_SESSION_REQUEST_PATH_MAX];
    C_CHAR *slash;
    type_status status;

    lib_status load_status;

    if (out_request == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_request = (vm_session_request) {0};
    if (path == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    load_status = lib_storage_file_read_owned(path, VM_APP_INI_MAX_BYTES, &bytes,
        &byte_count);
    if (load_status == LIB_STATUS_NO_MEMORY) return TYPE_STATUS_NO_MEMORY;
    if (load_status != LIB_STATUS_OK) return TYPE_STATUS_FAULT;
    document = lib_allocate(byte_count + 1u);
    if (document == LIB_NULL) {
        lib_release(bytes);
        return TYPE_STATUS_NO_MEMORY;
    }
    lib_memory_copy(document, bytes, byte_count);
    document[byte_count] = '\0';
    lib_release(bytes);
    if (!vm_app_ini_copy(directory, sizeof(directory), path)) { lib_release(document); return TYPE_STATUS_INVALID_ARGUMENT; }
    slash = vm_app_ini_last_separator(directory);
    if (slash == LIB_NULL) (C_VOID)vm_app_ini_copy(directory, sizeof(directory), ".");
    else if (slash == directory) slash[1u] = '\0';
    else *slash = '\0';
    status = vm_app_ini_parse(directory, path, document, out_request);
    lib_release(document);
    return status;
}
