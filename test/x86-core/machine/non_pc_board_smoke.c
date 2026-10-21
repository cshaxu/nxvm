#include "lib/types/file.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"

typedef struct non_pc_board {
    lib_u64 advanced;
    lib_u32 reads;
    lib_u32 writes;
    lib_u32 acknowledgements;
    lib_u32 finalized;
    lib_u8 value;
    lib_bool interrupt;
    lib_bool timer_armed;
} non_pc_board;

static lib_status board_query(void *owner, lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access)
{
    (void)owner;
    (void)access;
    return physical == 0x7000u && bytes == 1u ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status board_read(void *owner, lib_u32 physical, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only)
{
    non_pc_board *board = owner;

    if (physical != 0x7000u || bytes != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    if (!observe_only) ++board->reads;
    *(lib_u8 *)destination = board->value;
    return LIB_STATUS_OK;
}

static lib_status board_write(void *owner, lib_u32 physical, lib_uptr source,
    lib_uptr bytes)
{
    non_pc_board *board = owner;

    if (physical != 0x7000u || bytes != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    board->value = *(const lib_u8 *)source;
    ++board->writes;
    return LIB_STATUS_OK;
}

static lib_bool board_interrupt(void *owner)
{
    return ((non_pc_board *)owner)->interrupt;
}

static lib_u8 board_acknowledge(void *owner)
{
    non_pc_board *board = owner;

    ++board->acknowledgements;
    board->interrupt = LIB_FALSE;
    return 0x30u;
}

static void board_advance(void *owner, lib_u64 ticks, lib_u64 due_tick)
{
    non_pc_board *board = owner;

    (void)due_tick;
    board->advanced += ticks;
    board->timer_armed = LIB_FALSE;
}

static void board_deadline(void *owner, lib_u64 now,
    x86_core_machine_attachment_deadline_observation *observation)
{
    (void)now;
    *observation = (x86_core_machine_attachment_deadline_observation){
        .source_ticks = ((non_pc_board *)owner)->timer_armed ? 7u : 0u
    };
}

static void board_reset(void *owner)
{
    non_pc_board *board = owner;

    board->advanced = 0u;
    board->reads = 0u;
    board->writes = 0u;
    board->acknowledgements = 0u;
    board->value = 0u;
    board->interrupt = LIB_FALSE;
    board->timer_armed = LIB_FALSE;
}

static void board_finalize(void *owner)
{
    ++((non_pc_board *)owner)->finalized;
}

static lib_i32 non_pc_board_run(x86_core_cpu_profile profile, lib_u32 reset_address)
{
    /* Project-authored reset jump, RAM/MMIO writes, HALT and IRQ return.
     * This is not a PC BIOS or an implementation of any commercial board. */
    const lib_u8 rom[80] = {
        [0] = 0xeau, 0x10u, 0x80u, 0u, 0u,
        [16] = 0xb8u, 0x34u, 0x12u, 0xa3u, 0u, 0x02u,
        0xb0u, 0x5au, 0xa2u, 0u, 0x70u, 0xfbu, 0x90u, 0xf4u,
        0xb0u, 0xa5u, 0xa2u, 0u, 0x70u, 0xf4u,
        [64] = 0xc6u, 0x06u, 0x02u, 0x02u, 0x7bu, 0xcfu
    };
    const lib_u16 vector[] = {0x8040u, 0u};
    const x86_core_machine_executor_config configuration = {
        .memory_bytes = 65536u, .cpu_profile = profile,
        .ticks_per_instruction = 1u
    };
    const x86_core_machine_memory_alias_config alias = {0x3000u, 0x2000u, 16u};
    const x86_core_machine_memory_device_route route = {
        .physical_start = 0x7000u, .bytes = 1u,
        .callbacks = {board_read, board_write, board_query}, .failure = LIB_NULL
    };
    non_pc_board board = {0};
    const x86_core_machine_attachment attachment = {
        .deadline = board_deadline,
        .interrupt_pending = board_interrupt,
        .interrupt_acknowledge = board_acknowledge,
        .after_internal_advance = board_advance,
        .reset_devices = board_reset, .finalize_devices = board_finalize,
        .context = &board
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_machine_observation observation;
    x86_core_machine_memory_route observed_route;
    lib_u64 before, after;
    lib_u16 word = 0u;
    lib_u8 value = 0u;
    lib_u8 advanced = 0u;
    lib_u32 instruction_pointer = 0u;
    lib_i32 failed = 1;
    const char *phase = "construction";

    if (x86_core_machine_neutral_create(&configuration, &machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_install_memory_aliases(machine, &alias, 1u, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_machine_install_memory_device_routes(machine, &route, 1u,
            LIB_NULL, LIB_NULL, &board) != LIB_STATUS_OK ||
        x86_core_machine_register_immutable_rom_mapping(machine, 0x8000u,
            rom, sizeof(rom)) != LIB_STATUS_OK ||
        x86_core_machine_register_immutable_rom_mapping_reset_alias(machine,
            0x8000u, reset_address, 16u) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    phase = "reset and execution";
    if (x86_core_machine_capture_observation(machine, &observation) != LIB_STATUS_OK ||
        observation.lifecycle != X86_CORE_MACHINE_STOPPED || observation.elapsed_ticks != 0u ||
        x86_core_machine_debug_write_register(machine, X86_CORE_CPU_DEBUG_ESP,
            0x6000u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0xc0u, vector, sizeof(vector)) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){64u, 0u},
            &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != 8u || board.value != 0x5au || board.writes != 1u ||
        board.reads != 0u || board.advanced == 0u ||
        x86_core_machine_memory_read(machine, 0x200u, &word, sizeof(word)) != LIB_STATUS_OK ||
        word != 0x1234u) goto done;
    phase = "alias and ROM";
    if (x86_core_machine_memory_write(machine, 0x3000u, &board.value, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x2000u, &value, 1u) != LIB_STATUS_OK ||
        value != 0x5au ||
        x86_core_machine_memory_query(machine, 0x20000u, 1u,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &observed_route) != LIB_STATUS_INTERNAL_ERROR ||
        x86_core_machine_memory_write(machine, 0x8000u, &value, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x8000u, &value, 1u) != LIB_STATUS_OK ||
        value != rom[0]) goto done;
    phase = "observation";
    if (x86_core_machine_get_elapsed_ticks(machine, &before) != LIB_STATUS_OK ||
        x86_core_machine_memory_inspect(machine, 0x7000u, &value, 1u) != LIB_STATUS_OK ||
        value != 0x5au || board.reads != 0u || board.writes != 1u ||
        x86_core_machine_get_elapsed_ticks(machine, &after) != LIB_STATUS_OK || after != before)
        goto done;
    board.timer_armed = LIB_TRUE;
    phase = "halt deadline";
    if (x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
        !advanced || board.timer_armed ||
        x86_core_machine_get_elapsed_ticks(machine, &after) != LIB_STATUS_OK ||
        after != before + 7u || board.advanced != after) goto done;
    board.interrupt = LIB_TRUE;
    phase = "interrupt entry";
    if (x86_core_machine_run(machine, (x86_core_machine_run_budget){64u, 0u},
            &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        board.acknowledgements != 1u || board.interrupt ||
        x86_core_machine_debug_read_register(machine, X86_CORE_CPU_DEBUG_EIP,
            &instruction_pointer) != LIB_STATUS_OK || instruction_pointer != 0x8040u)
        goto done;
    /* Architectural interrupt entry is its own non-retiring run boundary. */
    phase = "interrupt return and reset";
    if (x86_core_machine_run(machine, (x86_core_machine_run_budget){64u, 0u},
            &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != 5u ||
        board.value != 0xa5u || board.writes != 2u ||
        x86_core_machine_memory_read(machine, 0x202u, &value, 1u) != LIB_STATUS_OK ||
        value != 0x7bu ||
        x86_core_machine_capture_observation(machine, &observation) != LIB_STATUS_OK ||
        observation.lifecycle != X86_CORE_MACHINE_PAUSED ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK || board.writes != 0u ||
        board.acknowledgements != 0u || board.advanced != 0u ||
        x86_core_machine_memory_read(machine, 0x202u, &value, 1u) != LIB_STATUS_OK || value != 0u)
        goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("NON-PC-BOARD:%s:profile=%u:reason=%u:retired=%llu:value=%u:writes=%u:irq=%u\n",
        phase, (lib_u32)profile, (lib_u32)result.reason, result.executed,
        board.value, board.writes, board.acknowledgements);
    x86_core_machine_destroy(machine);
    if (board.finalized != 1u) failed = 1;
    return failed;
}

lib_i32 main(void)
{
    if (non_pc_board_run(X86_CORE_CPU_PROFILE_80386,0xfffffff0u) ||
        non_pc_board_run(X86_CORE_CPU_PROFILE_V30,0xffff0u)) return 1;
    lib_c_printf("X86-CORE:NON-PC-BOARD:OK\n");
    return 0;
}
