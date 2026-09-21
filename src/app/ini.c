#include "type.h"

#include "app/ini_interface.h"

static C_CHAR *vm_app_ini_trim(C_CHAR *text)
{
    C_CHAR *end;
    while (*text != '\0' && STD_ISSPACE(*text)) ++text;
    end = text + STD_STRLEN(text);
    while (end != text && STD_ISSPACE(end[-1])) --end;
    *end = '\0';
    return text;
}

static C_INT vm_app_ini_copy(C_CHAR *destination, STD_SIZE_T capacity,
    const C_CHAR *source)
{
    STD_SIZE_T length;
    if (destination == STD_NULL || source == STD_NULL) return 0;
    length = STD_STRLEN(source);
    if (length >= capacity) return 0;
    STD_MEMCPY(destination, source, length + 1u);
    return 1;
}

static C_INT vm_app_ini_absolute(const C_CHAR *path)
{
    return path != STD_NULL && (path[0] == '/' || path[0] == '\\' ||
        (path[0] != '\0' && path[1] == ':'));
}

static C_CHAR *vm_app_ini_last_separator(C_CHAR *path)
{
    C_CHAR *last = STD_NULL;
    if (path == STD_NULL) return STD_NULL;
    while (*path != '\0') {
        if (*path == '/' || *path == '\\') last = path;
        ++path;
    }
    return last;
}

static C_CHAR *vm_app_ini_find(C_CHAR *text, C_CHAR needle)
{
    if (text == STD_NULL) return STD_NULL;
    while (*text != '\0') {
        if (*text == needle) return text;
        ++text;
    }
    return STD_NULL;
}

static C_INT vm_app_ini_path(C_CHAR *destination, STD_SIZE_T capacity,
    const C_CHAR *directory, const C_CHAR *value)
{
    STD_SIZE_T directory_length;
    STD_SIZE_T value_length;
    if (destination == STD_NULL || directory == STD_NULL || value == STD_NULL ||
        value[0] == '\0') return 0;
    if (vm_app_ini_absolute(value)) return vm_app_ini_copy(destination, capacity, value);
    directory_length = STD_STRLEN(directory);
    value_length = STD_STRLEN(value);
    if (directory_length + 1u + value_length >= capacity) return 0;
    STD_MEMCPY(destination, directory, directory_length);
    if (directory_length != 0u && directory[directory_length - 1u] != '/' &&
        directory[directory_length - 1u] != '\\') destination[directory_length++] = '/';
    STD_MEMCPY(destination + directory_length, value, value_length + 1u);
    return 1;
}

static C_INT vm_app_ini_mode(const C_CHAR *value, lib_storage_medium_mode *out_mode)
{
    if (value == STD_NULL || out_mode == STD_NULL) return 0;
    if (!STD_STRCMP(value, "direct")) *out_mode = LIB_STORAGE_MEDIUM_DIRECT;
    else if (!STD_STRCMP(value, "readonly")) *out_mode = LIB_STORAGE_MEDIUM_READONLY;
    else if (!STD_STRCMP(value, "overlay")) *out_mode = LIB_STORAGE_MEDIUM_OVERLAY;
    else return 0;
    return 1;
}

static C_INT vm_app_ini_memory(const C_CHAR *value, STD_SIZE_T *out_bytes)
{
    type_unsigned_64 kib = 0u;
    const C_CHAR *cursor = value;
    if (value == STD_NULL || out_bytes == STD_NULL || *value == '\0') return 0;
    while (*cursor != '\0') {
        if (*cursor < '0' || *cursor > '9' || kib > ((type_unsigned_64)-1 - 9u) / 10u)
            return 0;
        kib = kib * 10u + (type_unsigned_64)(*cursor++ - '0');
    }
    if (kib > (type_unsigned_64)(STD_SIZE_T)-1 / 1024u) return 0;
    *out_bytes = (STD_SIZE_T)kib * 1024u;
    return 1;
}

static C_INT vm_app_ini_medium(vm_session_request *request, C_INT floppy,
    STD_SIZE_T slot, const C_CHAR *directory, C_CHAR *value)
{
    C_CHAR *separator;
    C_CHAR *path;
    C_CHAR *mode;
    C_CHAR (*paths)[VM_SESSION_REQUEST_PATH_MAX];
    lib_storage_medium_mode *modes;
    STD_SIZE_T *count;

    if (request == STD_NULL || value == STD_NULL || slot >= VM_SESSION_REQUEST_MEDIA_SLOT_COUNT)
        return 0;
    separator = STD_NULL;
    {
        C_CHAR *scan = value;
        while (*scan != '\0') { if (*scan == '|') separator = scan; ++scan; }
    }
    if (separator == STD_NULL) return 0;
    *separator = '\0';
    path = vm_app_ini_trim(value);
    mode = vm_app_ini_trim(separator + 1u);
    paths = floppy ? request->floppy : request->fixed_disk;
    modes = floppy ? request->floppy_mode : request->fixed_disk_mode;
    count = floppy ? &request->floppy_count : &request->fixed_disk_count;
    if (slot != *count || paths[slot][0] != '\0' || !vm_app_ini_path(paths[slot],
            VM_SESSION_REQUEST_PATH_MAX, directory, path) || !vm_app_ini_mode(mode,
            &modes[slot])) return 0;
    if (*count <= slot) *count = slot + 1u;
    return 1;
}

type_status vm_app_ini_parse(const C_CHAR *directory, const C_CHAR *name,
    C_CHAR *document, vm_session_request *out_request)
{
    C_CHAR *line;
    C_CHAR *cursor;
    C_CHAR section[16] = {0};
    C_INT memory_seen = 0;
    C_INT display_seen = 0;
    C_INT console_control_seen = 0;

    if (directory == STD_NULL || name == STD_NULL || document == STD_NULL ||
        out_request == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_request = (vm_session_request) {0};
    if (!vm_app_ini_copy(out_request->file_name, sizeof(out_request->file_name), name))
        return TYPE_STATUS_INVALID_ARGUMENT;
    (void)vm_app_ini_copy(out_request->display, sizeof(out_request->display), "console");
    out_request->floppy_mode[0u] = LIB_STORAGE_MEDIUM_OVERLAY;
    out_request->floppy_mode[1u] = LIB_STORAGE_MEDIUM_OVERLAY;
    out_request->fixed_disk_mode[0u] = LIB_STORAGE_MEDIUM_OVERLAY;
    out_request->fixed_disk_mode[1u] = LIB_STORAGE_MEDIUM_OVERLAY;
    cursor = document;
    while (cursor != STD_NULL && *cursor != '\0') {
        C_CHAR *equals;
        C_CHAR *key;
        C_CHAR *value;
        line = cursor;
        cursor = vm_app_ini_find(cursor, '\n');
        if (cursor != STD_NULL) *cursor++ = '\0';
        line = vm_app_ini_trim(line);
        if (*line == '\0' || *line == ';' || *line == '#') continue;
        if (line[0] == '[') {
            C_CHAR *close = vm_app_ini_find(line, ']');
            if (close == STD_NULL || close[1] != '\0') return TYPE_STATUS_INVALID_ARGUMENT;
            *close = '\0';
            if (!vm_app_ini_copy(section, sizeof(section), line + 1u))
                return TYPE_STATUS_INVALID_ARGUMENT;
            continue;
        }
        equals = vm_app_ini_find(line, '=');
        if (equals == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
        *equals = '\0';
        key = vm_app_ini_trim(line);
        value = vm_app_ini_trim(equals + 1u);
        if (!STD_STRCMP(section, "machine") && !STD_STRCMP(key, "memory_kib")) {
            if (memory_seen || !vm_app_ini_memory(value, &out_request->memory_bytes))
                return TYPE_STATUS_INVALID_ARGUMENT;
            memory_seen = 1;
        } else if (!STD_STRCMP(section, "presentation") && !STD_STRCMP(key, "display")) {
            if (display_seen || (STD_STRCMP(value, "console") && STD_STRCMP(value, "window")) ||
                !vm_app_ini_copy(out_request->display, sizeof(out_request->display), value))
                return TYPE_STATUS_INVALID_ARGUMENT;
            display_seen = 1;
        } else if (!STD_STRCMP(section, "presentation") && !STD_STRCMP(key, "console_control")) {
            if (console_control_seen) return TYPE_STATUS_INVALID_ARGUMENT;
            if (!STD_STRCMP(value, "true")) out_request->console_control = 1;
            else if (!STD_STRCMP(value, "false")) out_request->console_control = 0;
            else return TYPE_STATUS_INVALID_ARGUMENT;
            console_control_seen = 1;
        } else if (!STD_STRCMP(section, "media") && !STD_STRCMP(key, "floppy0")) {
            if (!vm_app_ini_medium(out_request, 1, 0u, directory, value)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else if (!STD_STRCMP(section, "media") && !STD_STRCMP(key, "floppy1")) {
            if (!vm_app_ini_medium(out_request, 1, 1u, directory, value)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else if (!STD_STRCMP(section, "media") && !STD_STRCMP(key, "fixed_disk0")) {
            if (!vm_app_ini_medium(out_request, 0, 0u, directory, value)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else if (!STD_STRCMP(section, "media") && !STD_STRCMP(key, "fixed_disk1")) {
            if (!vm_app_ini_medium(out_request, 0, 1u, directory, value)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return section[0] == '\0' ? TYPE_STATUS_INVALID_ARGUMENT : TYPE_STATUS_OK;
}

type_status vm_app_ini_load(const C_CHAR *path, vm_session_request *out_request)
{
    STD_FILE *file = STD_NULL;
    C_LONG bytes;
    C_CHAR *document;
    C_CHAR directory[VM_SESSION_REQUEST_PATH_MAX];
    C_CHAR *slash;
    type_status status;

    if (path == STD_NULL || out_request == STD_NULL || (file = STD_FOPEN(path, "rb")) == STD_NULL ||
        STD_FSEEK(file, 0, SEEK_END) != 0 || (bytes = STD_FTELL(file)) < 0 ||
        STD_FSEEK(file, 0, SEEK_SET) != 0 || (document = STD_MALLOC((STD_SIZE_T)bytes + 1u)) == STD_NULL) {
        if (file != STD_NULL) STD_FCLOSE(file);
        return TYPE_STATUS_FAULT;
    }
    if (STD_FREAD(document, 1u, (STD_SIZE_T)bytes, file) != (STD_SIZE_T)bytes) {
        STD_FREE(document); STD_FCLOSE(file); return TYPE_STATUS_FAULT;
    }
    document[bytes] = '\0';
    STD_FCLOSE(file);
    if (!vm_app_ini_copy(directory, sizeof(directory), path)) { STD_FREE(document); return TYPE_STATUS_INVALID_ARGUMENT; }
    slash = vm_app_ini_last_separator(directory);
    if (slash == STD_NULL) (void)vm_app_ini_copy(directory, sizeof(directory), ".");
    else *slash = '\0';
    status = vm_app_ini_parse(directory, path, document, out_request);
    STD_FREE(document);
    return status;
}
