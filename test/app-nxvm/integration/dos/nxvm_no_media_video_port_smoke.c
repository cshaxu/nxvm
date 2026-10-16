#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "test/x86-nxvm/setup/session_ini.h"

#define VM_NO_MEDIA_PROBE_INSTRUCTION_BUDGET 100000u
#define VM_NO_MEDIA_TEXT_CELLS (80u * 25u)

static lib_i32 vm_no_media_snapshot_has_text(
    const x86_video_snapshot *snapshot, const char *text)
{
    lib_size cell;
    lib_size character;
    lib_size length;

    if (snapshot == LIB_NULL || text == LIB_NULL) return 0;
    length = lib_text_length(text);
    for (cell = 0u; cell + length <= VM_NO_MEDIA_TEXT_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (snapshot->characters[cell + character] !=
                (lib_u8)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static lib_status vm_no_media_fixture(integration_ini_session *ini_session,
    void *opaque)
{
    lib_size slot;
    x86_nxvm_machine *session = ini_session->session;

    (void)opaque;
    for (slot = 0u; slot < X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT; ++slot)
        if (session->floppy[slot] != LIB_NULL &&
            x86_nxvm_machine_fdd_remove_for(session->floppy[slot]))
            return LIB_STATUS_IO_ERROR;
    for (slot = 0u; slot < X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT; ++slot)
        if (session->fixed_disk[slot] != LIB_NULL &&
            x86_nxvm_machine_hdd_remove(session->fixed_disk[slot]))
            return LIB_STATUS_IO_ERROR;
    return LIB_STATUS_OK;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;
    x86_nxvm_machine *session;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    x86_core_machine_observation observation;
    x86_video_snapshot snapshot;
    lib_u8 opcode[2];
    lib_u8 functions[256] = {0};
    lib_u16 cursor;
    lib_u64 instruction;
    lib_u32 int10_count = 0u;
    lib_u32 f2_count = 0u;
    lib_i32 key_wait_seen = 0;
    lib_i32 failed = 1;
    lib_u32 eax;

    if (argc != 3 || integration_ini_session_open_with_overlay_transform(
            argv[1], argv[2], vm_no_media_fixture, LIB_NULL,
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    if (session == LIB_NULL || !session->active ||
        session->x86_core_machine == LIB_NULL) goto fail;
    if (x86_nxvm_machine_reset(session) != LIB_STATUS_OK) goto fail;
    for (instruction = 0u; instruction < VM_NO_MEDIA_PROBE_INSTRUCTION_BUDGET;
         ++instruction) {
        if (x86_core_machine_capture_observation(session->x86_core_machine,
                &observation) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(session->x86_core_machine,
                observation.cpu.cs_base + observation.cpu.eip, opcode,
                sizeof(opcode)) != LIB_STATUS_OK) {
            goto fail;
        }
        if (opcode[0] == 0xcdu && opcode[1] == 0x10u) {
            ++int10_count;
            if (x86_core_machine_debug_read_register(session->x86_core_machine,
                    X86_CORE_CPU_DEBUG_EAX, &eax) != LIB_STATUS_OK) {
                goto fail;
            }
            functions[(eax >> 8u) & 0xffu] = 1u;
        }
        if (opcode[0] == 0xcdu && opcode[1] == 0xf2u) ++f2_count;
        if (opcode[0] == 0xb4u && opcode[1] == 0x11u) key_wait_seen = 1;
        if (x86_core_machine_run(session->x86_core_machine, budget, &result) !=
            LIB_STATUS_OK || result.reason == X86_CORE_MACHINE_STOP_FAULT) {
            goto fail;
        }
        if (key_wait_seen) break;
    }
    if (x86_core_machine_memory_read(session->x86_core_machine, 0x0450u, &cursor,
            sizeof(cursor)) != LIB_STATUS_OK ||
        x86_nxvm_board_capture_display_snapshot(session->board, &snapshot) !=
            LIB_STATUS_OK || cursor != 0x0600u || !snapshot.cursor_visible ||
        snapshot.cursor_x != 0u || snapshot.cursor_y != 6u ||
        int10_count == 0u || f2_count != 0u ||
        !key_wait_seen || !vm_no_media_snapshot_has_text(&snapshot,
            "Invalid boot disk")) {
        goto fail;
    }
    failed = 0;
    printf("NXVM:VIDEO:ROM:OK INT10=%u F2=%u CURSOR=%04x AH=",
        int10_count, f2_count, cursor);
    for (instruction = 0u; instruction < 256u; ++instruction) {
        if (functions[instruction]) printf("%02X", (lib_u32)instruction);
    }
    printf("\n");

fail:
    integration_ini_session_close(&ini_session);
    return failed;
}
