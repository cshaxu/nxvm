#include "lib/types/types_interface.h"
#include "type.h"

#include <windows.h>

#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/memory_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/waiting.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define VM_FDC242_BOOT_BUDGET 6000000u
#define VM_FDC242_RUN_BUDGET 400000u
#define VM_FDC242_MARKER_CELL 1920u
#define VM_FDC242_DMA_ADDRESS 0x00080000u
#define VM_FDC242_TRACK_BYTES (18u * 512u)
#define VM_FDC242_HANDLER_OFFSET 0x0280u
#define VM_FDC242_IRQ_COUNT_OFFSET 0x02a0u
#define VM_FDC242_RESULT_OFFSET 0x02a1u
#define VM_FDC242_DISPLAY_OBSERVATION_QUANTUM 256u

static lib_u16 vm_fdc242_fat_get(const lib_u8 *fat, lib_u16 cluster)
{
    lib_u32 offset = cluster + cluster / 2u;
    lib_u16 pair = (lib_u16)(fat[offset] | ((lib_u16)fat[offset + 1u] << 8));
    return (cluster & 1u) ? pair >> 4u : pair & 0x0fffu;
}

static C_VOID vm_fdc242_fat_set(lib_u8 *fat, lib_u16 cluster, lib_u16 value)
{
    lib_u32 offset = cluster + cluster / 2u;
    lib_u16 pair = (lib_u16)(fat[offset] | ((lib_u16)fat[offset + 1u] << 8));
    if (cluster & 1u) pair = (lib_u16)((pair & 0x000fu) | (value << 4u));
    else pair = (lib_u16)((pair & 0xf000u) | value);
    fat[offset] = (lib_u8)pair;
    fat[offset + 1u] = (lib_u8)(pair >> 8u);
}

static C_INT vm_fdc242_install(lib_u8 *image, DWORD size)
{
    static const lib_u8 program[] = {
        0x1e,0x31,0xc0,0x8e,0xd8,0xb8,0x80,0x02,0xa3,0x38,0x00,
        0x0e,0x58,0xa3,0x3a,0x00,0x1f, 0xe4,0x21,0x24,0xbf,0xe6,0x21,0xfb,
        0xb0,0x06,0xe6,0x0a, 0x30,0xc0,0xe6,0x0d, 0xe6,0x0c, 0xe6,0x04, 0xb0,0x00,0xe6,0x04,
        0xb0,0xff,0xe6,0x05, 0xb0,0x23,0xe6,0x05, 0xb0,0x08,0xe6,0x81,
        0xb0,0x46,0xe6,0x0b,
        0xba,0xf2,0x03, 0xb0,0x0c,0xee, 0xba,0xf5,0x03,
        0xb0,0x08,0xee, 0xec,0xec, 0xb0,0x08,0xee, 0xec,0xec,
        0xb0,0x08,0xee, 0xec,0xec, 0xb0,0x08,0xee, 0xec,0xec,
        0xb0,0x0a,0xee, 0xb0,0x00,0xee,
        0x80,0x3e,0xa0,0x02,0x00,0x74,0xf9,
        0xec,0xa2,0xb0,0x02, 0xec,0xa2,0xb1,0x02, 0xec,0xa2,0xb2,0x02,
        0xec,0xa2,0xb3,0x02, 0xec,0xa2,0xb4,0x02, 0xec,0xa2,0xb5,0x02,
        0xec,0xa2,0xb6,0x02, 0xb0,0x08,0xee, 0xec,0xa2,0xb7,0x02,
        0xec,0xa2,0xb8,0x02, 0xc6,0x06,0xa0,0x02,0x00,
        0xe4,0x21,0x24,0xbf,0xe6,0x21,
        0xba,0xf2,0x03, 0xb0,0x1c,0xee,
        0xba,0xf5,0x03,
        0xb0,0x03,0xee, 0xb0,0xdf,0xee, 0xb0,0x02,0xee,
        0xb0,0x02,0xe6,0x0a,
        0xb0,0x42,0xee, 0xb0,0x00,0xee, 0xb0,0x00,0xee,
        0xb0,0x00,0xee, 0xb0,0x01,0xee, 0xb0,0x02,0xee,
        0xb0,0x12,0xee, 0xb0,0x1b,0xee, 0xb0,0xff,0xee,
        0x80,0x3e,0xa0,0x02,0x01,0x74,0xf9,
        0xba,0xf4,0x03, 0xec,0x24,0xc0,0x3c,0xc0,0x75,0xf9,
        0xba,0xf5,0x03,
        0xec,0xa2,0xa1,0x02, 0xec,0xa2,0xa2,0x02, 0xec,0xa2,0xa3,0x02,
        0xec,0xa2,0xa4,0x02, 0xec,0xa2,0xa5,0x02, 0xec,0xa2,0xa6,0x02,
        0xec,0xa2,0xa7,0x02,
        0xb0,0x08,0xee, 0xec,0xa2,0xa8,0x02, 0xec,0xa2,0xa9,0x02,
        0xb0,0x20,0xe6,0x20,
        0xb8,0x00,0xb8, 0x8e,0xc0,
        0x26,0xc7,0x06,0x00,0x0f,0x46,0x07,
        0x26,0xc7,0x06,0x02,0x0f,0x44,0x07,
        0x26,0xc7,0x06,0x04,0x0f,0x43,0x07,
        0xeb,0xfe,
        [0x180] = 0x50,0x1e,0x0e,0x1f,0xfe,0x06,0xa0,0x02,
        0xe4,0x21,0x0c,0x40,0xe6,0x21,0xb0,0x20,0xe6,0x20,
        0x1f,0x58,0xcf
    };
    lib_u32 bps, spc, reserved, fats, roots, spf, root_start, root_bytes,
        data_start, clusters, cluster, root;
    lib_u8 *entry = LIB_NULL;
    if (image == LIB_NULL || size < 512u) return 0;
    bps = image[11u] | ((lib_u32)image[12u] << 8u); spc = image[13u];
    reserved = image[14u] | ((lib_u32)image[15u] << 8u); fats = image[16u];
    roots = image[17u] | ((lib_u32)image[18u] << 8u);
    spf = image[22u] | ((lib_u32)image[23u] << 8u);
    if (!bps || !spc || !fats || !spf) return 0;
    root_start = (reserved + fats * spf) * bps; root_bytes = roots * 32u;
    data_start = root_start + ((root_bytes + bps - 1u) / bps) * bps;
    if (data_start >= size || root_start + root_bytes > size || sizeof(program) > bps * spc)
        return 0;
    for (root = 0u; root < roots; ++root) {
        lib_u8 *candidate = image + root_start + root * 32u;
        if (candidate[0] == 0u || candidate[0] == 0xe5u) { entry = candidate; break; }
    }
    clusters = (size - data_start) / (bps * spc);
    for (cluster = 2u; cluster < clusters + 2u; ++cluster) {
        if (vm_fdc242_fat_get(image + reserved * bps, (lib_u16)cluster) == 0u) break;
    }
    if (entry == LIB_NULL || cluster >= clusters + 2u) return 0;
    lib_memory_set(entry, 0, 32u); lib_memory_copy(entry, "FDC242  COM", 11u);
    entry[11u] = 0x20u; entry[26u] = (lib_u8)cluster; entry[27u] = (lib_u8)(cluster >> 8u);
    entry[28u] = (lib_u8)sizeof(program); entry[29u] = (lib_u8)(sizeof(program) >> 8u);
    for (root = 0u; root < fats; ++root) vm_fdc242_fat_set(image +
        (reserved + root * spf) * bps, (lib_u16)cluster, 0x0fffu);
    lib_memory_copy(image + data_start + (cluster - 2u) * bps * spc, program, sizeof(program));
    return 1;
}

static type_status vm_fdc242_install_on_overlay(
    integration_ini_session *ini_session, C_VOID *opaque)
{
    lib_u8 *image = LIB_NULL;
    lib_u8 *expected = (lib_u8 *)opaque;
    lib_size size = 0u;
    C_INT installed;

    if (ini_session == LIB_NULL || expected == LIB_NULL ||
        integration_ini_session_overlay_read(ini_session, VM_MACHINE_MEDIA_FDD_ID,
            (C_VOID **)&image, &size) != TYPE_STATUS_OK || size > MAXDWORD) return TYPE_STATUS_FAULT;
    installed = vm_fdc242_install(image, (DWORD)size) &&
        integration_ini_session_overlay_write(ini_session, VM_MACHINE_MEDIA_FDD_ID,
            image, size) == TYPE_STATUS_OK;
    if (installed) lib_memory_copy(expected, image, VM_FDC242_TRACK_BYTES);
    lib_release(image);
    return installed ? TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}

static C_INT vm_fdc242_has_prompt(const core_machine_display_snapshot *snapshot)
{
    lib_size index;
    if (snapshot == LIB_NULL || snapshot->kind != CORE_MACHINE_DISPLAY_KIND_TEXT) return 0;
    for (index = 0u; index + 3u < 2000u; ++index) {
        if (STD_ISALPHA(snapshot->characters[index]) && snapshot->characters[index + 1u] == ':' &&
            snapshot->characters[index + 2u] == '\\' && snapshot->characters[index + 3u] == '>') return 1;
    }
    return 0;
}

static C_INT vm_fdc242_run_until(vm_machine *session, lib_u32 limit,
    lib_u32 quantum, C_INT require_marker)
{
    core_machine_run_budget budget = {quantum, 0u}; core_machine_run_result result;
    core_machine_display_snapshot snapshot; lib_u32 used = 0u;
    while (used < limit) {
        C_INT advanced = 0;

        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT) return 0;
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
            (vm_machine_waiting_advance(session, &result, &advanced) != TYPE_STATUS_OK ||
            !advanced)) return 0;
        used += budget.instructions;
        /* Both the prompt and the test program's marker persist.  Sampling
         * the display less frequently does not alter the run quantum or the
         * instruction-level FDC/DMA execution being compared. */
        if (used < limit && used % VM_FDC242_DISPLAY_OBSERVATION_QUANTUM != 0u)
            continue;
        if (core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK) return 0;
        if (require_marker ? snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
                snapshot.characters[VM_FDC242_MARKER_CELL] == 'F' &&
                snapshot.characters[VM_FDC242_MARKER_CELL + 1u] == 'D' &&
                snapshot.characters[VM_FDC242_MARKER_CELL + 2u] == 'C' :
            vm_fdc242_has_prompt(&snapshot)) return 1;
    }
    return 0;
}

typedef struct vm_fdc242_result {
    lib_u8 bytes[VM_FDC242_TRACK_BYTES];
    lib_u8 result[10];
    lib_u8 off_result[9];
} vm_fdc242_result;

static C_INT vm_fdc242_run_case(integration_ini_session *ini_session,
    lib_u32 quantum, vm_fdc242_result *out_result)
{
    static const lib_u8 command[] = {0x2bu,0x23u,0x21u,0x1eu,0x25u,0x1eu,0x5au};
    vm_machine *session = LIB_NULL;
    lib_u16 program_cs = 0u;
    lib_size index;
    C_INT ok = 0;

    if (ini_session == LIB_NULL || out_result == LIB_NULL || quantum == 0u ||
        integration_ini_session_restart(ini_session) != TYPE_STATUS_OK) goto done;
    session = ini_session->session;
    vm_machine_executor_state_start(session->control.state);
    if (!vm_fdc242_run_until(session, VM_FDC242_BOOT_BUDGET, quantum, 0u)) goto done;
    for (index = 0u; index < sizeof(command); ++index) if (core_machine_keyboard_receive_native_byte(
        session->core_machine, command[index]) != TYPE_STATUS_OK) goto done;
    if (!vm_fdc242_run_until(session, VM_FDC242_RUN_BUDGET, quantum, 1) ||
        core_machine_memory_read(session->core_machine, VM_FDC242_DMA_ADDRESS,
        out_result->bytes, sizeof(out_result->bytes)) != TYPE_STATUS_OK) goto done;
    if (core_machine_memory_read(session->core_machine, 0x003au, &program_cs,
        sizeof(program_cs)) != TYPE_STATUS_OK || core_machine_memory_read(session->core_machine,
        ((lib_u32)program_cs << 4u) + VM_FDC242_IRQ_COUNT_OFFSET, out_result->result,
        sizeof(out_result->result)) != TYPE_STATUS_OK || core_machine_memory_read(session->core_machine,
        ((lib_u32)program_cs << 4u) + 0x02b0u, out_result->off_result,
        sizeof(out_result->off_result)) != TYPE_STATUS_OK) goto done;
    ok = 1;
done:
    if (session != LIB_NULL) vm_machine_executor_state_stop(session->control.state);
    return ok;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_ini_session ini_session;
    lib_u8 expected[VM_FDC242_TRACK_BYTES];
    vm_fdc242_result one_instruction = {0};
    vm_fdc242_result short_quantum = {0};
    C_INT passed = 0;
    lib_size first_mismatch = sizeof(expected);

    if (argc != 3 || integration_ini_session_open_with_overlay_transform(argv[1], argv[2],
            vm_fdc242_install_on_overlay, expected, &ini_session) != TYPE_STATUS_OK) {
        return 77;
    }
    passed = vm_fdc242_run_case(&ini_session, 1u, &one_instruction) &&
        vm_fdc242_run_case(&ini_session, 128u, &short_quantum) &&
        lib_memory_compare(expected, one_instruction.bytes, sizeof(expected)) == 0 &&
        lib_memory_compare(&one_instruction, &short_quantum, sizeof(one_instruction)) == 0 &&
        one_instruction.result[0] == 1u && one_instruction.result[1] == 0x20u &&
        one_instruction.result[2] == 0u && one_instruction.result[3] == 0u &&
        one_instruction.result[4] == 0u && one_instruction.result[5] == 0u &&
        one_instruction.result[6] == 0x13u && one_instruction.result[7] == 0x02u &&
        one_instruction.result[8] == 0x80u && one_instruction.result[9] == 0u &&
        one_instruction.off_result[0] == core_machine_fdc_ST0_ABNORMAL &&
        one_instruction.off_result[1] == 0x04u && one_instruction.off_result[7] ==
        0x80u;
    if (!passed) {
        for (lib_size index = 0u; index < sizeof(expected); ++index) {
            if (expected[index] != one_instruction.bytes[index]) {
                first_mismatch = index;
                break;
            }
        }
        STD_FPRINTF(STD_STDERR, "M5:T242:S4:FDC:DOS:FAIL bytes=%d@%zu:%02x/%02x runs=%d result=%d off=%d/%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",
            lib_memory_compare(expected, one_instruction.bytes, sizeof(expected)) == 0,
            first_mismatch,
            first_mismatch < sizeof(expected) ? expected[first_mismatch] : 0u,
            first_mismatch < sizeof(expected) ? one_instruction.bytes[first_mismatch] : 0u,
            lib_memory_compare(&one_instruction, &short_quantum, sizeof(one_instruction)) == 0,
            one_instruction.result[0] == 1u && one_instruction.result[1] == 0x20u &&
            one_instruction.result[2] == 0u && one_instruction.result[3] == 0u &&
            one_instruction.result[4] == 0u && one_instruction.result[5] == 0u &&
            one_instruction.result[6] == 0x13u && one_instruction.result[7] == 0x02u &&
            one_instruction.result[8] == 0x80u && one_instruction.result[9] == 0u,
            one_instruction.off_result[0] == core_machine_fdc_ST0_ABNORMAL &&
            one_instruction.off_result[1] == 0x04u && one_instruction.off_result[7] ==
            0x80u,
            one_instruction.off_result[0], one_instruction.off_result[1],
            one_instruction.off_result[2], one_instruction.off_result[3],
            one_instruction.off_result[4], one_instruction.off_result[5],
            one_instruction.off_result[6], one_instruction.off_result[7],
            one_instruction.off_result[8]);
        integration_ini_session_close(&ini_session); return 1;
    }
    integration_ini_session_close(&ini_session);
    STD_PRINTF("M5:T268:S3:FDC-MOTOR:DOS:OK\n");
    STD_PRINTF("M5:T269:S3:DMA-GRANT:DOS:OK\n");
    STD_PRINTF("M5:T290:S3:FDC:DOS:OK\n");
    STD_PRINTF("M5:T291:S3:FDC:DOS:OK\n");
    STD_PRINTF("M5:T242:S4:FDC:DOS:OK\n"); return 0;
}
