#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "core/platform/input_interface.h"
#include "vm/platform/vm_request_transport.h"

#define VM_MOUSE_DOS_BOOT_BUDGET 800000u
#define VM_MOUSE_DOS_RUN_BUDGET 400000u
#define VM_MOUSE_DOS_MARKER_CELL 1920u

typedef struct vm_mouse_dos_program {
    type_unsigned_8 bytes[512];
    type_unsigned_16 length;
} vm_mouse_dos_program;

static C_INT vm_mouse_dos_put(vm_mouse_dos_program *program, type_unsigned_8 value)
{
    if (program == STD_NULL || program->length >= sizeof(program->bytes)) return 0;
    program->bytes[program->length++] = value;
    return 1;
}

static C_INT vm_mouse_dos_word(vm_mouse_dos_program *program, type_unsigned_16 value)
{
    return vm_mouse_dos_put(program, (type_unsigned_8)value) &&
        vm_mouse_dos_put(program, (type_unsigned_8)(value >> 8));
}

static C_VOID vm_mouse_dos_patch_word(vm_mouse_dos_program *program,
    type_unsigned_16 position, type_unsigned_16 value)
{
    program->bytes[position] = (type_unsigned_8)value;
    program->bytes[position + 1u] = (type_unsigned_8)(value >> 8);
}

static C_INT vm_mouse_dos_fat12_get(const type_unsigned_8 *fat, type_unsigned_16 cluster)
{
    type_unsigned_32 offset = cluster + cluster / 2u;
    type_unsigned_16 value = (type_unsigned_16)(fat[offset] | ((type_unsigned_16)fat[offset + 1u] << 8));

    return (cluster & 1u) != 0u ? value >> 4 : value & 0x0fffu;
}

static C_VOID vm_mouse_dos_fat12_set(type_unsigned_8 *fat, type_unsigned_16 cluster,
    type_unsigned_16 value)
{
    type_unsigned_32 offset = cluster + cluster / 2u;
    type_unsigned_16 pair = (type_unsigned_16)(fat[offset] | ((type_unsigned_16)fat[offset + 1u] << 8));

    if ((cluster & 1u) != 0u) pair = (type_unsigned_16)((pair & 0x000fu) | (value << 4));
    else pair = (type_unsigned_16)((pair & 0xf000u) | value);
    fat[offset] = (type_unsigned_8)pair;
    fat[offset + 1u] = (type_unsigned_8)(pair >> 8);
}

static C_INT vm_mouse_dos_copy_image(const C_CHAR *source, C_CHAR path[MAX_PATH],
    type_unsigned_8 **out_image, DWORD *out_size)
{
    HANDLE input = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    LARGE_INTEGER size;
    type_unsigned_8 *image = STD_NULL;
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
    type_unsigned_16 *out_bytes_offset)
{
    type_unsigned_16 handler_patch;
    type_unsigned_16 count_patch[4];
    type_unsigned_16 bytes_patch;
    type_unsigned_16 wait_setup;
    type_unsigned_16 wait_packet;
    type_unsigned_16 handler;
    type_unsigned_16 count;
    type_unsigned_16 bytes;
    type_unsigned_16 index;

    if (program == STD_NULL || out_bytes_offset == STD_NULL) return 0;
    STD_MEMSET(program, 0, sizeof(*program));
    /* COM entry: install ordinary IRQ12 handler, unmask PIC, then issue
     * AUX reset, identify, reporting, configuration, and status requests. */
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
    for (index = 0u; index < 8u; ++index) {
        static const type_unsigned_8 command[] = {
            0xffu, 0xf2u, 0xf4u, 0xf3u, 200u, 0xe8u, 0x03u, 0xe9u
        };
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
    if (!vm_mouse_dos_word(program, 0u) || !vm_mouse_dos_put(program, 0x0eu) ||
        !vm_mouse_dos_put(program, 0x72u) || !vm_mouse_dos_put(program,
            (type_unsigned_8)(wait_setup - program->length - 1u))) return 0;
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
    if (!vm_mouse_dos_word(program, 0u) || !vm_mouse_dos_put(program, 0x11u) ||
        !vm_mouse_dos_put(program, 0x72u) || !vm_mouse_dos_put(program,
            (type_unsigned_8)(wait_packet - program->length - 1u))) return 0;
    if (!vm_mouse_dos_put(program, 0xb8u) || !vm_mouse_dos_word(program, 0xb800u) ||
        !vm_mouse_dos_put(program, 0x8eu) || !vm_mouse_dos_put(program, 0xc0u) ||
        !vm_mouse_dos_put(program, 0x26u) || !vm_mouse_dos_put(program, 0xc7u) ||
        !vm_mouse_dos_put(program, 0x06u) || !vm_mouse_dos_word(program, 0x0f00u) ||
        !vm_mouse_dos_word(program, 0x074fu) || !vm_mouse_dos_put(program, 0xb8u) ||
        !vm_mouse_dos_word(program, 0x4c00u) || !vm_mouse_dos_put(program, 0xcdu) ||
        !vm_mouse_dos_put(program, 0x21u)) return 0;
    handler = (type_unsigned_16)(0x0100u + program->length);
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
    count = (type_unsigned_16)(0x0100u + program->length);
    if (!vm_mouse_dos_word(program, 0u)) return 0;
    bytes = (type_unsigned_16)(0x0100u + program->length);
    for (index = 0u; index < 17u; ++index) if (!vm_mouse_dos_put(program, 0u)) return 0;
    for (index = 0u; index < 4u; ++index) vm_mouse_dos_patch_word(program,
        count_patch[index], count);
    vm_mouse_dos_patch_word(program, bytes_patch, bytes);
    *out_bytes_offset = bytes;
    return 1;
}

static C_INT vm_mouse_dos_install_program(type_unsigned_8 *image, DWORD image_size,
    const C_CHAR *path, type_unsigned_16 *out_bytes_offset)
{
    vm_mouse_dos_program program;
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

    if (!vm_mouse_dos_build_program(&program, out_bytes_offset) ||
        image == STD_NULL || image_size < 512u || path == STD_NULL) return 0;
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
        if (candidate[0] == 0u || candidate[0] == 0xe5u) { entry = candidate; break; }
    }
    if (entry == STD_NULL) return 0;
    clusters = (image_size - data_start) / (bytes_per_sector * sectors_per_cluster);
    for (cluster = 2u; cluster < clusters + 2u; ++cluster) {
        if (vm_mouse_dos_fat12_get(image + reserved_sectors * bytes_per_sector,
                (type_unsigned_16)cluster) == 0) break;
    }
    if (cluster >= clusters + 2u || program.length >
        bytes_per_sector * sectors_per_cluster) return 0;
    STD_MEMSET(entry, 0, 32u);
    STD_MEMCPY(entry, "MOUSE241COM", 11u);
    entry[11u] = 0x20u;
    entry[26u] = (type_unsigned_8)cluster;
    entry[27u] = (type_unsigned_8)(cluster >> 8);
    entry[28u] = (type_unsigned_8)program.length;
    entry[29u] = (type_unsigned_8)(program.length >> 8);
    for (root = 0u; root < fat_count; ++root) {
        vm_mouse_dos_fat12_set(image + (reserved_sectors + root * sectors_per_fat) *
            bytes_per_sector, (type_unsigned_16)cluster, 0x0fffu);
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

static C_INT vm_mouse_dos_run_until(vm_session *session, type_unsigned_32 limit,
    type_unsigned_8 wanted)
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
        if (wanted == 0u ? vm_mouse_dos_has_prompt(&snapshot) :
            snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
            snapshot.characters[VM_MOUSE_DOS_MARKER_CELL] == wanted) return 1;
        executed += budget.instructions;
    }
    return 0;
}

static C_INT vm_mouse_dos_run_until_packet(vm_session *session,
    type_unsigned_32 buffer_address, const type_unsigned_8 expected[17])
{
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    type_unsigned_8 actual[17];
    type_unsigned_32 executed;

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
    static const type_unsigned_8 command[] = { 0x3au, 0x44u, 0x3cu, 0x1bu, 0x24u,
        0x1eu, 0x25u, 0x16u, 0x5au };
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    type_unsigned_8 *image = STD_NULL;
    DWORD image_size = 0u;
    C_CHAR path[MAX_PATH] = {0};
    core_machine_observation observation;
    core_machine_display_snapshot snapshot;
    static const type_unsigned_8 expected[] = {
        0xfau, 0xaau, 0x00u, 0xfau, 0x00u, 0xfau,
        0xfau, 0xfau, 0xfau, 0xfau, 0xfau, 0x20u, 0x03u, 200u,
        0x29u, 0x05u, 0xfdu
    };
    type_unsigned_16 bytes_offset = 0u;
    type_unsigned_32 bytes_address;
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
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
                command[index]) != TYPE_STATUS_OK) goto done;
    }
    if (!vm_mouse_dos_run_until(session, VM_MOUSE_DOS_RUN_BUDGET, 'S')) goto done;
    if (core_machine_capture_observation(session->core_machine, &observation) !=
        TYPE_STATUS_OK) goto done;
    bytes_address = ((type_unsigned_32)observation.cpu.cs << 4) + bytes_offset;
    stage = 5;
    {
        core_platform_input_event event = {0};
        event.kind = CORE_PLATFORM_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 5;
        event.data.relative_mouse.delta_y = 3;
        event.data.relative_mouse.buttons = 0x01u;
        if (vm_session_submit_host_input(session, &event) !=
            TYPE_STATUS_OK) goto done;
    }
    vm_platform_request_transport_observe_execution_boundary(session->request_transport);
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
    STD_PRINTF("M5:T267:S3:AUX:DOS:OK\n");
    return 0;
}
