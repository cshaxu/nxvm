#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "core/machine/hdc.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_ATA253_BOOT_BUDGET 800000u
#define VM_ATA253_RUN_BUDGET 400000u
#define VM_ATA253_MARKER_CELL 1920u

typedef struct vm_ata253_program {
    type_unsigned_8 bytes[256];
    type_unsigned_16 length;
} vm_ata253_program;

static C_INT vm_ata253_put(vm_ata253_program *program, type_unsigned_8 value)
{
    if (program == STD_NULL || program->length >= sizeof(program->bytes)) return 0;
    program->bytes[program->length++] = value;
    return 1;
}

static C_INT vm_ata253_word(vm_ata253_program *program, type_unsigned_16 value)
{
    return vm_ata253_put(program, (type_unsigned_8)value) &&
        vm_ata253_put(program, (type_unsigned_8)(value >> 8u));
}

static C_INT vm_ata253_out_task_file(vm_ata253_program *program,
    type_unsigned_8 command)
{
    return vm_ata253_put(program, 0xbau) && vm_ata253_word(program, 0x01f2u) &&
        vm_ata253_put(program, 0xb0u) && vm_ata253_put(program, 2u) &&
        vm_ata253_put(program, 0xeeu) &&
        vm_ata253_put(program, 0x42u) && vm_ata253_put(program, 0xb0u) &&
        vm_ata253_put(program, 3u) && vm_ata253_put(program, 0xeeu) &&
        vm_ata253_put(program, 0x42u) && vm_ata253_put(program, 0x30u) &&
        vm_ata253_put(program, 0xc0u) && vm_ata253_put(program, 0xeeu) &&
        vm_ata253_put(program, 0x42u) && vm_ata253_put(program, 0xeeu) &&
        vm_ata253_put(program, 0x42u) && vm_ata253_put(program, 0xb0u) &&
        vm_ata253_put(program, 0x40u) && vm_ata253_put(program, 0xeeu) &&
        vm_ata253_put(program, 0x42u) && vm_ata253_put(program, 0xb0u) &&
        vm_ata253_put(program, command) && vm_ata253_put(program, 0xeeu);
}

static C_INT vm_ata253_write_sector(vm_ata253_program *program, type_unsigned_16 word)
{
    return vm_ata253_put(program, 0xb9u) && vm_ata253_word(program, 256u) &&
        vm_ata253_put(program, 0xb8u) && vm_ata253_word(program, word) &&
        vm_ata253_put(program, 0xefu) && vm_ata253_put(program, 0xe2u) &&
        vm_ata253_put(program, 0xfdu);
}

static C_INT vm_ata253_set_nien(vm_ata253_program *program, C_INT enabled)
{
    return vm_ata253_put(program, 0xbau) && vm_ata253_word(program, 0x03f6u) &&
        vm_ata253_put(program, 0xb0u) &&
        vm_ata253_put(program, enabled ? CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN : 0u) &&
        vm_ata253_put(program, 0xeeu);
}

static C_INT vm_ata253_discard_words(vm_ata253_program *program, type_unsigned_16 count)
{
    return vm_ata253_put(program, 0xb9u) && vm_ata253_word(program, count) &&
        vm_ata253_put(program, 0xedu) && vm_ata253_put(program, 0xe2u) &&
        vm_ata253_put(program, 0xfdu);
}

static C_INT vm_ata253_wait_drq(vm_ata253_program *program)
{
    return vm_ata253_put(program, 0xbau) && vm_ata253_word(program, 0x01f7u) &&
        vm_ata253_put(program, 0xecu) && vm_ata253_put(program, 0xa8u) &&
        vm_ata253_put(program, CORE_MACHINE_HDC_STATUS_BSY) && vm_ata253_put(program, 0x75u) &&
        vm_ata253_put(program, 0xfbu) && vm_ata253_put(program, 0xa8u) &&
        vm_ata253_put(program, CORE_MACHINE_HDC_STATUS_DRQ) && vm_ata253_put(program, 0x74u) &&
        vm_ata253_put(program, 0xf7u);
}

static C_INT vm_ata253_wait_ready(vm_ata253_program *program)
{
    return vm_ata253_put(program, 0xbau) && vm_ata253_word(program, 0x01f7u) &&
        vm_ata253_put(program, 0xecu) && vm_ata253_put(program, 0xa8u) &&
        vm_ata253_put(program, CORE_MACHINE_HDC_STATUS_BSY) && vm_ata253_put(program, 0x75u) &&
        vm_ata253_put(program, 0xfbu) && vm_ata253_put(program, 0xa8u) &&
        vm_ata253_put(program, CORE_MACHINE_HDC_STATUS_DRQ) && vm_ata253_put(program, 0x75u) &&
        vm_ata253_put(program, 0xf7u);
}

static C_INT vm_ata253_marker(vm_ata253_program *program, type_unsigned_8 character,
    type_unsigned_8 exit_code)
{
    return vm_ata253_put(program, 0xb8u) && vm_ata253_word(program, 0xb800u) &&
        vm_ata253_put(program, 0x8eu) && vm_ata253_put(program, 0xc0u) &&
        vm_ata253_put(program, 0x26u) && vm_ata253_put(program, 0xc7u) &&
        vm_ata253_put(program, 0x06u) && vm_ata253_word(program, 0x0f00u) &&
        vm_ata253_word(program, (type_unsigned_16)(0x0700u | character)) &&
        vm_ata253_put(program, 0xb8u) && vm_ata253_word(program,
            (type_unsigned_16)(0x4c00u | exit_code)) && vm_ata253_put(program, 0xcdu) &&
        vm_ata253_put(program, 0x21u);
}

static C_INT vm_ata253_build_program(vm_ata253_program *program)
{
    type_unsigned_16 first_failure;
    type_unsigned_16 second_failure;
    type_unsigned_16 failure;
    type_signed_16 delta;

    if (program == STD_NULL) return 0;
    STD_MEMSET(program, 0, sizeof(*program));
    if (!vm_ata253_set_nien(program, 1) ||
        !vm_ata253_out_task_file(program, 0x30u) ||
        !vm_ata253_wait_drq(program) ||
        !vm_ata253_put(program, 0xbau) || !vm_ata253_word(program, 0x01f0u) ||
        !vm_ata253_write_sector(program, 0x1357u) ||
        !vm_ata253_wait_drq(program) ||
        !vm_ata253_put(program, 0xbau) || !vm_ata253_word(program, 0x01f0u) ||
        !vm_ata253_write_sector(program, 0x2468u) ||
        !vm_ata253_wait_ready(program) ||
        !vm_ata253_set_nien(program, 0) ||
        !vm_ata253_out_task_file(program, 0x20u) ||
        !vm_ata253_wait_drq(program) ||
        !vm_ata253_put(program, 0xbau) || !vm_ata253_word(program, 0x01f0u) ||
        !vm_ata253_put(program, 0xedu) || !vm_ata253_put(program, 0x3du) ||
        !vm_ata253_word(program, 0x1357u) || !vm_ata253_put(program, 0x75u)) {
        return 0;
    }
    first_failure = program->length;
    if (!vm_ata253_put(program, 0u) || !vm_ata253_discard_words(program, 255u) ||
        !vm_ata253_wait_drq(program) ||
        !vm_ata253_put(program, 0xbau) || !vm_ata253_word(program, 0x01f0u) ||
        !vm_ata253_put(program, 0xedu) || !vm_ata253_put(program, 0x3du) ||
        !vm_ata253_word(program, 0x2468u) || !vm_ata253_put(program, 0x75u)) {
        return 0;
    }
    second_failure = program->length;
    if (!vm_ata253_put(program, 0u) || !vm_ata253_discard_words(program, 255u) ||
        !vm_ata253_marker(program, 'O', 0u)) return 0;
    failure = program->length;
    if (!vm_ata253_marker(program, 'X', 1u)) return 0;
    delta = (type_signed_16)failure - (type_signed_16)(first_failure + 1u);
    if (delta < -128 || delta > 127) return 0;
    program->bytes[first_failure] = (type_unsigned_8)(type_signed_8)delta;
    delta = (type_signed_16)failure - (type_signed_16)(second_failure + 1u);
    if (delta < -128 || delta > 127) return 0;
    program->bytes[second_failure] = (type_unsigned_8)(type_signed_8)delta;
    return 1;
}

static type_unsigned_16 vm_ata253_fat12_get(const type_unsigned_8 *fat, type_unsigned_16 cluster)
{
    type_unsigned_32 offset = cluster + cluster / 2u;
    type_unsigned_16 pair = (type_unsigned_16)(fat[offset] | ((type_unsigned_16)fat[offset + 1u] << 8u));

    return (cluster & 1u) != 0u ? pair >> 4u : pair & 0x0fffu;
}

static C_VOID vm_ata253_fat12_set(type_unsigned_8 *fat, type_unsigned_16 cluster, type_unsigned_16 value)
{
    type_unsigned_32 offset = cluster + cluster / 2u;
    type_unsigned_16 pair = (type_unsigned_16)(fat[offset] | ((type_unsigned_16)fat[offset + 1u] << 8u));

    if ((cluster & 1u) != 0u) pair = (type_unsigned_16)((pair & 0x000fu) | (value << 4u));
    else pair = (type_unsigned_16)((pair & 0xf000u) | value);
    fat[offset] = (type_unsigned_8)pair;
    fat[offset + 1u] = (type_unsigned_8)(pair >> 8u);
}

static C_INT vm_ata253_clone(const C_CHAR *source, C_CHAR path[MAX_PATH],
    type_unsigned_8 **out_image, DWORD *out_size)
{
    HANDLE input = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    LARGE_INTEGER size;
    type_unsigned_8 *image = STD_NULL;
    DWORD count;

    if (source == STD_NULL || path == STD_NULL || out_image == STD_NULL ||
        out_size == STD_NULL) return 0;
    input = CreateFileA(source, GENERIC_READ, FILE_SHARE_READ, STD_NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, STD_NULL);
    if (input == INVALID_HANDLE_VALUE || !GetFileSizeEx(input, &size) ||
        size.QuadPart <= 0 || size.QuadPart > MAXDWORD) goto fail;
    image = STD_MALLOC((STD_SIZE_T)size.QuadPart);
    if (image == STD_NULL || !ReadFile(input, image, (DWORD)size.QuadPart,
            &count, STD_NULL) || count != (DWORD)size.QuadPart ||
        GetTempPathA(MAX_PATH, path) == 0u || GetTempFileNameA(path, "n64", 0u,
            path) == 0u) goto fail;
    output = CreateFileA(path, GENERIC_WRITE, 0u, STD_NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, STD_NULL);
    if (output == INVALID_HANDLE_VALUE || !WriteFile(output, image,
            (DWORD)size.QuadPart, &count, STD_NULL) || count != (DWORD)size.QuadPart) {
        goto fail;
    }
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

static C_INT vm_ata253_zero_image(type_unsigned_8 *image, DWORD image_size,
    const C_CHAR *path)
{
    HANDLE output;
    DWORD written;

    if (image == STD_NULL || image_size == 0u || path == STD_NULL) return 0;
    STD_MEMSET(image, 0, image_size);
    output = CreateFileA(path, GENERIC_WRITE, 0u, STD_NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, STD_NULL);
    if (output == INVALID_HANDLE_VALUE || !WriteFile(output, image, image_size,
            &written, STD_NULL) || written != image_size) {
        if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
        return 0;
    }
    CloseHandle(output);
    return 1;
}

static C_INT vm_ata253_install(type_unsigned_8 *image, DWORD image_size,
    const C_CHAR *path)
{
    vm_ata253_program program;
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

    if (!vm_ata253_build_program(&program) || image == STD_NULL ||
        image_size < 512u || path == STD_NULL) return 0;
    bytes_per_sector = image[11u] | ((type_unsigned_32)image[12u] << 8u);
    sectors_per_cluster = image[13u];
    reserved_sectors = image[14u] | ((type_unsigned_32)image[15u] << 8u);
    fat_count = image[16u];
    root_entries = image[17u] | ((type_unsigned_32)image[18u] << 8u);
    sectors_per_fat = image[22u] | ((type_unsigned_32)image[23u] << 8u);
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
    clusters = (image_size - data_start) / (bytes_per_sector * sectors_per_cluster);
    for (cluster = 2u; cluster < clusters + 2u; ++cluster) {
        if (vm_ata253_fat12_get(image + reserved_sectors * bytes_per_sector,
                (type_unsigned_16)cluster) == 0u) break;
    }
    if (entry == STD_NULL || cluster >= clusters + 2u || program.length >
        bytes_per_sector * sectors_per_cluster) return 0;
    STD_MEMSET(entry, 0, 32u);
    STD_MEMCPY(entry, "ATA253  COM", 11u);
    entry[11u] = 0x20u;
    entry[26u] = (type_unsigned_8)cluster;
    entry[27u] = (type_unsigned_8)(cluster >> 8u);
    entry[28u] = (type_unsigned_8)program.length;
    entry[29u] = (type_unsigned_8)(program.length >> 8u);
    for (root = 0u; root < fat_count; ++root) {
        vm_ata253_fat12_set(image + (reserved_sectors + root * sectors_per_fat) *
            bytes_per_sector, (type_unsigned_16)cluster, 0x0fffu);
    }
    STD_MEMCPY(image + data_start + (cluster - 2u) * bytes_per_sector *
        sectors_per_cluster, program.bytes, program.length);
    output = CreateFileA(path, GENERIC_WRITE, 0u, STD_NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, STD_NULL);
    if (output == INVALID_HANDLE_VALUE || !WriteFile(output, image, image_size,
            &written, STD_NULL) || written != image_size) {
        if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
        return 0;
    }
    CloseHandle(output);
    return 1;
}

static C_INT vm_ata253_has_prompt(const core_machine_display_snapshot *snapshot)
{
    STD_SIZE_T cell;

    if (snapshot == STD_NULL || snapshot->kind != CORE_MACHINE_DISPLAY_KIND_TEXT) return 0;
    for (cell = 0u; cell + 3u < 80u * 25u; ++cell) {
        if (STD_ISALPHA(snapshot->characters[cell]) &&
            snapshot->characters[cell + 1u] == ':' &&
            snapshot->characters[cell + 2u] == '\\' &&
            snapshot->characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

static C_INT vm_ata253_run_until(vm_session *session, type_unsigned_32 limit,
    type_unsigned_8 marker)
{
    const core_machine_run_budget budget = { 128u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    type_unsigned_32 executed = 0u;

    while (executed < limit) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK) return 0;
        if (marker != 0u ? snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
                snapshot.characters[VM_ATA253_MARKER_CELL] == marker :
            vm_ata253_has_prompt(&snapshot)) return 1;
        executed += budget.instructions;
    }
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    static const type_unsigned_8 command[] = { 0x1cu, 0x2cu, 0x1cu, 0x1eu, 0x2eu,
        0x26u, 0x5au };
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    type_unsigned_8 *fdd_image = STD_NULL;
    type_unsigned_8 *hdd_image = STD_NULL;
    DWORD fdd_size = 0u;
    DWORD hdd_size = 0u;
    C_CHAR fdd_path[MAX_PATH] = {0};
    C_CHAR hdd_path[MAX_PATH] = {0};
    STD_SIZE_T index;
    C_INT passed = 0;

    if (argc != 3 || !vm_ata253_clone(argv[1], fdd_path, &fdd_image, &fdd_size) ||
        !vm_ata253_install(fdd_image, fdd_size, fdd_path) ||
        !vm_ata253_clone(argv[2], hdd_path, &hdd_image, &hdd_size) ||
        !vm_ata253_zero_image(hdd_image, hdd_size, hdd_path)) goto done;
    config.fdd_image = fdd_path;
    config.hdd_image = hdd_path;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    if (vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL ||
        !vm_ata253_run_until(session, VM_ATA253_BOOT_BUDGET, 0u)) goto done;
    for (index = 0u; index < sizeof(command); ++index) {
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
                command[index]) != TYPE_STATUS_OK) goto done;
    }
    passed = vm_ata253_run_until(session, VM_ATA253_RUN_BUDGET, 'O');

done:
    vm_session_destroy(session);
    if (fdd_path[0] != '\0') DeleteFileA(fdd_path);
    if (hdd_path[0] != '\0') DeleteFileA(hdd_path);
    STD_FREE(fdd_image);
    STD_FREE(hdd_image);
    if (!passed) return 1;
    STD_PRINTF("M5:T286:S3:ATA-NIEN:DOS:OK\n");
    STD_PRINTF("M5:T253:S3:ATA-PIO:DOS:OK\n");
    return 0;
}
