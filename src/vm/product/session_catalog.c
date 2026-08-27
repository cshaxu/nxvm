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

static C_INT vm_product_session_catalog_sha256_is_valid(const C_CHAR *value)
{
    STD_SIZE_T index;
    if (value == STD_NULL || STD_STRLEN(value) != 64u) return 0;
    for (index = 0u; index < 64u; ++index) {
        C_CHAR c = value[index];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F'))) return 0;
    }
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
    const C_CHAR *name, vm_product_session_request *entry)
{
    C_CHAR path[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR line[512];
    C_CHAR *value;
    core_platform_file_reader *file = STD_NULL;
    C_INT section = 0;
    C_INT media = 0;
    C_INT firmware_slot = 0;
    C_INT firmware_even_slot = 0;
    C_INT firmware_even_section = 0;
    C_INT firmware_even_bytes = 0;
    C_INT firmware_even_map = 0;
    C_INT firmware_even_path = 0;
    C_INT firmware_even_sha256 = 0;
    C_INT firmware_odd_slot = 0;
    C_INT firmware_odd_section = 0;
    C_INT firmware_odd_bytes = 0;
    C_INT firmware_odd_map = 0;
    C_INT firmware_odd_path = 0;
    C_INT firmware_odd_sha256 = 0;
    C_INT firmware_xt_system_slot = 0;
    C_INT firmware_xt_system_section = 0;
    C_INT firmware_xt_system_bytes = 0;
    C_INT firmware_xt_system_map = 0;
    C_INT firmware_xt_system_path = 0;
    C_INT firmware_xt_system_sha256 = 0;
    C_INT firmware_xt_xebec_slot = 0;
    C_INT firmware_xt_xebec_section = 0;
    C_INT firmware_xt_xebec_bytes = 0;
    C_INT firmware_xt_xebec_map = 0;
    C_INT firmware_xt_xebec_path = 0;
    C_INT firmware_xt_xebec_sha256 = 0;
    C_INT firmware_provenance = 0;
    C_INT firmware = 0;
    C_INT media_section = 0;
    C_INT schema = 0;
    C_INT profile = 0;
    C_INT cpu = 0;
    C_INT fpu = 0;
    C_INT memory = 0;
    C_INT display = 0;
    C_INT boot = 0;
    C_INT floppy_format = 0;
    C_INT floppy = 0;
    C_INT hard_disk = 0;
    C_CHAR firmware_provenance_text[sizeof(entry->model40_provenance)] = {0};

    if (!vm_product_session_catalog_path(path, sizeof(path), directory, name) ||
        core_platform_file_reader_open(path, &file) != TYPE_STATUS_OK) return 0;
    STD_MEMSET(entry, 0, sizeof(*entry));
    while (core_platform_file_reader_next(file, line, sizeof(line))) {
        C_CHAR *text = vm_product_session_catalog_trim(line);
        if (*text == '\0' || *text == '#') continue;
        if (!STD_STRCMP(text, "media:")) {
            if (media_section) break;
            section = 2; media = 0; firmware_slot = 0; media_section = 1; continue;
        }
        if (!STD_STRCMP(text, "firmware:")) {
            if (firmware) break;
            section = 3; media = 0; firmware_slot = 0; firmware = 1; continue;
        }
        if (section == 3 && !STD_STRCMP(text, "rom_even:")) {
            if (firmware_even_section) break;
            firmware_slot = 1; firmware_even_section = 1; continue;
        }
        if (section == 3 && !STD_STRCMP(text, "rom_odd:")) {
            if (firmware_odd_section) break;
            firmware_slot = 2; firmware_odd_section = 1; continue;
        }
        if (section == 3 && !STD_STRCMP(text, "system_rom:")) {
            if (firmware_xt_system_section) break;
            firmware_slot = 3; firmware_xt_system_section = 1; continue;
        }
        if (section == 3 && !STD_STRCMP(text, "xebec_option:")) {
            if (firmware_xt_xebec_section) break;
            firmware_slot = 4; firmware_xt_xebec_section = 1; continue;
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
        if (section == 0 && vm_product_session_catalog_parse_value(text, "boot", &value)) {
            if (boot || !vm_product_session_catalog_copy(entry->boot, sizeof(entry->boot), value)) break;
            boot = 1; continue;
        }
        if (section == 0 && vm_product_session_catalog_parse_value(text, "floppy_format", &value)) {
            if (floppy_format || !vm_product_session_catalog_copy(entry->floppy_format,
                    sizeof(entry->floppy_format), value)) break;
            floppy_format = 1; continue;
        }
        if (section == 2 && vm_product_session_catalog_parse_value(text, "floppy", &value)) {
            if (floppy) break;
            floppy = 1;
            if (!STD_STRCMP(value, "null")) continue;
            if (*value == '\0') { media = 1; continue; }
            break;
        }
        if (section == 2 && vm_product_session_catalog_parse_value(text, "hard_disk", &value)) {
            if (hard_disk) break;
            hard_disk = 1;
            if (!STD_STRCMP(value, "null")) continue;
            if (*value == '\0') { media = 2; continue; }
            break;
        }
        if (section == 3 && firmware_slot != 0 && vm_product_session_catalog_parse_value(text, "slot", &value)) {
            if ((firmware_slot == 1 && !STD_STRCMP(value, "system-rom-even")) ||
                (firmware_slot == 2 && !STD_STRCMP(value, "system-rom-odd")) ||
                (firmware_slot == 3 && !STD_STRCMP(value, "xt-base-system")) ||
                (firmware_slot == 4 && !STD_STRCMP(value, "xt-xebec-option"))) {
                if ((firmware_slot == 1 && firmware_even_slot) ||
                    (firmware_slot == 2 && firmware_odd_slot) ||
                    (firmware_slot == 3 && firmware_xt_system_slot) ||
                    (firmware_slot == 4 && firmware_xt_xebec_slot)) break;
                if (firmware_slot == 1) firmware_even_slot = 1;
                else if (firmware_slot == 2) firmware_odd_slot = 1;
                else if (firmware_slot == 3) firmware_xt_system_slot = 1;
                else firmware_xt_xebec_slot = 1;
                continue;
            }
            break;
        }
        if (section == 3 && firmware_slot != 0 && vm_product_session_catalog_parse_value(text, "path", &value)) {
            C_CHAR *target = firmware_slot == 1 ? entry->model40_even_path :
                firmware_slot == 2 ? entry->model40_odd_path : firmware_slot == 3 ?
                entry->xt_system_path : entry->xt_xebec_path;
            if ((firmware_slot == 1 && firmware_even_path) ||
                (firmware_slot == 2 && firmware_odd_path) ||
                (firmware_slot == 3 && firmware_xt_system_path) ||
                (firmware_slot == 4 && firmware_xt_xebec_path) ||
                !vm_product_session_catalog_path(target, VM_PRODUCT_SESSION_CATALOG_PATH_MAX,
                    directory, value)) break;
            if (firmware_slot == 1) firmware_even_path = 1;
            else if (firmware_slot == 2) firmware_odd_path = 1;
            else if (firmware_slot == 3) firmware_xt_system_path = 1;
            else firmware_xt_xebec_path = 1;
            continue;
        }
        if (section == 3 && firmware_slot != 0 && vm_product_session_catalog_parse_value(text, "bytes", &value)) {
            if ((firmware_slot <= 2 && STD_STRCMP(value, "16384")) ||
                (firmware_slot == 3 && STD_STRCMP(value, "65536")) ||
                (firmware_slot == 4 && STD_STRCMP(value, "8192"))) break;
            if ((firmware_slot == 1 && firmware_even_bytes) ||
                (firmware_slot == 2 && firmware_odd_bytes) ||
                (firmware_slot == 3 && firmware_xt_system_bytes) ||
                (firmware_slot == 4 && firmware_xt_xebec_bytes)) break;
            if (firmware_slot == 1) firmware_even_bytes = 1;
            else if (firmware_slot == 2) firmware_odd_bytes = 1;
            else if (firmware_slot == 3) firmware_xt_system_bytes = 1;
            else firmware_xt_xebec_bytes = 1;
            continue;
        }
        if (section == 3 && firmware_slot != 0 && vm_product_session_catalog_parse_value(text, "sha256", &value)) {
            C_CHAR *target = firmware_slot == 1 ? entry->model40_even_sha256 :
                firmware_slot == 2 ? entry->model40_odd_sha256 : firmware_slot == 3 ?
                entry->xt_system_sha256 : entry->xt_xebec_sha256;
            if ((firmware_slot == 1 && firmware_even_sha256) ||
                (firmware_slot == 2 && firmware_odd_sha256) ||
                (firmware_slot == 3 && firmware_xt_system_sha256) ||
                (firmware_slot == 4 && firmware_xt_xebec_sha256) ||
                !vm_product_session_catalog_copy(target, 65u, value)) break;
            if (firmware_slot == 1) firmware_even_sha256 = 1;
            else if (firmware_slot == 2) firmware_odd_sha256 = 1;
            else if (firmware_slot == 3) firmware_xt_system_sha256 = 1;
            else firmware_xt_xebec_sha256 = 1;
            continue;
        }
        if (section == 3 && firmware_slot != 0 && vm_product_session_catalog_parse_value(text, "map", &value)) {
            if (STD_STRCMP(value, "read-only")) break;
            if ((firmware_slot == 1 && firmware_even_map) ||
                (firmware_slot == 2 && firmware_odd_map) ||
                (firmware_slot == 3 && firmware_xt_system_map) ||
                (firmware_slot == 4 && firmware_xt_xebec_map)) break;
            if (firmware_slot == 1) firmware_even_map = 1;
            else if (firmware_slot == 2) firmware_odd_map = 1;
            else if (firmware_slot == 3) firmware_xt_system_map = 1;
            else firmware_xt_xebec_map = 1;
            continue;
        }
        if (section == 3 && firmware_slot == 0 && vm_product_session_catalog_parse_value(text, "provenance", &value)) {
            if (firmware_provenance || *value == '\0' || !vm_product_session_catalog_copy(
                    firmware_provenance_text, sizeof(firmware_provenance_text), value)) break;
            firmware_provenance = 1;
            continue;
        }
        if (section == 2 && media != 0 && vm_product_session_catalog_parse_value(text, "image", &value)) {
            C_CHAR *target = media == 1 ? entry->floppy : entry->hard_disk;
            if (!vm_product_session_catalog_path(target, VM_PRODUCT_SESSION_CATALOG_PATH_MAX,
                    directory, value)) break;
            media = 0; continue;
        }
        break;
    }
    core_platform_file_reader_close(file);
    if (!schema || !profile || !display || !boot || !media_section || !floppy ||
        !hard_disk || media != 0) return 0;
    if (firmware && !STD_STRCMP(entry->profile, "compaq-deskpro-386-model-40") &&
        !vm_product_session_catalog_copy(entry->model40_provenance,
            sizeof(entry->model40_provenance), firmware_provenance_text)) return 0;
    if (firmware && !STD_STRCMP(entry->profile, "ibm-5160-model-268") &&
        !vm_product_session_catalog_copy(entry->xt_provenance,
            sizeof(entry->xt_provenance), firmware_provenance_text)) return 0;
    if (firmware && !STD_STRCMP(entry->profile, "compaq-deskpro-386-model-40") &&
        (!firmware_even_slot || !firmware_even_bytes || !firmware_even_map ||
         !firmware_odd_slot || !firmware_odd_bytes || !firmware_odd_map ||
         !firmware_provenance || entry->model40_even_path[0] == '\0' ||
         entry->model40_even_sha256[0] == '\0' || entry->model40_odd_path[0] == '\0' ||
         entry->model40_odd_sha256[0] == '\0' ||
         !vm_product_session_catalog_sha256_is_valid(entry->model40_even_sha256) ||
         !vm_product_session_catalog_sha256_is_valid(entry->model40_odd_sha256))) return 0;
    if (firmware && !STD_STRCMP(entry->profile, "ibm-5160-model-268") &&
        (!firmware_xt_system_slot || !firmware_xt_system_bytes || !firmware_xt_system_map ||
         !firmware_provenance || entry->xt_system_path[0] == '\0' ||
         !vm_product_session_catalog_sha256_is_valid(entry->xt_system_sha256) ||
         (firmware_xt_xebec_section && (!firmware_xt_xebec_slot || !firmware_xt_xebec_bytes ||
          !firmware_xt_xebec_map || entry->xt_xebec_path[0] == '\0' ||
          !vm_product_session_catalog_sha256_is_valid(entry->xt_xebec_sha256))))) return 0;
    if (!vm_product_session_catalog_copy(entry->file_name, sizeof(entry->file_name), name)) return 0;
    return 1;
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
        if (vm_product_session_catalog_parse(directory, item->d_name, &entry)) {
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
