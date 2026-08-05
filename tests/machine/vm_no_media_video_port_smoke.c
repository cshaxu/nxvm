#include "type.h"

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "tests/support/vm_session_fixture.h"

#define VM_NO_MEDIA_PROBE_INSTRUCTION_BUDGET 100000u
#define VM_NO_MEDIA_TEXT_CELLS (80u * 25u)

static C_INT vm_no_media_snapshot_has_text(
    const core_machine_display_snapshot *snapshot, const C_CHAR *text)
{
    STD_SIZE_T cell;
    STD_SIZE_T character;
    STD_SIZE_T length = STD_STRLEN(text);

    if (snapshot == STD_NULL || text == STD_NULL) return 0;
    for (cell = 0u; cell + length <= VM_NO_MEDIA_TEXT_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (snapshot->characters[cell + character] !=
                (uint8_t)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_observation observation;
    core_machine_display_snapshot snapshot;
    uint8_t opcode[2];
    uint8_t functions[256] = {0};
    uint16_t cursor;
    uint64_t instruction;
    C_UINT int10_count = 0u;
    C_UINT f2_count = 0u;
    C_INT key_wait_seen = 0;
    C_INT failed = 0;
    t_cpu *cpu;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    if (!vm_session_fixture_is_active(session) || vm_session_fixture_machine(session) == STD_NULL) goto fail;
    vm_session_reset(session);
    cpu = core_machine_debug_cpu_borrow(vm_session_fixture_machine(session));
    if (cpu == STD_NULL) goto fail;
    for (instruction = 0u; instruction < VM_NO_MEDIA_PROBE_INSTRUCTION_BUDGET;
         ++instruction) {
        if (core_machine_capture_observation(vm_session_fixture_machine(session),
                &observation) != TYPE_STATUS_OK ||
            core_machine_memory_read(vm_session_fixture_machine(session),
                observation.cpu.cs_base + observation.cpu.eip, opcode,
                sizeof(opcode)) != TYPE_STATUS_OK) {
            failed = 1;
            break;
        }
        if (opcode[0] == 0xcdu && opcode[1] == 0x10u) {
            ++int10_count;
            functions[cpu->data.ah] = 1u;
        }
        if (opcode[0] == 0xcdu && opcode[1] == 0xf2u) ++f2_count;
        if (opcode[0] == 0xb4u && opcode[1] == 0x11u) key_wait_seen = 1;
        if (core_machine_run(vm_session_fixture_machine(session), budget, &result) !=
            TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            failed = 1;
            break;
        }
        if (key_wait_seen) break;
    }
    if (core_machine_memory_read(vm_session_fixture_machine(session), 0x0450u, &cursor,
            sizeof(cursor)) != TYPE_STATUS_OK ||
        core_machine_capture_display_snapshot(vm_session_fixture_machine(session), &snapshot) !=
            TYPE_STATUS_OK || cursor != 0x0600u || int10_count == 0u || f2_count != 0u ||
        !key_wait_seen || !vm_no_media_snapshot_has_text(&snapshot,
            "Invalid boot disk")) {
        failed = 1;
    }
    if (!failed) {
        STD_PRINTF("M5:T212:S2:VIDEO:ROM:OK INT10=%u F2=%u CURSOR=%04x AH=",
            int10_count, f2_count, cursor);
        for (instruction = 0u; instruction < 256u; ++instruction) {
            if (functions[instruction]) STD_PRINTF("%02X", (C_UINT)instruction);
        }
        STD_PRINTF("\n");
    }

fail:
    vm_session_destroy(session);
    return failed;
}
