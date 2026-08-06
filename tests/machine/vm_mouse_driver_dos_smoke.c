#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session.h"
#include "core/platform/input_interface.h"
#include "vm/platform/vm_request_transport.h"

#define VM_MOUSE_DOS_BOOT_BUDGET 800000u
#define VM_MOUSE_DOS_RUN_BUDGET 400000u
#define VM_MOUSE_DOS_MARKER_CELL 1920u

typedef struct vm_mouse_dos_program {
    uint8_t bytes[512];
    uint16_t length;
} vm_mouse_dos_program;

static C_INT vm_mouse_dos_put(vm_mouse_dos_program *program, uint8_t value)
{
    if (program == STD_NULL || program->length >= sizeof(program->bytes)) return 0;
    program->bytes[program->length++] = value;
    return 1;
}

static C_INT vm_mouse_dos_word(vm_mouse_dos_program *program, uint16_t value)
{
    return vm_mouse_dos_put(program, (uint8_t)value) &&
        vm_mouse_dos_put(program, (uint8_t)(value >> 8));
}

static C_VOID vm_mouse_dos_patch_word(vm_mouse_dos_program *program,
    uint16_t position, uint16_t value)
{
    program->bytes[position] = (uint8_t)value;
    program->bytes[position + 1u] = (uint8_t)(value >> 8);
}

static C_INT vm_mouse_dos_fat12_get(const uint8_t *fat, uint16_t cluster)
{
    uint32_t offset = cluster + cluster / 2u;
    uint16_t value = (uint16_t)(fat[offset] | ((uint16_t)fat[offset + 1u] << 8));

    return (cluster & 1u) != 0u ? value >> 4 : value & 0x0fffu;
}

static C_VOID vm_mouse_dos_fat12_set(uint8_t *fat, uint16_t cluster,
    uint16_t value)
{
    uint32_t offset = cluster + cluster / 2u;
    uint16_t pair = (uint16_t)(fat[offset] | ((uint16_t)fat[offset + 1u] << 8));

    if ((cluster & 1u) != 0u) pair = (uint16_t)((pair & 0x000fu) | (value << 4));
    else pair = (uint16_t)((pair & 0xf000u) | value);
    fat[offset] = (uint8_t)pair;
    fat[offset + 1u] = (uint8_t)(pair >> 8);
}

static C_INT vm_mouse_dos_copy_image(const C_CHAR *source, C_CHAR path[MAX_PATH],
    uint8_t **out_image, DWORD *out_size)
{
    HANDLE input = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    LARGE_INTEGER size;
    uint8_t *image = STD_NULL;
    DWORD count;

    if (source == STD_NULL || out_image == STD_NULL || out_size == STD_NULL) return 0;
    input = CreateFileA(source, GENERIC_READ, FILE_SHARE_READ, STD_NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, STD_NULL);
    if (input == INVALID_HANDLE_VALUE || !GetFileSizeEx(input, &size) ||
        size.QuadPart <= 0 || size.QuadPart > MAXDWORD) goto fail;
    image = STD_MALLOC((STD_SIZE_T)size.QuadPart);
    if (image == STD_NULL || !ReadFile(input, image, (DWORD)size.QuadPart,
            &count, STD_NULL) || count != (DWORD)size.QuadPart) goto fail;
    if (GetTempPathA(MAX_PATH, path) == 0u ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) goto fail;
    output = CreateFileA(path, GENERIC_WRITE, 0u, STD_NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, STD_NULL);
    if (output == INVALID_HANDLE_VALUE || !WriteFile(output, image,
            (DWORD)size.QuadPart, &count, STD_NULL) ||
        count != (DWORD)size.QuadPart) goto fail;
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

static C_INT vm_mouse_dos_build_program(vm_mouse_dos_program *program,
    uint16_t *out_bytes_offset)
{
    uint16_t handler_patch;
    uint16_t count_patch[4];
    uint16_t bytes_patch;
    uint16_t wait_setup;
    uint16_t wait_packet;
    uint16_t handler;
    uint16_t count;
    uint16_t bytes;
    uint16_t index;

    if (program == STD_NULL || out_bytes_offset == STD_NULL) return 0;
    STD_MEMSET(program, 0, sizeof(*program));
    /* COM entry: install ordinary IRQ12 handler, unmask PIC, then issue
     * AUX reset, identify, and enable-reporting requests. */
    if (!vm_mouse_dos_put(program, 0x0eu) || !vm_mouse_dos_put(program, 0x1fu) ||
        !vm_mouse_dos_put(program, 0xfau) || !vm_mouse_dos_put(program, 0x31u) ||
        !vm_mouse_dos_put(program, 0xc0u) || !vm_mouse_dos_put(program, 0x8eu) ||
        !vm_mouse_dos_put(program, 0xc0u) || !vm_mouse_dos_put(program, 0x26u) ||
        !vm_mouse_dos_put(program, 0xc7u) || !vm_mouse_dos_put(program, 0x06u) ||
        !vm_mouse_dos_word(program, 0x01d0u)) return 0;
    handler_patch = program->length;
    if (!vm_mouse_dos_word(program, 0u) || !vm_mouse_dos_put(program, 0x8cu) ||
        !vm_mouse_dos_put(program, 0xc8u) || !vm_mouse_dos_put(program, 0x26u) ||
        !vm_mouse_dos_put(program, 0xa3u) || !vm_mouse_dos_word(program, 0x01d2u) ||
        !vm_mouse_dos_put(program, 0xb0u) || !vm_mouse_dos_put(program, 0xfbu) ||
        !vm_mouse_dos_put(program, 0xe6u) || !vm_mouse_dos_put(program, 0x21u) ||
        !vm_mouse_dos_put(program, 0xb0u) || !vm_mouse_dos_put(program, 0xefu) ||
        !vm_mouse_dos_put(program, 0xe6u) || !vm_mouse_dos_put(program, 0xa1u) ||
        !vm_mouse_dos_put(program, 0xfbu)) return 0;
    for (index = 0u; index < 3u; ++index) {
        static const uint8_t command[] = { 0xffu, 0xf2u, 0xf4u };
        if (!vm_mouse_dos_put(program, 0xb0u) || !vm_mouse_dos_put(program, 0xd4u) ||
            !vm_mouse_dos_put(program, 0xe6u) || !vm_mouse_dos_put(program, 0x64u) ||
            !vm_mouse_dos_put(program, 0xb0u) ||
            !vm_mouse_dos_put(program, command[index]) ||
            !vm_mouse_dos_put(program, 0xe6u) || !vm_mouse_dos_put(program, 0x60u)) {
            return 0;
        }
    }
    if (!vm_mouse_dos_put(program, 0xb8u) || !vm_mouse_dos_word(program, 0xb800u) ||
        !vm_mouse_dos_put(program, 0x8eu) || !vm_mouse_dos_put(program, 0xc0u) ||
        !vm_mouse_dos_put(program, 0x26u) || !vm_mouse_dos_put(program, 0xc7u) ||
        !vm_mouse_dos_put(program, 0x06u) || !vm_mouse_dos_word(program, 0x0f00u) ||
        !vm_mouse_dos_word(program, 0x0752u)) return 0;

    wait_setup = program->length;
    if (!vm_mouse_dos_put(program, 0x83u) || !vm_mouse_dos_put(program, 0x3eu)) return 0;
    count_patch[0] = program->length;
    if (!vm_mouse_dos_word(program, 0u) || !vm_mouse_dos_put(program, 0x06u) ||
        !vm_mouse_dos_put(program, 0x72u) || !vm_mouse_dos_put(program,
            (uint8_t)(wait_setup - program->length - 1u))) return 0;
    /* The guest has consumed reset/identify/enable replies and is now ready
     * for one host-originated relative packet. */
    if (!vm_mouse_dos_put(program, 0xb8u) || !vm_mouse_dos_word(program, 0xb800u) ||
        !vm_mouse_dos_put(program, 0x8eu) || !vm_mouse_dos_put(program, 0xc0u) ||
        !vm_mouse_dos_put(program, 0x26u) || !vm_mouse_dos_put(program, 0xc7u) ||
        !vm_mouse_dos_put(program, 0x06u) || !vm_mouse_dos_word(program, 0x0f00u) ||
        !vm_mouse_dos_word(program, 0x0753u)) return 0;
    wait_packet = program->length;
    if (!vm_mouse_dos_put(program, 0x83u) || !vm_mouse_dos_put(program, 0x3eu)) return 0;
    count_patch[1] = program->length;
    if (!vm_mouse_dos_word(program, 0u) || !vm_mouse_dos_put(program, 0x09u) ||
        !vm_mouse_dos_put(program, 0x72u) || !vm_mouse_dos_put(program,
            (uint8_t)(wait_packet - program->length - 1u))) return 0;
    if (!vm_mouse_dos_put(program, 0xb8u) || !vm_mouse_dos_word(program, 0xb800u) ||
        !vm_mouse_dos_put(program, 0x8eu) || !vm_mouse_dos_put(program, 0xc0u) ||
        !vm_mouse_dos_put(program, 0x26u) || !vm_mouse_dos_put(program, 0xc7u) ||
        !vm_mouse_dos_put(program, 0x06u) || !vm_mouse_dos_word(program, 0x0f00u) ||
        !vm_mouse_dos_word(program, 0x074fu) || !vm_mouse_dos_put(program, 0xb8u) ||
        !vm_mouse_dos_word(program, 0x4c00u) || !vm_mouse_dos_put(program, 0xcdu) ||
        !vm_mouse_dos_put(program, 0x21u)) return 0;
    handler = (uint16_t)(0x0100u + program->length);
    vm_mouse_dos_patch_word(program, handler_patch, handler);

    if (!vm_mouse_dos_put(program, 0x50u) || !vm_mouse_dos_put(program, 0x53u) ||
        !vm_mouse_dos_put(program, 0x1eu) || !vm_mouse_dos_put(program, 0x0eu) ||
        !vm_mouse_dos_put(program, 0x1fu) || !vm_mouse_dos_put(program, 0xe4u) ||
        !vm_mouse_dos_put(program, 0x60u) || !vm_mouse_dos_put(program, 0x8bu) ||
        !vm_mouse_dos_put(program, 0x1eu)) return 0;
    count_patch[2] = program->length;
    if (!vm_mouse_dos_word(program, 0u) || !vm_mouse_dos_put(program, 0x88u) ||
        !vm_mouse_dos_put(program, 0x87u)) return 0;
    bytes_patch = program->length;
    if (!vm_mouse_dos_word(program, 0u) || !vm_mouse_dos_put(program, 0x43u) ||
        !vm_mouse_dos_put(program, 0x89u) || !vm_mouse_dos_put(program, 0x1eu)) return 0;
    count_patch[3] = program->length;
    if (!vm_mouse_dos_word(program, 0u) || !vm_mouse_dos_put(program, 0xb0u) ||
        !vm_mouse_dos_put(program, 0x20u) || !vm_mouse_dos_put(program, 0xe6u) ||
        !vm_mouse_dos_put(program, 0xa0u) || !vm_mouse_dos_put(program, 0xe6u) ||
        !vm_mouse_dos_put(program, 0x20u) || !vm_mouse_dos_put(program, 0x1fu) ||
        !vm_mouse_dos_put(program, 0x5bu) || !vm_mouse_dos_put(program, 0x58u) ||
        !vm_mouse_dos_put(program, 0xcfu)) return 0;
    count = (uint16_t)(0x0100u + program->length);
    if (!vm_mouse_dos_word(program, 0u)) return 0;
    bytes = (uint16_t)(0x0100u + program->length);
    for (index = 0u; index < 9u; ++index) if (!vm_mouse_dos_put(program, 0u)) return 0;
    for (index = 0u; index < 4u; ++index) vm_mouse_dos_patch_word(program,
        count_patch[index], count);
    vm_mouse_dos_patch_word(program, bytes_patch, bytes);
    *out_bytes_offset = bytes;
    return 1;
}

static C_INT vm_mouse_dos_install_program(uint8_t *image, DWORD image_size,
    const C_CHAR *path, uint16_t *out_bytes_offset)
{
    vm_mouse_dos_program program;
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

    if (!vm_mouse_dos_build_program(&program, out_bytes_offset) ||
        image == STD_NULL || image_size < 512u || path == STD_NULL) return 0;
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
        if (candidate[0] == 0u || candidate[0] == 0xe5u) { entry = candidate; break; }
    }
    if (entry == STD_NULL) return 0;
    clusters = (image_size - data_start) / (bytes_per_sector * sectors_per_cluster);
    for (cluster = 2u; cluster < clusters + 2u; ++cluster) {
        if (vm_mouse_dos_fat12_get(image + reserved_sectors * bytes_per_sector,
                (uint16_t)cluster) == 0) break;
    }
    if (cluster >= clusters + 2u || program.length >
        bytes_per_sector * sectors_per_cluster) return 0;
    STD_MEMSET(entry, 0, 32u);
    STD_MEMCPY(entry, "MOUSE241COM", 11u);
    entry[11u] = 0x20u;
    entry[26u] = (uint8_t)cluster;
    entry[27u] = (uint8_t)(cluster >> 8);
    entry[28u] = (uint8_t)program.length;
    entry[29u] = (uint8_t)(program.length >> 8);
    for (root = 0u; root < fat_count; ++root) {
        vm_mouse_dos_fat12_set(image + (reserved_sectors + root * sectors_per_fat) *
            bytes_per_sector, (uint16_t)cluster, 0x0fffu);
    }
    STD_MEMCPY(image + data_start + (cluster - 2u) * bytes_per_sector *
        sectors_per_cluster, program.bytes, program.length);
    output = CreateFileA(path, GENERIC_WRITE, 0u, STD_NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, STD_NULL);
    if (output == INVALID_HANDLE_VALUE) return 0;
    if (!WriteFile(output, image, image_size, &written, STD_NULL) || written != image_size) {
        CloseHandle(output);
        return 0;
    }
    CloseHandle(output);
    return 1;
}

static C_INT vm_mouse_dos_has_prompt(const core_machine_display_snapshot *snapshot)
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

static C_INT vm_mouse_dos_run_until(vm_session *session, uint32_t limit,
    uint8_t wanted)
{
    core_machine_run_budget budget = { 128u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    uint32_t executed = 0u;

    while (executed < limit) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK) return 0;
        if (wanted == 0u ? vm_mouse_dos_has_prompt(&snapshot) :
            snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
            snapshot.characters[VM_MOUSE_DOS_MARKER_CELL] == wanted) return 1;
        executed += budget.instructions;
    }
    return 0;
}

static C_INT vm_mouse_dos_run_until_packet(vm_session *session,
    uint32_t buffer_address, const uint8_t expected[9])
{
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    uint8_t actual[9];
    uint32_t executed;

    if (session == STD_NULL || expected == STD_NULL) return 0;
    for (executed = 0u; executed < VM_MOUSE_DOS_RUN_BUDGET; ++executed) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_memory_read(session->core_machine, buffer_address,
                actual, sizeof(actual)) != TYPE_STATUS_OK) return 0;
        if (STD_MEMCMP(actual, expected, sizeof(actual)) == 0) return 1;
    }
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    static const uint8_t command[] = { 0x32u, 0x18u, 0x16u, 0x1fu, 0x12u,
        0x03u, 0x05u, 0x02u, 0x1cu };
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    uint8_t *image = STD_NULL;
    DWORD image_size = 0u;
    C_CHAR path[MAX_PATH] = {0};
    core_machine_observation observation;
    core_machine_display_snapshot snapshot;
    static const uint8_t expected[] = { 0xfau, 0xaau, 0x00u, 0xfau, 0x00u,
        0xfau, 0x29u, 0x05u, 0xfdu };
    uint16_t bytes_offset = 0u;
    uint32_t bytes_address;
    STD_SIZE_T index;
    C_INT passed = 0;
    C_INT stage = 0;

    stage = 1;
    if (argc != 2 || !vm_mouse_dos_copy_image(argv[1], path, &image, &image_size) ||
        !vm_mouse_dos_install_program(image, image_size, path, &bytes_offset)) goto done;
    stage = 2;
    config.fdd_image = path;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    if (vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL) goto done;
    stage = 3;
    if (!vm_mouse_dos_run_until(session, VM_MOUSE_DOS_BOOT_BUDGET, 0u)) goto done;
    stage = 4;
    for (index = 0u; index < sizeof(command); ++index) {
        if (core_machine_keyboard_submit_scan_code(session->core_machine,
                command[index]) != TYPE_STATUS_OK) goto done;
    }
    if (!vm_mouse_dos_run_until(session, VM_MOUSE_DOS_RUN_BUDGET, 'S')) goto done;
    if (core_machine_capture_observation(session->core_machine, &observation) !=
        TYPE_STATUS_OK) goto done;
    bytes_address = ((uint32_t)observation.cpu.cs << 4) + bytes_offset;
    stage = 5;
    {
        core_platform_input_event event = {0};
        event.kind = CORE_PLATFORM_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 5;
        event.data.relative_mouse.delta_y = 3;
        event.data.relative_mouse.buttons = 0x01u;
        if (core_platform_input_source_submit(&session->input_source, &event) !=
            TYPE_STATUS_OK) goto done;
    }
    vm_platform_request_transport_observe_execution_boundary(&session->request_transport);
    if (!vm_mouse_dos_run_until_packet(session, bytes_address, expected) ||
        core_machine_capture_display_snapshot(session->core_machine, &snapshot) !=
            TYPE_STATUS_OK || snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT ||
        snapshot.characters[VM_MOUSE_DOS_MARKER_CELL] == 'O') goto done;
    stage = 6;
    passed = vm_mouse_dos_run_until(session, VM_MOUSE_DOS_RUN_BUDGET, 'O');
    if (!passed && session != STD_NULL) {
        core_machine_display_snapshot snapshot;

        if (core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) == TYPE_STATUS_OK &&
            snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT) {
            STD_FPRINTF(STD_STDERR, "M5:T241:MOUSE-DRIVER:MARKER=%02X\n",
                snapshot.characters[VM_MOUSE_DOS_MARKER_CELL]);
        }
    }

done:
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    STD_FREE(image);
    if (!passed) {
        STD_FPRINTF(STD_STDERR, "M5:T241:MOUSE-DRIVER:DOS:FAIL:STAGE=%d\n", stage);
        return 1;
    }
    STD_PRINTF("M5:T241:S2:MOUSE-DRIVER:DOS:OK\n");
    return 0;
}
