#include "type.h"

#include <dirent.h>

#include "vm/product/session_catalog.h"
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

static C_INT vm_product_session_catalog_parse(const C_CHAR *directory,
    const C_CHAR *name, vm_product_session_catalog_entry *entry)
{
    C_CHAR path[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR line[512];
    C_CHAR *value;
    STD_FILE *file;
    C_INT section = 0;
    C_INT media = 0;
    C_INT schema = 0;
    C_INT profile = 0;
    C_INT display = 0;
    C_INT boot = 0;

    if (!vm_product_session_catalog_path(path, sizeof(path), directory, name) ||
        (file = STD_FOPEN(path, "rb")) == STD_NULL) return 0;
    STD_MEMSET(entry, 0, sizeof(*entry));
    while (STD_FGETS(line, (C_INT)sizeof(line), file) != STD_NULL) {
        C_CHAR *text = vm_product_session_catalog_trim(line);
        if (*text == '\0' || *text == '#') continue;
        if (!STD_STRCMP(text, "machine:")) { section = 1; media = 0; continue; }
        if (!STD_STRCMP(text, "media:")) { section = 2; media = 0; continue; }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "schema", &value)) {
            if (schema || STD_STRCMP(value, "nxvm-session/v1")) break;
            schema = 1; continue;
        }
        if (section == 1 && vm_product_session_catalog_parse_value(text, "profile", &value)) {
            if (profile || !vm_product_session_catalog_copy(entry->profile, sizeof(entry->profile), value)) break;
            profile = 1; continue;
        }
        if (section == 1 && vm_product_session_catalog_parse_value(text, "cpu", &value)) {
            if (!vm_product_session_catalog_copy(entry->cpu, sizeof(entry->cpu), value)) break;
            continue;
        }
        if (section == 1 && vm_product_session_catalog_parse_value(text, "fpu", &value)) {
            if (!vm_product_session_catalog_copy(entry->fpu, sizeof(entry->fpu), value)) break;
            continue;
        }
        if (section == 1 && vm_product_session_catalog_parse_value(text, "memory_kib", &value)) {
            if (core_product_utils_parse_memory_kib(value,
                    &entry->memory_bytes) != TYPE_STATUS_OK) break;
            continue;
        }
        if (section == 1 && vm_product_session_catalog_parse_value(text, "display", &value)) {
            if (display || !vm_product_session_catalog_copy(entry->display, sizeof(entry->display), value)) break;
            display = 1; continue;
        }
        if (section == 1 && vm_product_session_catalog_parse_value(text, "boot", &value)) {
            if (boot || !vm_product_session_catalog_copy(entry->boot, sizeof(entry->boot), value)) break;
            boot = 1; continue;
        }
        if (section == 2 && vm_product_session_catalog_parse_value(text, "floppy", &value)) {
            if (!STD_STRCMP(value, "null")) continue;
            if (*value == '\0') { media = 1; continue; }
            break;
        }
        if (section == 2 && vm_product_session_catalog_parse_value(text, "hard_disk", &value)) {
            if (!STD_STRCMP(value, "null")) continue;
            if (*value == '\0') { media = 2; continue; }
            break;
        }
        if (section == 2 && media != 0 && vm_product_session_catalog_parse_value(text, "image", &value)) {
            C_CHAR *target = media == 1 ? entry->floppy : entry->hard_disk;
            if (!vm_product_session_catalog_path(target, VM_PRODUCT_SESSION_CATALOG_PATH_MAX,
                    directory, value)) break;
            media = 0; continue;
        }
        break;
    }
    STD_FCLOSE(file);
    if (!schema || !profile || !display || !boot || media != 0) return 0;
    if (STD_STRCMP(entry->profile, "default-pc-at") &&
        STD_STRCMP(entry->profile, "ibm-5170-model-339")) return 0;
    if (!STD_STRCMP(entry->profile, "ibm-5170-model-339") &&
        (entry->cpu[0] != '\0' || entry->fpu[0] != '\0' ||
         entry->memory_bytes != 0u || entry->hard_disk[0] != '\0')) return 0;
    if (STD_STRCMP(entry->display, "console") && STD_STRCMP(entry->display, "window") &&
        STD_STRCMP(entry->display, "auto")) return 0;
    if (STD_STRCMP(entry->boot, "floppy") && STD_STRCMP(entry->boot, "hard_disk") &&
        STD_STRCMP(entry->boot, "rom")) return 0;
    if ((!STD_STRCMP(entry->boot, "floppy") && entry->floppy[0] == '\0') ||
        (!STD_STRCMP(entry->boot, "hard_disk") && entry->hard_disk[0] == '\0')) return 0;
    if (!vm_product_session_catalog_copy(entry->file_name, sizeof(entry->file_name), name)) return 0;
    return 1;
}

static C_INT vm_product_session_catalog_compare(const C_VOID *left, const C_VOID *right)
{
    const vm_product_session_catalog_entry *a = left;
    const vm_product_session_catalog_entry *b = right;
    return STD_STRCMP(a->file_name, b->file_name);
}

C_VOID vm_product_session_catalog_initialize(vm_product_session_catalog *catalog,
    const C_CHAR *directory)
{
    DIR *dir;
    struct dirent *item;

    if (catalog == STD_NULL) return;
    STD_MEMSET(catalog, 0, sizeof(*catalog));
    if (directory == STD_NULL || (dir = opendir(directory)) == STD_NULL) return;
    while ((item = readdir(dir)) != STD_NULL) {
        STD_SIZE_T length = STD_STRLEN(item->d_name);
        const C_CHAR *extension;
        vm_product_session_catalog_entry entry;

        if (length < 5u || catalog->count == VM_PRODUCT_SESSION_CATALOG_MAX) continue;
        extension = item->d_name + length - 5u;
        if (STD_STRCMP(extension, ".yaml") &&
            (length < 4u || STD_STRCMP(item->d_name + length - 4u, ".yml"))) continue;
        if (vm_product_session_catalog_parse(directory, item->d_name, &entry)) {
            catalog->entries[catalog->count++] = entry;
        } else {
            ++catalog->rejected;
        }
    }
    closedir(dir);
    qsort(catalog->entries, catalog->count, sizeof(catalog->entries[0]),
        vm_product_session_catalog_compare);
}

const vm_product_session_catalog_entry *vm_product_session_catalog_get(
    const vm_product_session_catalog *catalog, STD_SIZE_T index)
{
    return catalog != STD_NULL && index < catalog->count ? &catalog->entries[index] : STD_NULL;
}
