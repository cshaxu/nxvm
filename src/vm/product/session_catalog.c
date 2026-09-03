#include "type.h"

#include <dirent.h>

#include "vm/product/session_catalog.h"

struct vm_product_session_catalog {
    vm_product_session_request entries[VM_PRODUCT_SESSION_CATALOG_MAX];
    STD_SIZE_T count;
    STD_SIZE_T rejected;
};

#include "core/platform/file.h"
#include "core/product/utils.h"

static C_CHAR *vm_product_session_catalog_trim(C_CHAR *value)
{
    C_CHAR *end;

    while (*value != '\0' && STD_ISSPACE(*value)) ++value;
    end = value + STD_STRLEN(value);
    while (end != value && STD_ISSPACE(end[-1])) --end;
    *end = '\0';
    return value;
}

static C_INT vm_product_session_catalog_copy(C_CHAR *destination,
    STD_SIZE_T capacity, const C_CHAR *source)
{
    STD_SIZE_T length = source == STD_NULL ? 0u : STD_STRLEN(source);

    if (destination == STD_NULL || capacity == 0u || length >= capacity) return 0;
    if (source != STD_NULL) STD_MEMCPY(destination, source, length + 1u);
    else destination[0] = '\0';
    return 1;
}

static C_INT vm_product_session_catalog_path(C_CHAR *destination,
    STD_SIZE_T capacity, const C_CHAR *directory, const C_CHAR *name)
{
    C_INT written;

    if (name == STD_NULL) return 0;
    if (name[0] == '/' || name[0] == '\\' ||
        (STD_ISALPHA(name[0]) && name[1] == ':')) {
        return vm_product_session_catalog_copy(destination, capacity, name);
    }
    written = STD_SNPRINTF(destination, capacity, "%s/%s", directory, name);
    return written >= 0 && (STD_SIZE_T)written < capacity;
}

static C_INT vm_product_session_catalog_parse_value(C_CHAR *line,
    const C_CHAR *key, C_CHAR **out_value)
{
    STD_SIZE_T length = STD_STRLEN(key);

    if (STD_MEMCMP(line, key, length) != 0 || line[length] != ':') return 0;
    *out_value = vm_product_session_catalog_trim(line + length + 1u);
    if (**out_value == '\"') {
        C_CHAR *end = *out_value + STD_STRLEN(*out_value);
        if (end == *out_value + 1 || end[-1] != '\"') return 0;
        *out_value += 1;
        end[-1] = '\0';
    }
    return 1;
}

static C_INT vm_product_session_catalog_parse_document(const C_CHAR *directory,
    const C_CHAR *name, C_CHAR *document, vm_product_session_request *entry)
{
    C_CHAR *cursor;
    C_CHAR *line;
    C_CHAR *value;
    C_INT section = 0;
    C_INT media = 0;
    C_INT firmware = 0;
    C_INT firmware_bios = 0;
    C_INT firmware_video = 0;
    C_INT firmware_cmos = 0;
    C_INT firmware_font = 0;
    C_INT media_section = 0;
    C_INT schema = 0;
    C_INT profile = 0;
    C_INT cpu = 0;
    C_INT fpu = 0;
    C_INT memory = 0;
    C_INT display = 0;
    C_INT floppy_format = 0;
    C_INT floppy = 0;
    C_INT hard_disk = 0;

    if (directory == STD_NULL || name == STD_NULL || document == STD_NULL ||
        entry == STD_NULL) return 0;
    STD_MEMSET(entry, 0, sizeof(*entry));
    cursor = document;
    while (*cursor != '\0') {
        line = cursor;
        while (*cursor != '\0' && *cursor != '\n') ++cursor;
        if (*cursor == '\n') *cursor++ = '\0';
        C_CHAR *text = vm_product_session_catalog_trim(line);
        if (*text == '\0' || *text == '#') continue;
        if (!STD_STRCMP(text, "media:")) {
            if (media_section) break;
            section = 2; media = 0; media_section = 1; continue;
        }
        if (!STD_STRCMP(text, "firmware:")) {
            if (firmware) break;
            section = 3; media = 0; firmware = 1; continue;
        }
        if (section == 3 && vm_product_session_catalog_parse_value(text, "bios", &value)) {
            if (firmware_bios) break;
            firmware_bios = 1;
            if (!STD_STRCMP(value, "[]")) continue;
            if (*value == '\0') continue;
            break;
        }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "schema", &value)) {
            if (schema || STD_STRCMP(value, "nxvm-session")) break;
            schema = 1; continue;
        }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "profile", &value)) {
            if (profile || !vm_product_session_catalog_copy(entry->profile, sizeof(entry->profile), value)) break;
            profile = 1; continue;
        }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "cpu", &value)) {
            if (cpu || !vm_product_session_catalog_copy(entry->cpu, sizeof(entry->cpu), value)) break;
            cpu = 1; continue;
        }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "fpu", &value)) {
            if (fpu || !vm_product_session_catalog_copy(entry->fpu, sizeof(entry->fpu), value)) break;
            fpu = 1; continue;
        }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "memory_kib", &value)) {
            if (memory || core_product_utils_parse_memory_kib(value,
                    &entry->memory_bytes) != TYPE_STATUS_OK) break;
            memory = 1;
            continue;
        }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "display", &value)) {
            if (display || !vm_product_session_catalog_copy(entry->display, sizeof(entry->display), value)) break;
            display = 1; continue;
        }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "floppy_format", &value)) {
            if (floppy_format || !vm_product_session_catalog_copy(entry->floppy_format,
                    sizeof(entry->floppy_format), value)) break;
            floppy_format = 1; continue;
        }
        if (section == 2 && vm_product_session_catalog_parse_value(text, "floppy", &value)) {
            if (floppy) break;
            floppy = 1;
            if (!STD_STRCMP(value, "[]")) continue;
            if (*value == '\0') { media = 1; continue; }
            break;
        }
        if (section == 2 && vm_product_session_catalog_parse_value(text, "fixed_disk", &value)) {
            if (hard_disk) break;
            hard_disk = 1;
            if (!STD_STRCMP(value, "[]")) continue;
            if (*value == '\0') { media = 2; continue; }
            break;
        }
        if (section == 3 && firmware_bios && text[0] == '-' &&
            vm_product_session_catalog_parse_value(vm_product_session_catalog_trim(text + 1u),
                "path", &value)) {
            if (entry->bios_count >= VM_PRODUCT_SESSION_BIOS_SLOT_COUNT ||
                !vm_product_session_catalog_path(entry->bios[entry->bios_count],
                    VM_PRODUCT_SESSION_CATALOG_PATH_MAX, directory, value)) break;
            ++entry->bios_count;
            continue;
        }
        if (section == 3 &&
            vm_product_session_catalog_parse_value(text, "video", &value)) {
            if (firmware_video) break;
            firmware_video = 1;
            if (!STD_STRCMP(value, "null")) continue;
            if (!vm_product_session_catalog_path(entry->video, sizeof(entry->video), directory, value)) break;
            continue;
        }
        if (section == 3 &&
            vm_product_session_catalog_parse_value(text, "cmos", &value)) {
            if (firmware_cmos) break;
            firmware_cmos = 1;
            if (!STD_STRCMP(value, "null")) continue;
            if (!vm_product_session_catalog_path(entry->cmos, sizeof(entry->cmos), directory, value)) break;
            continue;
        }
        if (section == 3 &&
            vm_product_session_catalog_parse_value(text, "font", &value)) {
            if (firmware_font || !vm_product_session_catalog_path(entry->font,
                    sizeof(entry->font), directory, value)) break;
            firmware_font = 1;
            continue;
        }
        if (section == 2 && media != 0 && text[0] == '-' &&
            vm_product_session_catalog_parse_value(vm_product_session_catalog_trim(text + 1u),
                "path", &value)) {
            STD_SIZE_T *count = media == 1 ? &entry->floppy_count : &entry->fixed_disk_count;
            C_CHAR (*target)[VM_PRODUCT_SESSION_CATALOG_PATH_MAX] = media == 1 ?
                entry->floppy : entry->fixed_disk;
            if (*count >= VM_PRODUCT_SESSION_MEDIA_SLOT_COUNT || !vm_product_session_catalog_path(
                    target[*count], VM_PRODUCT_SESSION_CATALOG_PATH_MAX, directory, value)) break;
            ++*count;
            continue;
        }
        break;
    }
    if (!schema || !profile || !display || !media_section || !floppy ||
        !hard_disk || (media != 0 && media != 1 && media != 2)) return 0;
    if (firmware && !STD_STRCMP(entry->profile, "compaq-deskpro-386-model-40") &&
        entry->bios_count != 2u) return 0;
    if (firmware && !STD_STRCMP(entry->profile, "ibm-5160-model-268") &&
        (entry->bios_count == 0u || entry->bios_count > 2u)) return 0;
    if (!vm_product_session_catalog_copy(entry->file_name, sizeof(entry->file_name), name)) return 0;
    return 1;
}

type_status vm_product_session_request_parse(const C_CHAR *directory,
    const C_CHAR *name, C_CHAR *document, vm_product_session_request *out_request)
{
    return vm_product_session_catalog_parse_document(directory, name, document,
        out_request) ? TYPE_STATUS_OK : TYPE_STATUS_INVALID_ARGUMENT;
}

static C_INT vm_product_session_catalog_parse_file(const C_CHAR *directory,
    const C_CHAR *name, vm_product_session_request *entry)
{
    C_CHAR path[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR *document;
    C_VOID *loaded = STD_NULL;
    STD_SIZE_T bytes = 0u;
    C_INT parsed;

    if (!vm_product_session_catalog_path(path, sizeof(path), directory, name) ||
        core_platform_file_read_all(path, 64u * 1024u, &loaded, &bytes) != TYPE_FALSE ||
        (document = (C_CHAR *)STD_MALLOC(bytes + 1u)) == STD_NULL) {
        STD_FREE(loaded);
        return 0;
    }
    if (bytes != 0u) STD_MEMCPY(document, loaded, bytes);
    document[bytes] = '\0';
    STD_FREE(loaded);
    parsed = vm_product_session_catalog_parse_document(directory, name, document, entry);
    STD_FREE(document);
    return parsed;
}

static C_INT vm_product_session_catalog_compare(const C_VOID *left, const C_VOID *right)
{
    const vm_product_session_request *a = left;
    const vm_product_session_request *b = right;
    return STD_STRCMP(a->file_name, b->file_name);
}

type_status vm_product_session_catalog_create(const C_CHAR *directory,
    vm_product_session_catalog **out_catalog)
{
    vm_product_session_catalog *catalog;
    DIR *dir;
    struct dirent *item;

    if (out_catalog == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_catalog = STD_NULL;
    catalog = (vm_product_session_catalog *)STD_CALLOC(1u, sizeof(*catalog));
    if (catalog == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (directory == STD_NULL || (dir = opendir(directory)) == STD_NULL) {
        *out_catalog = catalog;
        return TYPE_STATUS_OK;
    }
    while ((item = readdir(dir)) != STD_NULL) {
        STD_SIZE_T length = STD_STRLEN(item->d_name);
        const C_CHAR *extension;
        vm_product_session_request entry;

        if (length < 5u || catalog->count == VM_PRODUCT_SESSION_CATALOG_MAX) continue;
        extension = item->d_name + length - 5u;
        if (STD_STRCMP(extension, ".yaml") &&
            (length < 4u || STD_STRCMP(item->d_name + length - 4u, ".yml"))) continue;
        if (vm_product_session_catalog_parse_file(directory, item->d_name, &entry)) {
            catalog->entries[catalog->count++] = entry;
        } else {
            ++catalog->rejected;
        }
    }
    closedir(dir);
    qsort(catalog->entries, catalog->count, sizeof(catalog->entries[0]),
        vm_product_session_catalog_compare);
    *out_catalog = catalog;
    return TYPE_STATUS_OK;
}

C_VOID vm_product_session_catalog_destroy(vm_product_session_catalog *catalog)
{
    STD_FREE(catalog);
}

STD_SIZE_T vm_product_session_catalog_count(const vm_product_session_catalog *catalog)
{
    return catalog == STD_NULL ? 0u : catalog->count;
}

STD_SIZE_T vm_product_session_catalog_rejected(const vm_product_session_catalog *catalog)
{
    return catalog == STD_NULL ? 0u : catalog->rejected;
}

type_status vm_product_session_catalog_get_request(const vm_product_session_catalog *catalog,
    STD_SIZE_T index, vm_product_session_request *out_request)
{
    if (catalog == STD_NULL || out_request == STD_NULL || index >= catalog->count) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_request = catalog->entries[index];
    return TYPE_STATUS_OK;
}
