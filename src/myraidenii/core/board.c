#include "myraidenii/core/board_interface.h"

struct myraidenii_core_board {
    x86_core_machine *machine;
    lib_u8 *program;
    lib_u8 bank_selected;
};

static lib_bool myraidenii_core_board_range_contains(lib_u32 first,
    lib_size capacity, lib_u32 physical, lib_uptr bytes)
{
    return bytes != 0u && physical >= first &&
        (lib_u64)physical - first + bytes <= capacity;
}

static lib_status myraidenii_core_board_bank_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    const myraidenii_core_board *board = owner;

    (void)access;
    if (board == LIB_NULL || board->program == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return myraidenii_core_board_range_contains(MYRAIDENII_CORE_PROGRAM_WINDOW_START,
        MYRAIDENII_CORE_PROGRAM_WINDOW_BYTES, physical, bytes) ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_bank_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const myraidenii_core_board *board = owner;
    lib_size offset;
    lib_size source;

    (void)observe_only;
    if (board == LIB_NULL || board->program == LIB_NULL || destination == 0u ||
        !myraidenii_core_board_range_contains(MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            MYRAIDENII_CORE_PROGRAM_WINDOW_BYTES, physical, bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    offset = (lib_size)((lib_u64)physical - MYRAIDENII_CORE_PROGRAM_WINDOW_START);
    source = board->bank_selected ? offset : MYRAIDENII_CORE_PROGRAM_WINDOW_BYTES + offset;
    lib_memory_copy((void *)destination, board->program + source, bytes);
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_bank_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    const myraidenii_core_board *board = owner;

    if (board == LIB_NULL || source == 0u ||
        !myraidenii_core_board_range_contains(MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            MYRAIDENII_CORE_PROGRAM_WINDOW_BYTES, physical, bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    /* Decoded program ROM consumes writes without changing its immutable bytes. */
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_select_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return physical == MYRAIDENII_CORE_PROGRAM_BANK_SELECT_ADDRESS && bytes == 1u &&
        access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_select_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)physical;
    (void)destination;
    (void)bytes;
    (void)observe_only;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_select_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;

    if (board == LIB_NULL || source == 0u ||
        physical != MYRAIDENII_CORE_PROGRAM_BANK_SELECT_ADDRESS || bytes != 1u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    board->bank_selected = (*(const lib_u8 *)source & 0x80u) != 0u ? LIB_TRUE : LIB_FALSE;
    return LIB_STATUS_OK;
}

static void myraidenii_core_board_reset(void *owner)
{
    myraidenii_core_board *board = owner;

    if (board != LIB_NULL) board->bank_selected = LIB_FALSE;
}

static void myraidenii_core_board_finalize(void *owner)
{
    (void)owner;
}

lib_status myraidenii_core_board_create(const myraidenii_core_board_config *config,
    myraidenii_core_board **out_board)
{
    const x86_core_machine_executor_config machine_config = {
        .memory_bytes = 0x100000u,
        .cpu_profile = X86_CORE_CPU_PROFILE_V30
    };
    x86_core_machine_attachment attachment = {
        .reset_devices = myraidenii_core_board_reset,
        .finalize_devices = myraidenii_core_board_finalize
    };
    const x86_core_machine_memory_device_callbacks bank_callbacks = {
        myraidenii_core_board_bank_read, myraidenii_core_board_bank_write,
        myraidenii_core_board_bank_query
    };
    const x86_core_machine_memory_device_callbacks select_callbacks = {
        myraidenii_core_board_select_read, myraidenii_core_board_select_write,
        myraidenii_core_board_select_query
    };
    myraidenii_core_board *board;
    x86_core_machine_memory_device_route routes[2] = {
        {.physical_start = MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            .bytes = MYRAIDENII_CORE_PROGRAM_WINDOW_BYTES,
            .callbacks = bank_callbacks},
        {.physical_start = MYRAIDENII_CORE_PROGRAM_BANK_SELECT_ADDRESS,
            .bytes = 1u, .callbacks = select_callbacks}
    };
    lib_size index;
    lib_status status;

    if (out_board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_board = LIB_NULL;
    if (config == LIB_NULL || config->program_even == LIB_NULL ||
        config->program_odd == LIB_NULL ||
        config->program_lane_bytes != MYRAIDENII_CORE_PROGRAM_LANE_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    board = lib_allocate_zero(1u, sizeof(*board));
    if (board == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    board->program = lib_allocate(MYRAIDENII_CORE_PROGRAM_LANE_BYTES * 2u);
    if (board->program == LIB_NULL) {
        lib_release(board);
        return LIB_STATUS_NO_MEMORY;
    }
    for (index = 0u; index < MYRAIDENII_CORE_PROGRAM_LANE_BYTES; ++index) {
        board->program[index * 2u] = config->program_even[index];
        board->program[index * 2u + 1u] = config->program_odd[index];
    }
    status = x86_core_machine_neutral_create(&machine_config, &board->machine);
    if (status == LIB_STATUS_OK) {
        attachment.context = board;
        status = x86_core_machine_bind_attachment(board->machine, &attachment);
    }
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_install_memory_device_routes(board->machine, routes,
            2u, LIB_NULL, LIB_NULL, board);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_register_immutable_rom_mapping(board->machine,
            MYRAIDENII_CORE_STATIC_PROGRAM_START,
            board->program + MYRAIDENII_CORE_STATIC_PROGRAM_START,
            MYRAIDENII_CORE_STATIC_PROGRAM_BYTES);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_freeze_execution_providers(board->machine);
    if (status == LIB_STATUS_OK) status = x86_core_machine_reset(board->machine);
    if (status != LIB_STATUS_OK) {
        x86_core_machine_destroy(board->machine);
        lib_release(board->program);
        lib_release(board);
        return status;
    }
    *out_board = board;
    return LIB_STATUS_OK;
}

void myraidenii_core_board_destroy(myraidenii_core_board *board)
{
    if (board == LIB_NULL) return;
    x86_core_machine_destroy(board->machine);
    lib_release(board->program);
    lib_release(board);
}

x86_core_machine *myraidenii_core_board_machine_get(myraidenii_core_board *board)
{
    return board == LIB_NULL ? LIB_NULL : board->machine;
}
