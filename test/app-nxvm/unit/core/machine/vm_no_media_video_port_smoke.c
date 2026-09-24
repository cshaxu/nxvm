#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/rom/session_assets.h"
#include "../devices/support/core_machine_cpu_fixture.h"

#define VM_NO_MEDIA_PROBE_INSTRUCTION_BUDGET 100000u
#define VM_NO_MEDIA_TEXT_CELLS (80u * 25u)

static lib_i32 vm_no_media_snapshot_has_text(
    const core_machine_display_snapshot *snapshot, const char *text)
{
    lib_size cell;
    lib_size character;
    lib_size length = lib_text_length(text);

    if (snapshot == LIB_NULL || text == LIB_NULL) return 0;
    for (cell = 0u; cell + length <= VM_NO_MEDIA_TEXT_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (snapshot->characters[cell + character] !=
                (lib_u8)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_observation observation;
    core_machine_display_snapshot snapshot;
    lib_u8 opcode[2];
    lib_u8 functions[256] = {0};
    lib_u16 cursor;
    lib_u64 instruction;
    lib_u32 int10_count = 0u;
    lib_u32 f2_count = 0u;
    lib_i32 key_wait_seen = 0;
    lib_i32 failed = 0;
    t_cpu cpu;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK) return 1;
    if (!session->active || session->core_machine == LIB_NULL) goto fail;
    vm_machine_reset(session);
    for (instruction = 0u; instruction < VM_NO_MEDIA_PROBE_INSTRUCTION_BUDGET;
         ++instruction) {
        if (core_machine_capture_observation(session->core_machine,
                &observation) != LIB_STATUS_OK ||
            core_machine_memory_read(session->core_machine,
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
        if (opcode[0] == 0xcdu && opcode[1] == 0xf2u) ++f2_count;
        if (opcode[0] == 0xb4u && opcode[1] == 0x11u) key_wait_seen = 1;
        if (core_machine_run(session->core_machine, budget, &result) !=
            LIB_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            failed = 1;
            break;
        }
        if (key_wait_seen) break;
    }
    if (core_machine_memory_read(session->core_machine, 0x0450u, &cursor,
            sizeof(cursor)) != LIB_STATUS_OK ||
        core_machine_capture_display_snapshot(session->core_machine, &snapshot) !=
            LIB_STATUS_OK || cursor != 0x0600u || !snapshot.cursor_visible ||
        snapshot.cursor_x != 0u || snapshot.cursor_y != 6u ||
        int10_count == 0u || f2_count != 0u ||
        !key_wait_seen || !vm_no_media_snapshot_has_text(&snapshot,
            "Invalid boot disk")) {
        failed = 1;
    }
    if (!failed) {
        printf("M5:T212:S2:VIDEO:ROM:OK INT10=%u F2=%u CURSOR=%04x AH=",
            int10_count, f2_count, cursor);
        for (instruction = 0u; instruction < 256u; ++instruction) {
            if (functions[instruction]) printf("%02X", (lib_u32)instruction);
        }
        printf("\n");
    }

fail:
    vm_machine_destroy(session);
    return failed;
}
