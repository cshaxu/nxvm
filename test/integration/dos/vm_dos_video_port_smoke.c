#include "type.h"

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/machine/fdd.h"
#include "../../core/support/core_machine_cpu_fixture.h"

#define VM_DOS_VIDEO_PROBE_INSTRUCTION_BUDGET 1500000u
#define VM_DOS_VIDEO_DISPLAY_OBSERVATION_QUANTUM 256u
#define VM_DOS_VIDEO_TEXT_CELLS (80u * 25u)

static C_INT vm_dos_video_has_prompt(const core_machine_display_snapshot *snapshot)
{
    STD_SIZE_T cell;

    if (snapshot == STD_NULL) return 0;
    for (cell = 0u; cell + 3u < VM_DOS_VIDEO_TEXT_CELLS; ++cell) {
        if (STD_ISALPHA(snapshot->characters[cell]) &&
            snapshot->characters[cell + 1u] == ':' &&
            snapshot->characters[cell + 2u] == '\\' &&
            snapshot->characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_session *session = STD_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_observation observation;
    core_machine_display_snapshot snapshot;
    t_cpu cpu;
    type_unsigned_8 opcode[2];
    type_unsigned_8 functions[256] = {0};
    type_unsigned_64 instruction;
    C_UINT int10_count = 0u;
    C_UINT f2_count = 0u;
    C_INT prompt_seen = 0;
    C_INT failed = 0;

    if (argc != 2) return 1;
    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    if (!session->active || vm_machine_fdd_insert_for(&session->fdd, argv[1]) != 0) {
        goto fail;
    }
    vm_session_reset(session);
    for (instruction = 0u; instruction < VM_DOS_VIDEO_PROBE_INSTRUCTION_BUDGET;
         ++instruction) {
        if (core_machine_capture_observation(session->core_machine, &observation) !=
                TYPE_STATUS_OK || core_machine_memory_read(session->core_machine,
                observation.cpu.cs_base + observation.cpu.eip, opcode,
                sizeof(opcode)) != TYPE_STATUS_OK) {
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
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT) {
            failed = 1;
            break;
        }
        /* The prompt is persistent.  Preserve per-instruction INT observation,
         * but avoid copying a complete display frame after every instruction. */
        if ((instruction % VM_DOS_VIDEO_DISPLAY_OBSERVATION_QUANTUM == 0u ||
             instruction + 1u == VM_DOS_VIDEO_PROBE_INSTRUCTION_BUDGET) &&
            core_machine_capture_display_snapshot(session->core_machine, &snapshot) ==
                TYPE_STATUS_OK && vm_dos_video_has_prompt(&snapshot)) {
            prompt_seen = 1;
            break;
        }
    }
    if (failed || !prompt_seen || int10_count == 0u || f2_count != 0u) goto fail;
    STD_PRINTF("M5:T212:S2:VIDEO:DOS:OK INT10=%u F2=%u AH=", int10_count, f2_count);
    for (instruction = 0u; instruction < 256u; ++instruction) {
        if (functions[instruction]) STD_PRINTF("%02X", (C_UINT)instruction);
    }
    STD_PRINTF("\n");
    vm_session_destroy(session);
    return 0;

fail:
    STD_FPRINTF(STD_STDERR,
        "M5:T212:S2:VIDEO:DOS:FAIL INT10=%u F2=%u PROMPT=%d STOP=%d\n",
        int10_count, f2_count, prompt_seen, (C_INT)result.reason);
    vm_session_destroy(session);
    return 1;
}
