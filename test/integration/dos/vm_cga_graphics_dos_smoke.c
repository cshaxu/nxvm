#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"

#define VM_CGA_DOS_BOOT_BUDGET 800000u
#define VM_CGA_DOS_RUN_BUDGET 400000u

static C_INT vm_cga_dos_fat12_get(const type_unsigned_8 *fat, type_unsigned_16 cluster)
{
    type_unsigned_32 offset = cluster + cluster / 2u;
    type_unsigned_16 value = (type_unsigned_16)(fat[offset] | ((type_unsigned_16)fat[offset + 1u] << 8));

    return (cluster & 1u) != 0u ? value >> 4 : value & 0x0fffu;
}

static C_VOID vm_cga_dos_fat12_set(type_unsigned_8 *fat, type_unsigned_16 cluster, type_unsigned_16 value)
{
    type_unsigned_32 offset = cluster + cluster / 2u;
    type_unsigned_16 pair = (type_unsigned_16)(fat[offset] | ((type_unsigned_16)fat[offset + 1u] << 8));

    if ((cluster & 1u) != 0u) pair = (type_unsigned_16)((pair & 0x000fu) | (value << 4));
    else pair = (type_unsigned_16)((pair & 0xf000u) | value);
    fat[offset] = (type_unsigned_8)pair;
    fat[offset + 1u] = (type_unsigned_8)(pair >> 8);
}

static C_INT vm_cga_dos_copy_file(const C_CHAR *source, C_CHAR path[MAX_PATH],
    type_unsigned_8 **out_image, DWORD *out_size)
{
    HANDLE input = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    LARGE_INTEGER size;
    type_unsigned_8 *image = STD_NULL;
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

static C_INT vm_cga_dos_install_program(type_unsigned_8 *image, DWORD image_size,
    const C_CHAR *path)
{
    static const type_unsigned_8 program[] = {
        0xbau, 0xd8u, 0x03u, 0xb0u, 0x0au, 0xeeu,
        0xbau, 0xd9u, 0x03u, 0xb0u, 0x20u, 0xeeu,
        0xb8u, 0x00u, 0xb8u, 0x8eu, 0xc0u, 0x31u, 0xffu,
        0xb0u, 0x1bu, 0xaau, 0xbfu, 0x00u, 0x20u, 0xb0u,
        0xe4u, 0xaau, 0xb8u, 0x2au, 0x4cu, 0xcdu, 0x21u
    };
    type_unsigned_32 bytes_per_sector;
    type_unsigned_32 sectors_per_cluster;
    type_unsigned_32 reserved_sectors;
    type_unsigned_32 fat_count;
    type_unsigned_32 root_entries;
    type_unsigned_32 sectors_per_fat;
    type_unsigned_32 root_start;
    type_unsigned_32 root_bytes;
    type_unsigned_32 data_start;
    type_unsigned_32 clusters;
    type_unsigned_32 cluster;
    type_unsigned_32 root;
    type_unsigned_8 *entry = STD_NULL;
    HANDLE output;
    DWORD written;

    if (image == STD_NULL || image_size < 512u || path == STD_NULL) return 0;
    bytes_per_sector = image[11u] | ((type_unsigned_32)image[12u] << 8);
    sectors_per_cluster = image[13u];
    reserved_sectors = image[14u] | ((type_unsigned_32)image[15u] << 8);
    fat_count = image[16u];
    root_entries = image[17u] | ((type_unsigned_32)image[18u] << 8);
    sectors_per_fat = image[22u] | ((type_unsigned_32)image[23u] << 8);
    if (bytes_per_sector == 0u || sectors_per_cluster == 0u || fat_count == 0u ||
        sectors_per_fat == 0u) return 0;
    root_start = (reserved_sectors + fat_count * sectors_per_fat) * bytes_per_sector;
    root_bytes = root_entries * 32u;
    data_start = root_start + ((root_bytes + bytes_per_sector - 1u) /
        bytes_per_sector) * bytes_per_sector;
    if (data_start >= image_size || root_start + root_bytes > image_size) return 0;
    for (root = 0u; root < root_entries; ++root) {
        type_unsigned_8 *candidate = image + root_start + root * 32u;
        if (candidate[0] == 0u || candidate[0] == 0xe5u) {
            entry = candidate;
            break;
        }
    }
    if (entry == STD_NULL) return 0;
    clusters = (image_size - data_start) / (bytes_per_sector * sectors_per_cluster);
    for (cluster = 2u; cluster < clusters + 2u; ++cluster) {
        if (vm_cga_dos_fat12_get(image + reserved_sectors * bytes_per_sector,
                (type_unsigned_16)cluster) == 0) break;
    }
    if (cluster >= clusters + 2u || sizeof(program) >
        bytes_per_sector * sectors_per_cluster) return 0;
    STD_MEMSET(entry, 0, 32u);
    STD_MEMCPY(entry, "CGAT228 COM", 11u);
    entry[11u] = 0x20u;
    entry[26u] = (type_unsigned_8)cluster;
    entry[27u] = (type_unsigned_8)(cluster >> 8);
    entry[28u] = (type_unsigned_8)sizeof(program);
    entry[29u] = (type_unsigned_8)(sizeof(program) >> 8);
    for (root = 0u; root < fat_count; ++root) {
        vm_cga_dos_fat12_set(image + (reserved_sectors + root * sectors_per_fat) *
            bytes_per_sector, (type_unsigned_16)cluster, 0x0fffu);
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

static C_INT vm_cga_dos_run_until(vm_session *session, type_unsigned_32 limit,
    C_INT want_graphics)
{
    core_machine_run_budget budget = { 128u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    type_unsigned_32 executed = 0u;

    while (executed < limit) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK) return 0;
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            C_INT advanced = 0;

            if (vm_session_waiting_advance(session, &result, &advanced) != TYPE_STATUS_OK ||
                !advanced) return 0;
        }
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
    static const type_unsigned_8 command[] = { 0x21u, 0x34u, 0x1cu, 0x2cu, 0x1eu,
        0x1eu, 0x3eu, 0x5au };
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    type_unsigned_8 *image = STD_NULL;
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
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
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
