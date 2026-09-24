#include "lib/types/types_interface.h"
#include <ctype.h>
#include <stdio.h>

#include <windows.h>

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/waiting.h"
#include "app-nxvm/devices/guest_input_interface.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define VM_MOUSE_DOS_BOOT_BUDGET 800000u
#define VM_MOUSE_DOS_RUN_BUDGET 400000u
#define VM_MOUSE_DOS_MARKER_CELL 1920u

typedef struct vm_mouse_dos_program {
    lib_u8 bytes[512];
    lib_u16 length;
} vm_mouse_dos_program;

static lib_i32 vm_mouse_dos_put(vm_mouse_dos_program *program, lib_u8 value)
{
    if (program == LIB_NULL || program->length >= sizeof(program->bytes)) return 0;
    program->bytes[program->length++] = value;
    return 1;
}

static lib_i32 vm_mouse_dos_word(vm_mouse_dos_program *program, lib_u16 value)
{
    return vm_mouse_dos_put(program, (lib_u8)value) &&
        vm_mouse_dos_put(program, (lib_u8)(value >> 8));
}

static void vm_mouse_dos_patch_word(vm_mouse_dos_program *program,
    lib_u16 position, lib_u16 value)
{
    program->bytes[position] = (lib_u8)value;
    program->bytes[position + 1u] = (lib_u8)(value >> 8);
}

static lib_i32 vm_mouse_dos_fat12_get(const lib_u8 *fat, lib_u16 cluster)
{
    lib_u32 offset = cluster + cluster / 2u;
    lib_u16 value = (lib_u16)(fat[offset] | ((lib_u16)fat[offset + 1u] << 8));

    return (cluster & 1u) != 0u ? value >> 4 : value & 0x0fffu;
}

static void vm_mouse_dos_fat12_set(lib_u8 *fat, lib_u16 cluster,
    lib_u16 value)
{
    lib_u32 offset = cluster + cluster / 2u;
    lib_u16 pair = (lib_u16)(fat[offset] | ((lib_u16)fat[offset + 1u] << 8));

    if ((cluster & 1u) != 0u) pair = (lib_u16)((pair & 0x000fu) | (value << 4));
    else pair = (lib_u16)((pair & 0xf000u) | value);
    fat[offset] = (lib_u8)pair;
    fat[offset + 1u] = (lib_u8)(pair >> 8);
}

static lib_i32 vm_mouse_dos_build_program(vm_mouse_dos_program *program,
    lib_u16 *out_bytes_offset)
{
    lib_u16 handler_patch;
    lib_u16 count_patch[4];
    lib_u16 bytes_patch;
    lib_u16 wait_setup;
    lib_u16 wait_packet;
    lib_u16 handler;
    lib_u16 count;
    lib_u16 bytes;
    lib_u16 index;

    if (program == LIB_NULL || out_bytes_offset == LIB_NULL) return 0;
    lib_memory_set(program, 0, sizeof(*program));
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
        static const lib_u8 command[] = {
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
            (lib_u8)(wait_setup - program->length - 1u))) return 0;
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
            (lib_u8)(wait_packet - program->length - 1u))) return 0;
    if (!vm_mouse_dos_put(program, 0xb8u) || !vm_mouse_dos_word(program, 0xb800u) ||
        !vm_mouse_dos_put(program, 0x8eu) || !vm_mouse_dos_put(program, 0xc0u) ||
        !vm_mouse_dos_put(program, 0x26u) || !vm_mouse_dos_put(program, 0xc7u) ||
        !vm_mouse_dos_put(program, 0x06u) || !vm_mouse_dos_word(program, 0x0f00u) ||
        !vm_mouse_dos_word(program, 0x074fu) || !vm_mouse_dos_put(program, 0xb8u) ||
        !vm_mouse_dos_word(program, 0x4c00u) || !vm_mouse_dos_put(program, 0xcdu) ||
        !vm_mouse_dos_put(program, 0x21u)) return 0;
    handler = (lib_u16)(0x0100u + program->length);
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
    count = (lib_u16)(0x0100u + program->length);
    if (!vm_mouse_dos_word(program, 0u)) return 0;
    bytes = (lib_u16)(0x0100u + program->length);
    for (index = 0u; index < 17u; ++index) if (!vm_mouse_dos_put(program, 0u)) return 0;
    for (index = 0u; index < 4u; ++index) vm_mouse_dos_patch_word(program,
        count_patch[index], count);
    vm_mouse_dos_patch_word(program, bytes_patch, bytes);
    *out_bytes_offset = bytes;
    return 1;
}

static lib_i32 vm_mouse_dos_install_program(lib_u8 *image, DWORD image_size,
    lib_u16 *out_bytes_offset)
{
    vm_mouse_dos_program program;
    lib_u32 bytes_per_sector;
    lib_u32 sectors_per_cluster;
    lib_u32 reserved_sectors;
    lib_u32 fat_count;
    lib_u32 root_entries;
    lib_u32 sectors_per_fat;
    lib_u32 root_start;
    lib_u32 root_bytes;
    lib_u32 data_start;
    lib_u32 clusters;
    lib_u32 cluster;
    lib_u32 root;
    lib_u8 *entry = LIB_NULL;
    if (!vm_mouse_dos_build_program(&program, out_bytes_offset) ||
        image == LIB_NULL || image_size < 512u) return 0;
    bytes_per_sector = image[11u] | ((lib_u32)image[12u] << 8);
    sectors_per_cluster = image[13u];
    reserved_sectors = image[14u] | ((lib_u32)image[15u] << 8);
    fat_count = image[16u];
    root_entries = image[17u] | ((lib_u32)image[18u] << 8);
    sectors_per_fat = image[22u] | ((lib_u32)image[23u] << 8);
    if (bytes_per_sector == 0u || sectors_per_cluster == 0u || fat_count == 0u ||
        sectors_per_fat == 0u) return 0;
    root_start = (reserved_sectors + fat_count * sectors_per_fat) * bytes_per_sector;
    root_bytes = root_entries * 32u;
    data_start = root_start + ((root_bytes + bytes_per_sector - 1u) /
        bytes_per_sector) * bytes_per_sector;
    if (data_start >= image_size || root_start + root_bytes > image_size) return 0;
    for (root = 0u; root < root_entries; ++root) {
        lib_u8 *candidate = image + root_start + root * 32u;
        if (candidate[0] == 0u || candidate[0] == 0xe5u) { entry = candidate; break; }
    }
    if (entry == LIB_NULL) return 0;
    clusters = (image_size - data_start) / (bytes_per_sector * sectors_per_cluster);
    for (cluster = 2u; cluster < clusters + 2u; ++cluster) {
        if (vm_mouse_dos_fat12_get(image + reserved_sectors * bytes_per_sector,
                (lib_u16)cluster) == 0) break;
    }
    if (cluster >= clusters + 2u || program.length >
        bytes_per_sector * sectors_per_cluster) return 0;
    lib_memory_set(entry, 0, 32u);
    lib_memory_copy(entry, "MOUSE241COM", 11u);
    entry[11u] = 0x20u;
    entry[26u] = (lib_u8)cluster;
    entry[27u] = (lib_u8)(cluster >> 8);
    entry[28u] = (lib_u8)program.length;
    entry[29u] = (lib_u8)(program.length >> 8);
    for (root = 0u; root < fat_count; ++root) {
        vm_mouse_dos_fat12_set(image + (reserved_sectors + root * sectors_per_fat) *
            bytes_per_sector, (lib_u16)cluster, 0x0fffu);
    }
    lib_memory_copy(image + data_start + (cluster - 2u) * bytes_per_sector *
        sectors_per_cluster, program.bytes, program.length);
    return 1;
}

static lib_status vm_mouse_dos_install_on_overlay(
    integration_ini_session *ini_session, void *opaque)
{
    lib_u8 *image = LIB_NULL;
    lib_size image_size = 0u;
    lib_u16 *bytes_offset = (lib_u16 *)opaque;
    lib_i32 installed;

    if (ini_session == LIB_NULL || bytes_offset == LIB_NULL ||
        integration_ini_session_overlay_read(ini_session, VM_MACHINE_MEDIA_FDD_ID,
            (void **)&image, &image_size) != LIB_STATUS_OK || image_size > MAXDWORD) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    installed = vm_mouse_dos_install_program(image, (DWORD)image_size, bytes_offset) &&
        integration_ini_session_overlay_write(ini_session, VM_MACHINE_MEDIA_FDD_ID,
            image, image_size) == LIB_STATUS_OK;
    lib_release(image);
    return installed ? LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}

static lib_i32 vm_mouse_dos_has_prompt(const core_machine_display_snapshot *snapshot)
{
    lib_size cell;

    if (snapshot == LIB_NULL || snapshot->kind != CORE_MACHINE_DISPLAY_KIND_TEXT) return 0;
    for (cell = 0u; cell + 3u < 80u * 25u; ++cell) {
        if (isalpha(snapshot->characters[cell]) &&
            snapshot->characters[cell + 1u] == ':' &&
            snapshot->characters[cell + 2u] == '\\' &&
            snapshot->characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

static lib_i32 vm_mouse_dos_run_until(vm_machine *session, lib_u32 limit,
    lib_u8 wanted)
{
    core_machine_run_budget budget = { 128u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    lib_u32 executed = 0u;

    while (executed < limit) {
        if (core_machine_run(session->core_machine, budget, &result) != LIB_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != LIB_STATUS_OK) return 0;
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            lib_i32 advanced = 0;

            if (vm_machine_waiting_advance(session, &result, &advanced) != LIB_STATUS_OK ||
                !advanced) return 0;
        }
        if (wanted == 0u ? vm_mouse_dos_has_prompt(&snapshot) :
            snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
            snapshot.characters[VM_MOUSE_DOS_MARKER_CELL] == wanted) return 1;
        executed += budget.instructions;
    }
    return 0;
}

static lib_i32 vm_mouse_dos_run_until_packet(vm_machine *session,
    lib_u32 buffer_address, const lib_u8 expected[17])
{
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    lib_u8 actual[17];
    lib_u32 executed;

    if (session == LIB_NULL || expected == LIB_NULL) return 0;
    for (executed = 0u; executed < VM_MOUSE_DOS_RUN_BUDGET; ++executed) {
        if (core_machine_run(session->core_machine, budget, &result) != LIB_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_memory_read(session->core_machine, buffer_address,
                actual, sizeof(actual)) != LIB_STATUS_OK) return 0;
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            lib_i32 advanced = 0;

            if (vm_machine_waiting_advance(session, &result, &advanced) != LIB_STATUS_OK ||
                !advanced) return 0;
        }
        if (lib_memory_compare(actual, expected, sizeof(actual)) == 0) return 1;
    }
    return 0;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    static const lib_u8 command[] = { 0x3au, 0x44u, 0x3cu, 0x1bu, 0x24u,
        0x1eu, 0x25u, 0x16u, 0x5au };
    integration_ini_session ini_session;
    vm_machine *session;
    core_machine_observation observation;
    core_machine_display_snapshot snapshot;
    static const lib_u8 expected[] = {
        0xfau, 0xaau, 0x00u, 0xfau, 0x00u, 0xfau,
        0xfau, 0xfau, 0xfau, 0xfau, 0xfau, 0x20u, 0x03u, 200u,
        0x29u, 0x05u, 0xfdu
    };
    lib_u16 bytes_offset = 0u;
    lib_u32 bytes_address;
    lib_size index;
    lib_i32 passed = 0;
    lib_i32 stage = 0;

    stage = 1;
    if (argc != 3 || integration_ini_session_open_with_overlay_transform(argv[1], argv[2],
            vm_mouse_dos_install_on_overlay, &bytes_offset, &ini_session) !=
        LIB_STATUS_OK) return 77;
    stage = 2;
    session = ini_session.session;
    stage = 3;
    if (!vm_mouse_dos_run_until(session, VM_MOUSE_DOS_BOOT_BUDGET, 0u)) goto done;
    stage = 4;
    for (index = 0u; index < sizeof(command); ++index) {
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
                command[index]) != LIB_STATUS_OK) goto done;
    }
    if (!vm_mouse_dos_run_until(session, VM_MOUSE_DOS_RUN_BUDGET, 'S')) goto done;
    if (core_machine_capture_observation(session->core_machine, &observation) !=
        LIB_STATUS_OK) goto done;
    bytes_address = ((lib_u32)observation.cpu.cs << 4) + bytes_offset;
    stage = 5;
    {
        core_machine_guest_input_event event = {0};
        event.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 5;
        event.data.relative_mouse.delta_y = 3;
        event.data.relative_mouse.buttons = 0x01u;
        if (vm_machine_submit_host_input(session, &event) !=
            LIB_STATUS_OK) goto done;
    }
    if (!vm_mouse_dos_run_until_packet(session, bytes_address, expected) ||
        core_machine_capture_display_snapshot(session->core_machine, &snapshot) !=
            LIB_STATUS_OK || snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT ||
        snapshot.characters[VM_MOUSE_DOS_MARKER_CELL] == 'O') goto done;
    stage = 6;
    passed = vm_mouse_dos_run_until(session, VM_MOUSE_DOS_RUN_BUDGET, 'O');
    if (!passed && session != LIB_NULL) {
        core_machine_display_snapshot snapshot;

        if (core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) == LIB_STATUS_OK &&
            snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT) {
            fprintf(stderr, "M5:T241:MOUSE-DRIVER:MARKER=%02X\n",
                snapshot.characters[VM_MOUSE_DOS_MARKER_CELL]);
        }
    }

done:
    integration_ini_session_close(&ini_session);
    if (!passed) {
        fprintf(stderr, "M5:T241:MOUSE-DRIVER:DOS:FAIL:STAGE=%d\n", stage);
        return 1;
    }
    printf("M5:T267:S3:AUX:DOS:OK\n");
    return 0;
}
