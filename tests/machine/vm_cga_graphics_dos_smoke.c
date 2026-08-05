#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "tests/support/vm_session_fixture.h"

#define VM_CGA_DOS_BOOT_BUDGET 800000u
#define VM_CGA_DOS_RUN_BUDGET 400000u

static C_INT vm_cga_dos_fat12_get(const uint8_t *fat, uint16_t cluster)
{
    uint32_t offset = cluster + cluster / 2u;
    uint16_t value = (uint16_t)(fat[offset] | ((uint16_t)fat[offset + 1u] << 8));

    return (cluster & 1u) != 0u ? value >> 4 : value & 0x0fffu;
}

static C_VOID vm_cga_dos_fat12_set(uint8_t *fat, uint16_t cluster, uint16_t value)
{
    uint32_t offset = cluster + cluster / 2u;
    uint16_t pair = (uint16_t)(fat[offset] | ((uint16_t)fat[offset + 1u] << 8));

    if ((cluster & 1u) != 0u) pair = (uint16_t)((pair & 0x000fu) | (value << 4));
    else pair = (uint16_t)((pair & 0xf000u) | value);
    fat[offset] = (uint8_t)pair;
    fat[offset + 1u] = (uint8_t)(pair >> 8);
}

static C_INT vm_cga_dos_copy_file(const C_CHAR *source, C_CHAR path[MAX_PATH],
    uint8_t **out_image, DWORD *out_size)
{
    HANDLE input = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    LARGE_INTEGER size;
    uint8_t *image = STD_NULL;
    DWORD read_count;
    DWORD write_count;
    DWORD length;

    if (source == STD_NULL || out_image == STD_NULL || out_size == STD_NULL) return 0;
    input = CreateFileA(source, GENERIC_READ, FILE_SHARE_READ, STD_NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, STD_NULL);
    if (input == INVALID_HANDLE_VALUE || !GetFileSizeEx(input, &size) ||
        size.QuadPart <= 0 || size.QuadPart > MAXDWORD) goto fail;
    image = STD_MALLOC((STD_SIZE_T)size.QuadPart);
    if (image == STD_NULL || !ReadFile(input, image, (DWORD)size.QuadPart,
            &read_count, STD_NULL) || read_count != (DWORD)size.QuadPart) goto fail;
    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) goto fail;
    output = CreateFileA(path, GENERIC_WRITE, 0u, STD_NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, STD_NULL);
    if (output == INVALID_HANDLE_VALUE || !WriteFile(output, image,
            (DWORD)size.QuadPart, &write_count, STD_NULL) ||
        write_count != (DWORD)size.QuadPart) goto fail;
    CloseHandle(output);
    CloseHandle(input);
    *out_image = image;
    *out_size = (DWORD)size.QuadPart;
    return 1;

fail:
    if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
    if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
    if (path[0] != '\0') DeleteFileA(path);
    STD_FREE(image);
    return 0;
}

static C_INT vm_cga_dos_install_program(uint8_t *image, DWORD image_size,
    const C_CHAR *path)
{
    static const uint8_t program[] = {
        0xbau, 0xd8u, 0x03u, 0xb0u, 0x0au, 0xeeu,
        0xbau, 0xd9u, 0x03u, 0xb0u, 0x20u, 0xeeu,
        0xb8u, 0x00u, 0xb8u, 0x8eu, 0xc0u, 0x31u, 0xffu,
        0xb0u, 0x1bu, 0xaau, 0xbfu, 0x00u, 0x20u, 0xb0u,
        0xe4u, 0xaau, 0xb8u, 0x2au, 0x4cu, 0xcdu, 0x21u
    };
    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;
    uint32_t reserved_sectors;
    uint32_t fat_count;
    uint32_t root_entries;
    uint32_t sectors_per_fat;
    uint32_t root_start;
    uint32_t root_bytes;
    uint32_t data_start;
    uint32_t clusters;
    uint32_t cluster;
    uint32_t root;
    uint8_t *entry = STD_NULL;
    HANDLE output;
    DWORD written;

    if (image == STD_NULL || image_size < 512u || path == STD_NULL) return 0;
    bytes_per_sector = image[11u] | ((uint32_t)image[12u] << 8);
    sectors_per_cluster = image[13u];
    reserved_sectors = image[14u] | ((uint32_t)image[15u] << 8);
    fat_count = image[16u];
    root_entries = image[17u] | ((uint32_t)image[18u] << 8);
    sectors_per_fat = image[22u] | ((uint32_t)image[23u] << 8);
    if (bytes_per_sector == 0u || sectors_per_cluster == 0u || fat_count == 0u ||
        sectors_per_fat == 0u) return 0;
    root_start = (reserved_sectors + fat_count * sectors_per_fat) * bytes_per_sector;
    root_bytes = root_entries * 32u;
    data_start = root_start + ((root_bytes + bytes_per_sector - 1u) /
        bytes_per_sector) * bytes_per_sector;
    if (data_start >= image_size || root_start + root_bytes > image_size) return 0;
    for (root = 0u; root < root_entries; ++root) {
        uint8_t *candidate = image + root_start + root * 32u;
        if (candidate[0] == 0u || candidate[0] == 0xe5u) {
            entry = candidate;
            break;
        }
    }
    if (entry == STD_NULL) return 0;
    clusters = (image_size - data_start) / (bytes_per_sector * sectors_per_cluster);
    for (cluster = 2u; cluster < clusters + 2u; ++cluster) {
        if (vm_cga_dos_fat12_get(image + reserved_sectors * bytes_per_sector,
                (uint16_t)cluster) == 0) break;
    }
    if (cluster >= clusters + 2u || sizeof(program) >
        bytes_per_sector * sectors_per_cluster) return 0;
    STD_MEMSET(entry, 0, 32u);
    STD_MEMCPY(entry, "CGAT228 COM", 11u);
    entry[11u] = 0x20u;
    entry[26u] = (uint8_t)cluster;
    entry[27u] = (uint8_t)(cluster >> 8);
    entry[28u] = (uint8_t)sizeof(program);
    entry[29u] = (uint8_t)(sizeof(program) >> 8);
    for (root = 0u; root < fat_count; ++root) {
        vm_cga_dos_fat12_set(image + (reserved_sectors + root * sectors_per_fat) *
            bytes_per_sector, (uint16_t)cluster, 0x0fffu);
    }
    STD_MEMCPY(image + data_start + (cluster - 2u) * bytes_per_sector *
        sectors_per_cluster, program, sizeof(program));
    output = CreateFileA(path, GENERIC_WRITE, 0u, STD_NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, STD_NULL);
    if (output == INVALID_HANDLE_VALUE) return 0;
    if (!WriteFile(output, image, image_size, &written, STD_NULL) ||
        written != image_size) {
        CloseHandle(output);
        return 0;
    }
    CloseHandle(output);
    return 1;
}

static C_INT vm_cga_dos_has_prompt(const core_machine_display_snapshot *snapshot)
{
    STD_SIZE_T cell;

    if (snapshot == STD_NULL || snapshot->kind != CORE_MACHINE_DISPLAY_KIND_TEXT) {
        return 0;
    }
    for (cell = 0u; cell + 3u < 80u * 25u; ++cell) {
        if (STD_ISALPHA(snapshot->characters[cell]) &&
            snapshot->characters[cell + 1u] == ':' &&
            snapshot->characters[cell + 2u] == '\\' &&
            snapshot->characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

static C_INT vm_cga_dos_run_until(vm_session *session, uint32_t limit,
    C_INT want_graphics)
{
    core_machine_run_budget budget = { 128u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    uint32_t executed = 0u;

    while (executed < limit) {
        if (core_machine_run(vm_session_fixture_machine(session), budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_capture_display_snapshot(vm_session_fixture_machine(session),
                &snapshot) != TYPE_STATUS_OK) return 0;
        if (!want_graphics && vm_cga_dos_has_prompt(&snapshot)) return 1;
        if (want_graphics && snapshot.kind == CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4 &&
            snapshot.pixels[0] == 0u && snapshot.pixels[1] == 1u &&
            snapshot.pixels[2] == 2u && snapshot.pixels[3] == 3u &&
            snapshot.pixels[320u] == 3u && snapshot.pixels[321u] == 2u &&
            snapshot.pixels[322u] == 1u && snapshot.pixels[323u] == 0u) return 1;
        executed += budget.instructions;
    }
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    static const uint8_t command[] = { 0x2eu, 0x22u, 0x1eu, 0x14u, 0x03u,
        0x03u, 0x09u, 0x1cu };
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    uint8_t *image = STD_NULL;
    DWORD image_size = 0u;
    C_CHAR path[MAX_PATH] = {0};
    STD_SIZE_T index;
    C_INT passed = 0;

    if (argc != 2 || !vm_cga_dos_copy_file(argv[1], path, &image, &image_size) ||
        !vm_cga_dos_install_program(image, image_size, path)) goto done;
    config.fdd_image = path;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    if (vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL ||
        !vm_cga_dos_run_until(session, VM_CGA_DOS_BOOT_BUDGET, 0)) goto done;
    for (index = 0u; index < sizeof(command); ++index) {
        if (core_machine_keyboard_submit_scan_code(vm_session_fixture_machine(session),
                command[index]) != TYPE_STATUS_OK) goto done;
    }
    passed = vm_cga_dos_run_until(session, VM_CGA_DOS_RUN_BUDGET, 1);

done:
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    STD_FREE(image);
    if (!passed) return 1;
    STD_PRINTF("M5:T228:S3:CGA:DOS:OK\n");
    return 0;
}
