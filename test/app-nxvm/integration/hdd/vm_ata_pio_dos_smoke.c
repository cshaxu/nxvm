#include "lib/types/types_interface.h"
#include <ctype.h>
#include <stdio.h>

#include <windows.h>

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/hdc.h"
#include "test/app-nxvm/integration/support/session_ini.h"
#include "app-nxvm/machine/waiting.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"

#define VM_ATA253_BOOT_BUDGET 800000u
#define VM_ATA253_RUN_BUDGET 400000u
#define VM_ATA253_MARKER_CELL 1920u

typedef struct vm_ata253_program {
    lib_u8 bytes[256];
    lib_u16 length;
} vm_ata253_program;

static lib_i32 vm_ata253_put(vm_ata253_program *program, lib_u8 value)
{
    if (program == LIB_NULL || program->length >= sizeof(program->bytes)) return 0;
    program->bytes[program->length++] = value;
    return 1;
}

static lib_i32 vm_ata253_word(vm_ata253_program *program, lib_u16 value)
{
    return vm_ata253_put(program, (lib_u8)value) &&
        vm_ata253_put(program, (lib_u8)(value >> 8u));
}

static lib_i32 vm_ata253_out_task_file(vm_ata253_program *program,
    lib_u8 command)
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

static lib_i32 vm_ata253_write_sector(vm_ata253_program *program, lib_u16 word)
{
    return vm_ata253_put(program, 0xb9u) && vm_ata253_word(program, 256u) &&
        vm_ata253_put(program, 0xb8u) && vm_ata253_word(program, word) &&
        vm_ata253_put(program, 0xefu) && vm_ata253_put(program, 0xe2u) &&
        vm_ata253_put(program, 0xfdu);
}

static lib_i32 vm_ata253_set_nien(vm_ata253_program *program, lib_i32 enabled)
{
    return vm_ata253_put(program, 0xbau) && vm_ata253_word(program, 0x03f6u) &&
        vm_ata253_put(program, 0xb0u) &&
        vm_ata253_put(program, enabled ? CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN : 0u) &&
        vm_ata253_put(program, 0xeeu);
}

static lib_i32 vm_ata253_discard_words(vm_ata253_program *program, lib_u16 count)
{
    return vm_ata253_put(program, 0xb9u) && vm_ata253_word(program, count) &&
        vm_ata253_put(program, 0xedu) && vm_ata253_put(program, 0xe2u) &&
        vm_ata253_put(program, 0xfdu);
}

static lib_i32 vm_ata253_wait_drq(vm_ata253_program *program)
{
    return vm_ata253_put(program, 0xbau) && vm_ata253_word(program, 0x01f7u) &&
        vm_ata253_put(program, 0xecu) && vm_ata253_put(program, 0xa8u) &&
        vm_ata253_put(program, CORE_MACHINE_HDC_STATUS_BSY) && vm_ata253_put(program, 0x75u) &&
        vm_ata253_put(program, 0xfbu) && vm_ata253_put(program, 0xa8u) &&
        vm_ata253_put(program, CORE_MACHINE_HDC_STATUS_DRQ) && vm_ata253_put(program, 0x74u) &&
        vm_ata253_put(program, 0xf7u);
}

static lib_i32 vm_ata253_wait_ready(vm_ata253_program *program)
{
    return vm_ata253_put(program, 0xbau) && vm_ata253_word(program, 0x01f7u) &&
        vm_ata253_put(program, 0xecu) && vm_ata253_put(program, 0xa8u) &&
        vm_ata253_put(program, CORE_MACHINE_HDC_STATUS_BSY) && vm_ata253_put(program, 0x75u) &&
        vm_ata253_put(program, 0xfbu) && vm_ata253_put(program, 0xa8u) &&
        vm_ata253_put(program, CORE_MACHINE_HDC_STATUS_DRQ) && vm_ata253_put(program, 0x75u) &&
        vm_ata253_put(program, 0xf7u);
}

static lib_i32 vm_ata253_marker(vm_ata253_program *program, lib_u8 character,
    lib_u8 exit_code)
{
    return vm_ata253_put(program, 0xb8u) && vm_ata253_word(program, 0xb800u) &&
        vm_ata253_put(program, 0x8eu) && vm_ata253_put(program, 0xc0u) &&
        vm_ata253_put(program, 0x26u) && vm_ata253_put(program, 0xc7u) &&
        vm_ata253_put(program, 0x06u) && vm_ata253_word(program, 0x0f00u) &&
        vm_ata253_word(program, (lib_u16)(0x0700u | character)) &&
        vm_ata253_put(program, 0xb8u) && vm_ata253_word(program,
            (lib_u16)(0x4c00u | exit_code)) && vm_ata253_put(program, 0xcdu) &&
        vm_ata253_put(program, 0x21u);
}

static lib_i32 vm_ata253_build_program(vm_ata253_program *program)
{
    lib_u16 first_failure;
    lib_u16 second_failure;
    lib_u16 failure;
    lib_i16 delta;

    if (program == LIB_NULL) return 0;
    lib_memory_set(program, 0, sizeof(*program));
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
    delta = (lib_i16)failure - (lib_i16)(first_failure + 1u);
    if (delta < -128 || delta > 127) return 0;
    program->bytes[first_failure] = (lib_u8)(lib_i8)delta;
    delta = (lib_i16)failure - (lib_i16)(second_failure + 1u);
    if (delta < -128 || delta > 127) return 0;
    program->bytes[second_failure] = (lib_u8)(lib_i8)delta;
    return 1;
}

static lib_u16 vm_ata253_fat12_get(const lib_u8 *fat, lib_u16 cluster)
{
    lib_u32 offset = cluster + cluster / 2u;
    lib_u16 pair = (lib_u16)(fat[offset] | ((lib_u16)fat[offset + 1u] << 8u));

    return (cluster & 1u) != 0u ? pair >> 4u : pair & 0x0fffu;
}

static void vm_ata253_fat12_set(lib_u8 *fat, lib_u16 cluster, lib_u16 value)
{
    lib_u32 offset = cluster + cluster / 2u;
    lib_u16 pair = (lib_u16)(fat[offset] | ((lib_u16)fat[offset + 1u] << 8u));

    if ((cluster & 1u) != 0u) pair = (lib_u16)((pair & 0x000fu) | (value << 4u));
    else pair = (lib_u16)((pair & 0xf000u) | value);
    fat[offset] = (lib_u8)pair;
    fat[offset + 1u] = (lib_u8)(pair >> 8u);
}

static lib_i32 vm_ata253_zero_image(lib_u8 *image, DWORD image_size);
static lib_i32 vm_ata253_install(lib_u8 *image, DWORD image_size);

static lib_status vm_ata253_install_on_overlay(
    integration_ini_session *ini_session, void *opaque)
{
    lib_u8 *fdd_image = LIB_NULL;
    lib_u8 *hdd_image = LIB_NULL;
    lib_size fdd_size = 0u;
    lib_size hdd_size = 0u;
    lib_i32 ok;

    (void)opaque;
    if (ini_session == LIB_NULL || integration_ini_session_overlay_read(ini_session,
            VM_MACHINE_MEDIA_FDD_ID, (void **)&fdd_image, &fdd_size) != LIB_STATUS_OK ||
        integration_ini_session_overlay_read(ini_session, VM_MACHINE_MEDIA_HDD_ID,
            (void **)&hdd_image, &hdd_size) != LIB_STATUS_OK || fdd_size > MAXDWORD ||
        hdd_size > MAXDWORD || !vm_ata253_install(fdd_image, (DWORD)fdd_size) ||
        integration_ini_session_overlay_write(ini_session, VM_MACHINE_MEDIA_FDD_ID,
            fdd_image, fdd_size) != LIB_STATUS_OK) {
        lib_release(fdd_image);
        lib_release(hdd_image);
        return LIB_STATUS_INTERNAL_ERROR;
    }
    ok = vm_ata253_zero_image(hdd_image, (DWORD)hdd_size) &&
        integration_ini_session_overlay_write(ini_session, VM_MACHINE_MEDIA_HDD_ID,
            hdd_image, hdd_size) == LIB_STATUS_OK;
    lib_release(fdd_image);
    lib_release(hdd_image);
    return ok ? LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}

static lib_i32 vm_ata253_zero_image(lib_u8 *image, DWORD image_size)
{
    if (image == LIB_NULL || image_size == 0u) return 0;
    lib_memory_set(image, 0, image_size);
    return 1;
}

static lib_i32 vm_ata253_install(lib_u8 *image, DWORD image_size)
{
    vm_ata253_program program;
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
    if (!vm_ata253_build_program(&program) || image == LIB_NULL ||
        image_size < 512u) return 0;
    bytes_per_sector = image[11u] | ((lib_u32)image[12u] << 8u);
    sectors_per_cluster = image[13u];
    reserved_sectors = image[14u] | ((lib_u32)image[15u] << 8u);
    fat_count = image[16u];
    root_entries = image[17u] | ((lib_u32)image[18u] << 8u);
    sectors_per_fat = image[22u] | ((lib_u32)image[23u] << 8u);
    if (bytes_per_sector == 0u || sectors_per_cluster == 0u || fat_count == 0u ||
        sectors_per_fat == 0u) return 0;
    root_start = (reserved_sectors + fat_count * sectors_per_fat) * bytes_per_sector;
    root_bytes = root_entries * 32u;
    data_start = root_start + ((root_bytes + bytes_per_sector - 1u) /
        bytes_per_sector) * bytes_per_sector;
    if (data_start >= image_size || root_start + root_bytes > image_size) return 0;
    for (root = 0u; root < root_entries; ++root) {
        lib_u8 *candidate = image + root_start + root * 32u;

        if (candidate[0] == 0u || candidate[0] == 0xe5u) {
            entry = candidate;
            break;
        }
    }
    clusters = (image_size - data_start) / (bytes_per_sector * sectors_per_cluster);
    for (cluster = 2u; cluster < clusters + 2u; ++cluster) {
        if (vm_ata253_fat12_get(image + reserved_sectors * bytes_per_sector,
                (lib_u16)cluster) == 0u) break;
    }
    if (entry == LIB_NULL || cluster >= clusters + 2u || program.length >
        bytes_per_sector * sectors_per_cluster) return 0;
    lib_memory_set(entry, 0, 32u);
    lib_memory_copy(entry, "ATA253  COM", 11u);
    entry[11u] = 0x20u;
    entry[26u] = (lib_u8)cluster;
    entry[27u] = (lib_u8)(cluster >> 8u);
    entry[28u] = (lib_u8)program.length;
    entry[29u] = (lib_u8)(program.length >> 8u);
    for (root = 0u; root < fat_count; ++root) {
        vm_ata253_fat12_set(image + (reserved_sectors + root * sectors_per_fat) *
            bytes_per_sector, (lib_u16)cluster, 0x0fffu);
    }
    lib_memory_copy(image + data_start + (cluster - 2u) * bytes_per_sector *
        sectors_per_cluster, program.bytes, program.length);
    return 1;
}

static lib_i32 vm_ata253_has_prompt(const core_machine_display_snapshot *snapshot)
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

static lib_i32 vm_ata253_run_until(vm_machine *session, lib_u32 limit,
    lib_u8 marker)
{
    const core_machine_run_budget budget = { 128u, 0u };
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
        if (marker != 0u ? snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
                snapshot.characters[VM_ATA253_MARKER_CELL] == marker :
            vm_ata253_has_prompt(&snapshot)) return 1;
        executed += budget.instructions;
    }
    return 0;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    static const lib_u8 command[] = { 0x1cu, 0x2cu, 0x1cu, 0x1eu, 0x2eu,
        0x26u, 0x5au };
    integration_ini_session ini_session;
    vm_machine *session = LIB_NULL;
    lib_size index;
    lib_i32 passed = 0;

    if (argc != 3 || integration_ini_session_open_with_overlay_transform(argv[1], argv[2],
            vm_ata253_install_on_overlay, LIB_NULL, &ini_session) != LIB_STATUS_OK) {
        return 77;
    }
    session = ini_session.session;
    if (session == LIB_NULL ||
        !vm_ata253_run_until(session, VM_ATA253_BOOT_BUDGET, 0u)) goto done;
    for (index = 0u; index < sizeof(command); ++index) {
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
                command[index]) != LIB_STATUS_OK) goto done;
    }
    passed = vm_ata253_run_until(session, VM_ATA253_RUN_BUDGET, 'O');

done:
    integration_ini_session_close(&ini_session);
    if (!passed) return 1;
    printf("M5:T286:S3:ATA-NIEN:DOS:OK\n");
    printf("M5:T253:S3:ATA-PIO:DOS:OK\n");
    return 0;
}
