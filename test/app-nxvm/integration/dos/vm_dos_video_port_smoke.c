#include "lib/types/types_interface.h"
#include <ctype.h>
#include <stdio.h>

#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/waiting.h"
#include "../../unit/core/devices/support/core_machine_cpu_fixture.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define VM_DOS_VIDEO_PROBE_INSTRUCTION_BUDGET 1500000u
#define VM_DOS_VIDEO_DISPLAY_OBSERVATION_QUANTUM 256u
#define VM_DOS_VIDEO_TEXT_CELLS (80u * 25u)

static lib_i32 vm_dos_video_has_prompt(const core_machine_display_snapshot *snapshot)
{
    lib_size cell;

    if (snapshot == LIB_NULL) return 0;
    for (cell = 0u; cell + 3u < VM_DOS_VIDEO_TEXT_CELLS; ++cell) {
        if (isalpha(snapshot->characters[cell]) &&
            snapshot->characters[cell + 1u] == ':' &&
            snapshot->characters[cell + 2u] == '\\' &&
            snapshot->characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;
    vm_machine *session;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result = {0};
    core_machine_observation observation;
    core_machine_display_snapshot snapshot;
    t_cpu cpu;
    lib_u8 opcode[2];
    lib_u8 functions[256] = {0};
    lib_u64 instruction;
    lib_u32 int10_count = 0u;
    lib_u32 f2_count = 0u;
    lib_i32 prompt_seen = 0;
    lib_i32 failed = 0;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    if (!session->active) goto fail;
    vm_machine_reset(session);
    for (instruction = 0u; instruction < VM_DOS_VIDEO_PROBE_INSTRUCTION_BUDGET;
         ++instruction) {
        if (core_machine_capture_observation(session->core_machine, &observation) !=
                LIB_STATUS_OK || core_machine_memory_read(session->core_machine,
                observation.cpu.cs_base + observation.cpu.eip, opcode,
                sizeof(opcode)) != LIB_STATUS_OK) {
            failed = 1;
            break;
        }
        if (opcode[0] == 0xcdu && opcode[1] == 0x10u) {
            ++int10_count;
            cpu = test_core_machine_fixture_capture_cpu_after_run(
                session->core_machine);
            functions[cpu.data.ah] = 1u;
        }
        if (opcode[0] == 0xcdu && opcode[1] == 0xf2u) {
            ++f2_count;
        }
        if (core_machine_run(session->core_machine, budget, &result) != LIB_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT) {
            failed = 1;
            break;
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            lib_i32 advanced = 0;

            if (vm_machine_waiting_advance(session, &result, &advanced) != LIB_STATUS_OK ||
                !advanced) {
                failed = 1;
                break;
            }
        }
        /* The prompt is persistent.  Preserve per-instruction INT observation,
         * but avoid copying a complete display frame after every instruction. */
        if ((instruction % VM_DOS_VIDEO_DISPLAY_OBSERVATION_QUANTUM == 0u ||
             instruction + 1u == VM_DOS_VIDEO_PROBE_INSTRUCTION_BUDGET) &&
            core_machine_capture_display_snapshot(session->core_machine, &snapshot) ==
                LIB_STATUS_OK && vm_dos_video_has_prompt(&snapshot)) {
            prompt_seen = 1;
            break;
        }
    }
    if (failed || !prompt_seen || int10_count == 0u || f2_count != 0u) goto fail;
    printf("M5:T212:S2:VIDEO:DOS:OK INT10=%u F2=%u AH=", int10_count, f2_count);
    for (instruction = 0u; instruction < 256u; ++instruction) {
        if (functions[instruction]) printf("%02X", (lib_u32)instruction);
    }
    printf("\n");
    integration_ini_session_close(&ini_session);
    return 0;

fail:
    fprintf(stderr,
        "M5:T212:S2:VIDEO:DOS:FAIL INT10=%u F2=%u PROMPT=%d STOP=%d\n",
        int10_count, f2_count, prompt_seen, (lib_i32)result.reason);
    integration_ini_session_close(&ini_session);
    return 1;
}
