#include "myraidenii/core/board_interface.h"
#include "myraidenii/core/cop.h"
#include "myraidenii/core/video.h"

struct myraidenii_core_board {
    x86_core_machine *machine;
    myraidenii_core_cop *cop;
    myraidenii_core_board_control_failure_observation control_failure;
    myraidenii_core_video video;
    lib_u8 *program;
    lib_u8 bank_selected;
    lib_u8 main_interrupt_pending;
    lib_u8 sound_command[2];
    lib_u8 sound_response[2];
    lib_u8 sprite_protection_upload[0x20u];
    lib_u8 tile_bank_01;
    lib_u16 sprite_protection_offset;
    lib_u16 sprite_protection_source_segment;
    lib_u16 sprite_protection_destination;
    lib_u16 sprite_protection_x;
    lib_u16 sprite_protection_y;
    lib_u16 sprite_protection_max_x;
    lib_u16 sprite_protection_source;
    lib_u8 sound_command_pending;
    lib_u8 sound_interrupt_pending;
    lib_u16 tile_bank_2;
    lib_u16 dip_switches;
    lib_u16 player_inputs;
    lib_u16 system_inputs;
    lib_u8 coin_inputs;
};

enum {
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_HOLE_START = 0x43eu,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_HOLE_BYTES = 2u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_CONSTANT_START = 0x6c4u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_CONSTANT_BYTES = 2u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_OFFSET_START = 0x6c0u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_SOURCE_START = 0x6c2u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_DESTINATION_START = 0x6c6u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES = 2u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_X_START = 0x6d8u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_Y_START = 0x6dau,
    MYRAIDENII_CORE_SPRITE_PROTECTION_MAX_X_START = 0x6dcu,
    MYRAIDENII_CORE_SPRITE_PROTECTION_SOURCE_LATCH_START = 0x6deu,
    MYRAIDENII_CORE_SPRITE_PROTECTION_WINDOW_START = 0x6d8u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_WINDOW_BYTES = 8u,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_NOOP_START = 0x6c8u,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_NOOP_BYTES = 2u,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_06CA = 0x6cau,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0690 = 0x690u,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0692 = 0x692u,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0696 = 0x696u,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_069A = 0x69au,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_069C = 0x69cu,
    MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES = 2u,
    MYRAIDENII_CORE_TILE_BANK_2_ADDRESS = 0x470u,
    MYRAIDENII_CORE_TILE_BANK_2_BYTES = 2u,
    MYRAIDENII_CORE_TILE_BANK_01_ADDRESS = 0x6ccu,
    MYRAIDENII_CORE_TILE_BANK_01_NOOP_ADDRESS = 0x6cdu,
    MYRAIDENII_CORE_SPRITE_FLAGS_START = 0x6ceu,
    MYRAIDENII_CORE_SPRITE_FLAGS_BYTES = 2u,
    MYRAIDENII_CORE_CRTC_START = 0x600u,
    MYRAIDENII_CORE_CRTC_BYTES = MYRAIDENII_CORE_VIDEO_CRTC_BYTES,
    MYRAIDENII_CORE_CRTC_READABLE_BYTES = MYRAIDENII_CORE_VIDEO_CRTC_READABLE_BYTES,
    MYRAIDENII_CORE_SPRITE_OBJECT_WRITE_START = 0x650u,
    MYRAIDENII_CORE_SPRITE_OBJECT_WRITE_BYTES = 0x50u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_START = 0x6a0u,
    MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_BYTES = 0x20u
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

static lib_status myraidenii_core_board_control_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)physical;
    (void)access;
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    /* Wider mixed accesses must use Core's existing byte-route splitting. */
    return bytes == 1u ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static void myraidenii_core_board_control_failure_note(myraidenii_core_board *board,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (board == LIB_NULL || board->control_failure.valid) return;
    board->control_failure.valid = LIB_TRUE;
    board->control_failure.physical = physical;
    board->control_failure.bytes = (lib_u8)bytes;
    board->control_failure.access = access;
}

static lib_status myraidenii_core_board_control_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    myraidenii_core_board *board = owner;

    myraidenii_core_board_control_failure_note(board, physical, bytes,
        X86_CORE_MACHINE_MEMORY_ACCESS_READ);
    (void)destination;
    (void)observe_only;
    return LIB_STATUS_INVALID_STATE;
}

static lib_status myraidenii_core_board_control_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;

    myraidenii_core_board_control_failure_note(board, physical, bytes,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE);
    (void)source;
    return LIB_STATUS_INVALID_STATE;
}

static lib_bool myraidenii_core_board_selected_control_write_noop_contains(
    lib_u32 physical, lib_uptr bytes)
{
    return myraidenii_core_board_range_contains(MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_HOLE_START,
        MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_HOLE_BYTES, physical, bytes) ||
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SPRITE_PROTECTION_CONSTANT_START,
            MYRAIDENII_CORE_SPRITE_PROTECTION_CONSTANT_BYTES, physical, bytes) ||
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_NOOP_START,
            MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_NOOP_BYTES, physical, bytes) ||
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_06CA,
            1u, physical, bytes) ||
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0690,
            MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES, physical, bytes) ||
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0692,
            MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES, physical, bytes) ||
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0696,
            MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES, physical, bytes) ||
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_069A,
            MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES, physical, bytes) ||
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_069C,
            MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES, physical, bytes);
}

static lib_bool myraidenii_core_board_selected_control_write_noop_overlaps(
    lib_u32 physical, lib_uptr bytes)
{
    for (lib_uptr index = 0u; index < bytes; ++index)
        if (myraidenii_core_board_selected_control_write_noop_contains(physical + (lib_u32)index,
                1u)) return LIB_TRUE;
    return LIB_FALSE;
}

static lib_bool myraidenii_core_board_selected_control_write_noop_covers(
    lib_u32 physical, lib_uptr bytes)
{
    if (bytes == 0u) return LIB_FALSE;
    for (lib_uptr index = 0u; index < bytes; ++index)
        if (!myraidenii_core_board_selected_control_write_noop_contains(physical + (lib_u32)index,
                1u)) return LIB_FALSE;
    return LIB_TRUE;
}

static lib_status myraidenii_core_board_selected_control_write_noop_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE &&
        myraidenii_core_board_selected_control_write_noop_contains(physical, bytes) ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_selected_control_write_noop_write(void *owner,
    lib_u32 physical, lib_uptr source, lib_uptr bytes)
{
    if (owner == LIB_NULL || source == 0u ||
        !myraidenii_core_board_selected_control_write_noop_contains(physical, bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    /* Selected no-op writes have no state or readback. */
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_sprite_flags_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE &&
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SPRITE_FLAGS_START,
            MYRAIDENII_CORE_SPRITE_FLAGS_BYTES, physical, bytes) && bytes == 1u ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

/* L2 Board ownership of the mapped tile-bank-2 register.  Rendering meaning
 * remains a future Video concern; this endpoint only preserves bus state. */
static lib_status myraidenii_core_board_tile_bank_2_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return myraidenii_core_board_range_contains(MYRAIDENII_CORE_TILE_BANK_2_ADDRESS,
        MYRAIDENII_CORE_TILE_BANK_2_BYTES, physical, bytes) &&
        (access == X86_CORE_MACHINE_MEMORY_ACCESS_READ ||
         access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_tile_bank_2_read(void *owner,
    lib_u32 physical, lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const myraidenii_core_board *board = owner;
    lib_u8 *out = (lib_u8 *)destination;
    lib_size offset;

    (void)observe_only;
    if (board == LIB_NULL || destination == 0u ||
        myraidenii_core_board_tile_bank_2_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    offset = (lib_size)(physical - MYRAIDENII_CORE_TILE_BANK_2_ADDRESS);
    for (lib_size index = 0u; index < bytes; ++index)
        out[index] = (lib_u8)(board->tile_bank_2 >> ((offset + index) * 8u));
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_tile_bank_2_write(void *owner,
    lib_u32 physical, lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;
    const lib_u8 *in = (const lib_u8 *)source;
    lib_size offset;

    if (board == LIB_NULL || source == 0u ||
        myraidenii_core_board_tile_bank_2_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    offset = (lib_size)(physical - MYRAIDENII_CORE_TILE_BANK_2_ADDRESS);
    for (lib_size index = 0u; index < bytes; ++index) {
        lib_u16 shift = (lib_u16)((offset + index) * 8u);
        lib_u16 mask = (lib_u16)(0xffu << shift);

        board->tile_bank_2 = (lib_u16)((board->tile_bank_2 & (lib_u16)~mask) |
            ((lib_u16)in[index] << shift));
    }
    return LIB_STATUS_OK;
}

static lib_bool myraidenii_core_board_tile_bank_01_overlaps(lib_u32 physical,
    lib_uptr bytes)
{
    return bytes != 0u && physical <= MYRAIDENII_CORE_TILE_BANK_01_NOOP_ADDRESS &&
        (lib_u64)physical + bytes >
        MYRAIDENII_CORE_TILE_BANK_01_ADDRESS;
}

static lib_bool myraidenii_core_board_sprite_protection_window_overlaps(
    lib_u32 physical, lib_uptr bytes)
{
    return bytes != 0u && physical < MYRAIDENII_CORE_SPRITE_PROTECTION_WINDOW_START +
        MYRAIDENII_CORE_SPRITE_PROTECTION_WINDOW_BYTES && (lib_u64)physical + bytes >
        MYRAIDENII_CORE_SPRITE_PROTECTION_WINDOW_START;
}

static lib_status myraidenii_core_board_tile_bank_01_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return physical == MYRAIDENII_CORE_TILE_BANK_01_ADDRESS && bytes == 1u &&
        access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_tile_bank_01_read(void *owner,
    lib_u32 physical, lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)physical;
    (void)destination;
    (void)bytes;
    (void)observe_only;
    return LIB_STATUS_INVALID_STATE;
}

static lib_status myraidenii_core_board_tile_bank_01_write(void *owner,
    lib_u32 physical, lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;

    if (board == LIB_NULL || source == 0u ||
        myraidenii_core_board_tile_bank_01_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    board->tile_bank_01 = *(const lib_u8 *)source;
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_tile_bank_01_noop_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return physical == MYRAIDENII_CORE_TILE_BANK_01_NOOP_ADDRESS && bytes == 1u &&
        access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_tile_bank_01_noop_read(void *owner,
    lib_u32 physical, lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)physical;
    (void)destination;
    (void)bytes;
    (void)observe_only;
    return LIB_STATUS_INVALID_STATE;
}

static lib_status myraidenii_core_board_tile_bank_01_noop_write(void *owner,
    lib_u32 physical, lib_uptr source, lib_uptr bytes)
{
    if (owner == LIB_NULL || source == 0u ||
        myraidenii_core_board_tile_bank_01_noop_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_sprite_protection_latch_get(
    myraidenii_core_board *board, lib_u32 physical, lib_u16 **out_value,
    lib_bool *out_readable)
{
    lib_u32 first = physical & ~1u;

    if (board == LIB_NULL || out_value == LIB_NULL || out_readable == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    switch (first) {
    case MYRAIDENII_CORE_SPRITE_PROTECTION_OFFSET_START:
        *out_value = &board->sprite_protection_offset;
        *out_readable = LIB_TRUE;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SPRITE_PROTECTION_SOURCE_START:
        *out_value = &board->sprite_protection_source_segment;
        *out_readable = LIB_TRUE;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SPRITE_PROTECTION_DESTINATION_START:
        *out_value = &board->sprite_protection_destination;
        *out_readable = LIB_FALSE;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SPRITE_PROTECTION_X_START:
        *out_value = &board->sprite_protection_x;
        *out_readable = LIB_FALSE;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SPRITE_PROTECTION_Y_START:
        *out_value = &board->sprite_protection_y;
        *out_readable = LIB_FALSE;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SPRITE_PROTECTION_MAX_X_START:
        *out_value = &board->sprite_protection_max_x;
        *out_readable = LIB_TRUE;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SPRITE_PROTECTION_SOURCE_LATCH_START:
        *out_value = &board->sprite_protection_source;
        *out_readable = LIB_FALSE;
        return LIB_STATUS_OK;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_board_sprite_protection_latch_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    myraidenii_core_board *board = owner;
    lib_u16 *value;
    lib_bool readable;
    lib_u32 first = physical & ~1u;

    if (board == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    if (myraidenii_core_board_sprite_protection_latch_get(board, physical, &value, &readable) !=
            LIB_STATUS_OK ||
        !myraidenii_core_board_range_contains(first,
            MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES, physical, bytes) ||
        (access == X86_CORE_MACHINE_MEMORY_ACCESS_READ && !readable)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    (void)value;
    return access == X86_CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_sprite_protection_latch_read(void *owner,
    lib_u32 physical, lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    myraidenii_core_board *board = owner;
    lib_u16 *value;
    lib_bool readable;
    lib_u32 first = physical & ~1u;

    (void)observe_only;
    if (destination == 0u ||
        myraidenii_core_board_sprite_protection_latch_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ) != LIB_STATUS_OK ||
        myraidenii_core_board_sprite_protection_latch_get(board, physical, &value, &readable) !=
            LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (lib_uptr index = 0u; index < bytes; ++index)
        ((lib_u8 *)destination)[index] = (lib_u8)(*value >> ((physical + index - first) * 8u));
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_sprite_protection_latch_write(void *owner,
    lib_u32 physical, lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;
    lib_u16 *value;
    lib_bool readable;
    lib_u32 first = physical & ~1u;

    if (source == 0u ||
        myraidenii_core_board_sprite_protection_latch_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK ||
        myraidenii_core_board_sprite_protection_latch_get(board, physical, &value, &readable) !=
            LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (lib_uptr index = 0u; index < bytes; ++index) {
        lib_u16 shift = (lib_u16)((physical + index - first) * 8u);
        lib_u16 mask = (lib_u16)(0xffu << shift);

        *value = (lib_u16)((*value & (lib_u16)~mask) |
            ((lib_u16)((const lib_u8 *)source)[index] << shift));
    }
    return LIB_STATUS_OK;
}

/* L2 object ownership at the exact selected-set write. No object register
 * meaning, readback or frame effect is established yet. */
static lib_status myraidenii_core_board_sprite_object_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return myraidenii_core_board_range_contains(MYRAIDENII_CORE_SPRITE_OBJECT_WRITE_START,
        MYRAIDENII_CORE_SPRITE_OBJECT_WRITE_BYTES, physical, bytes) &&
        !myraidenii_core_board_selected_control_write_noop_covers(physical, bytes) &&
        !myraidenii_core_board_tile_bank_01_overlaps(physical, bytes) &&
        !myraidenii_core_board_sprite_protection_window_overlaps(physical, bytes) &&
        (access == X86_CORE_MACHINE_MEMORY_ACCESS_READ ||
         access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_sprite_object_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    if (owner == LIB_NULL || destination == 0u ||
        myraidenii_core_board_sprite_object_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    (void)observe_only;
    return LIB_STATUS_INVALID_STATE;
}

static lib_status myraidenii_core_board_sprite_object_write(void *owner,
    lib_u32 physical, lib_uptr source, lib_uptr bytes)
{
    if (owner == LIB_NULL || source == 0u ||
        myraidenii_core_board_sprite_object_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return myraidenii_core_board_selected_control_write_noop_overlaps(physical, bytes) ?
        LIB_STATUS_INVALID_STATE : LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_sprite_flags_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)physical;
    (void)destination;
    (void)bytes;
    (void)observe_only;
    return LIB_STATUS_INVALID_STATE;
}

static lib_status myraidenii_core_board_sprite_flags_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;

    if (board == LIB_NULL || source == 0u || bytes != 1u ||
        !myraidenii_core_board_range_contains(MYRAIDENII_CORE_SPRITE_FLAGS_START,
            MYRAIDENII_CORE_SPRITE_FLAGS_BYTES, physical, bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return myraidenii_core_video_sprite_flags_write(&board->video,
        (lib_u8)(physical - MYRAIDENII_CORE_SPRITE_FLAGS_START), *(const lib_u8 *)source);
}

static lib_status myraidenii_core_board_crtc_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    if (!myraidenii_core_board_range_contains(MYRAIDENII_CORE_CRTC_START,
            MYRAIDENII_CORE_CRTC_BYTES, physical, bytes)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if (access != X86_CORE_MACHINE_MEMORY_ACCESS_WRITE &&
        !myraidenii_core_board_range_contains(MYRAIDENII_CORE_CRTC_START,
            MYRAIDENII_CORE_CRTC_READABLE_BYTES, physical, bytes)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_crtc_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const myraidenii_core_board *board = owner;

    (void)observe_only;
    if (board == LIB_NULL || destination == 0u ||
        myraidenii_core_board_crtc_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (lib_uptr index = 0u; index < bytes; ++index) {
        lib_status status = myraidenii_core_video_crtc_read(&board->video,
            (lib_u8)(physical + index - MYRAIDENII_CORE_CRTC_START),
            (lib_u8 *)destination + index);

        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_crtc_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;

    if (board == LIB_NULL || source == 0u ||
        myraidenii_core_board_crtc_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (lib_uptr index = 0u; index < bytes; ++index) {
        lib_status status = myraidenii_core_video_crtc_write(&board->video,
            (lib_u8)(physical + index - MYRAIDENII_CORE_CRTC_START),
            ((const lib_u8 *)source)[index]);

        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_sprite_protection_upload_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE &&
        myraidenii_core_board_range_contains(MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_START,
            MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_BYTES, physical, bytes) ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status myraidenii_core_board_sprite_protection_upload_read(void *owner,
    lib_u32 physical, lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)physical;
    (void)destination;
    (void)bytes;
    (void)observe_only;
    return LIB_STATUS_INVALID_STATE;
}

static lib_status myraidenii_core_board_sprite_protection_upload_write(void *owner,
    lib_u32 physical, lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;

    if (board == LIB_NULL || source == 0u ||
        !myraidenii_core_board_range_contains(MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_START,
            MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_BYTES, physical, bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_copy(board->sprite_protection_upload +
        physical - MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_START, (const void *)source, bytes);
    return LIB_STATUS_OK;
}

static lib_bool myraidenii_core_board_interrupt_pending(void *owner)
{
    const myraidenii_core_board *board = owner;

    return board != LIB_NULL && board->main_interrupt_pending;
}

static void myraidenii_core_board_deadline(void *owner, lib_u64 now,
    x86_core_machine_attachment_deadline_observation *out_observation)
{
    const myraidenii_core_board *board = owner;

    (void)now;
    out_observation->source_ticks = myraidenii_core_video_ticks_until_vblank(&board->video);
}

static void myraidenii_core_board_advance(void *owner, lib_u64 ticks, lib_u64 due_tick)
{
    myraidenii_core_board *board = owner;

    (void)due_tick;
    if (myraidenii_core_video_advance(&board->video, ticks))
        board->main_interrupt_pending = LIB_TRUE;
    lib_status status = myraidenii_core_cop_advance(board->cop);

    if (status != LIB_STATUS_OK) {
        (void)x86_core_machine_report_fault(board->machine, (lib_u32)status);
    }
}

static lib_u8 myraidenii_core_board_interrupt_acknowledge(void *owner)
{
    myraidenii_core_board *board = owner;

    if (board != LIB_NULL) board->main_interrupt_pending = LIB_FALSE;
    return MYRAIDENII_CORE_MAIN_INTERRUPT_VECTOR;
}

static lib_status myraidenii_core_board_input_value(const myraidenii_core_board *board,
    lib_u32 physical, lib_u16 *out_value)
{
    if (board == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (physical) {
    case MYRAIDENII_CORE_DIP_SWITCHES_ADDRESS:
        *out_value = board->dip_switches;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_PLAYER_INPUTS_ADDRESS:
        *out_value = board->player_inputs;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SYSTEM_INPUTS_ADDRESS:
        *out_value = board->system_inputs;
        return LIB_STATUS_OK;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_board_input_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    lib_u16 value;
    lib_u32 first = physical & ~1u;

    (void)access;
    if (bytes == 0u || bytes > 2u || physical + bytes > first + 2u)
        return LIB_STATUS_UNSUPPORTED;
    return myraidenii_core_board_input_value(owner, first, &value);
}

static lib_status myraidenii_core_board_input_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    lib_u16 value;
    lib_u32 first = physical & ~1u;
    lib_status status;

    (void)observe_only;
    if (destination == 0u || bytes == 0u || bytes > 2u || physical + bytes > first + 2u)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = myraidenii_core_board_input_value(owner, first, &value);
    if (status != LIB_STATUS_OK) return status;
    for (lib_uptr index = 0u; index < bytes; ++index)
        ((lib_u8 *)destination)[index] = (lib_u8)(value >> ((physical + index - first) * 8u));
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_input_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)source;
    return myraidenii_core_board_input_query(owner, physical, bytes,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE);
}

static lib_status myraidenii_core_board_sound_query(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)owner;
    (void)access;
    if (bytes == 0u || bytes > 2u || physical < MYRAIDENII_CORE_SOUND_START ||
        physical >= MYRAIDENII_CORE_SOUND_START + MYRAIDENII_CORE_SOUND_BYTES ||
        physical + bytes > MYRAIDENII_CORE_SOUND_START + MYRAIDENII_CORE_SOUND_BYTES)
        return LIB_STATUS_UNSUPPORTED;
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_sound_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const myraidenii_core_board *board = owner;

    (void)observe_only;
    if (board == LIB_NULL || destination == 0u ||
        myraidenii_core_board_sound_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_uptr index = 0u; index < bytes; ++index) {
        lib_u8 offset = (lib_u8)((physical + index - MYRAIDENII_CORE_SOUND_START) >> 1u);
        lib_u8 value = 0xffu;

        if (((physical + index) & 1u) == 0u) {
            if (offset == 2u || offset == 3u) value = board->sound_response[offset - 2u];
            else if (offset == 5u) value = board->sound_command_pending ? 1u : 0u;
        }
        ((lib_u8 *)destination)[index] = value;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_board_sound_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_board *board = owner;

    if (board == LIB_NULL || source == 0u ||
        myraidenii_core_board_sound_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_uptr index = 0u; index < bytes; ++index) {
        lib_u8 offset = (lib_u8)((physical + index - MYRAIDENII_CORE_SOUND_START) >> 1u);

        if (((physical + index) & 1u) == 0u) {
            if (offset < 2u) board->sound_command[offset] = ((const lib_u8 *)source)[index];
            else if (offset == 4u) board->sound_interrupt_pending = LIB_TRUE;
            else if (offset == 6u) board->sound_command_pending = LIB_TRUE;
        }
    }
    return LIB_STATUS_OK;
}

static void myraidenii_core_board_reset(void *owner)
{
    myraidenii_core_board *board = owner;

    if (board != LIB_NULL) {
        board->bank_selected = LIB_FALSE;
        board->main_interrupt_pending = LIB_FALSE;
        board->sound_command[0] = 0u;
        board->sound_command[1] = 0u;
        board->sound_response[0] = 0u;
        board->sound_response[1] = 0u;
        board->tile_bank_01 = 0u;
        board->sprite_protection_offset = 0u;
        board->sprite_protection_source_segment = 0u;
        board->sprite_protection_destination = 0u;
        board->sprite_protection_x = 0u;
        board->sprite_protection_y = 0u;
        board->sprite_protection_max_x = 0u;
        board->sprite_protection_source = 0u;
        board->sound_command_pending = LIB_FALSE;
        board->sound_interrupt_pending = LIB_FALSE;
        board->tile_bank_2 = 0u;
        board->dip_switches = 0xffffu;
        board->player_inputs = 0xffffu;
        board->system_inputs = 0xffffu;
        board->coin_inputs = 0xffu;
        lib_memory_set(board->sprite_protection_upload, 0, sizeof(board->sprite_protection_upload));
        lib_memory_set(&board->control_failure, 0, sizeof(board->control_failure));
        myraidenii_core_cop_reset(board->cop);
        myraidenii_core_video_reset(&board->video);
    }
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
        .cpu_profile = X86_CORE_CPU_PROFILE_V30,
        .time_axis = {X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 16000000u}
    };
    x86_core_machine_attachment attachment = {
        .deadline = myraidenii_core_board_deadline,
        .before_internal_advance = myraidenii_core_board_advance,
        .interrupt_pending = myraidenii_core_board_interrupt_pending,
        .interrupt_acknowledge = myraidenii_core_board_interrupt_acknowledge,
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
    const x86_core_machine_memory_device_callbacks input_callbacks = {
        myraidenii_core_board_input_read, myraidenii_core_board_input_write,
        myraidenii_core_board_input_query
    };
    const x86_core_machine_memory_device_callbacks sound_callbacks = {
        myraidenii_core_board_sound_read, myraidenii_core_board_sound_write,
        myraidenii_core_board_sound_query
    };
    const x86_core_machine_memory_device_callbacks selected_control_write_noop_callbacks = {
        myraidenii_core_board_control_read, myraidenii_core_board_selected_control_write_noop_write,
        myraidenii_core_board_selected_control_write_noop_query
    };
    const x86_core_machine_memory_device_callbacks sprite_flags_callbacks = {
        myraidenii_core_board_sprite_flags_read, myraidenii_core_board_sprite_flags_write,
        myraidenii_core_board_sprite_flags_query
    };
    const x86_core_machine_memory_device_callbacks tile_bank_2_callbacks = {
        myraidenii_core_board_tile_bank_2_read, myraidenii_core_board_tile_bank_2_write,
        myraidenii_core_board_tile_bank_2_query
    };
    const x86_core_machine_memory_device_callbacks tile_bank_01_callbacks = {
        myraidenii_core_board_tile_bank_01_read, myraidenii_core_board_tile_bank_01_write,
        myraidenii_core_board_tile_bank_01_query
    };
    const x86_core_machine_memory_device_callbacks tile_bank_01_noop_callbacks = {
        myraidenii_core_board_tile_bank_01_noop_read,
        myraidenii_core_board_tile_bank_01_noop_write,
        myraidenii_core_board_tile_bank_01_noop_query
    };
    const x86_core_machine_memory_device_callbacks sprite_protection_latch_callbacks = {
        myraidenii_core_board_sprite_protection_latch_read,
        myraidenii_core_board_sprite_protection_latch_write,
        myraidenii_core_board_sprite_protection_latch_query
    };
    const x86_core_machine_memory_device_callbacks sprite_object_write_callbacks = {
        myraidenii_core_board_sprite_object_read, myraidenii_core_board_sprite_object_write,
        myraidenii_core_board_sprite_object_query
    };
    const x86_core_machine_memory_device_callbacks crtc_callbacks = {
        myraidenii_core_board_crtc_read, myraidenii_core_board_crtc_write,
        myraidenii_core_board_crtc_query
    };
    const x86_core_machine_memory_device_callbacks sprite_protection_upload_callbacks = {
        myraidenii_core_board_sprite_protection_upload_read,
        myraidenii_core_board_sprite_protection_upload_write,
        myraidenii_core_board_sprite_protection_upload_query
    };
    myraidenii_core_board *board;
    x86_core_machine_memory_device_route routes[30] = {
        {.physical_start = MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            .bytes = MYRAIDENII_CORE_PROGRAM_WINDOW_BYTES,
            .callbacks = bank_callbacks},
        {.physical_start = MYRAIDENII_CORE_PROGRAM_BANK_SELECT_ADDRESS,
            .bytes = 1u, .callbacks = select_callbacks},
        {.physical_start = MYRAIDENII_CORE_SOUND_START,
            .bytes = MYRAIDENII_CORE_SOUND_BYTES, .callbacks = sound_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_DIP_SWITCHES_ADDRESS,
            .bytes = 2u, .callbacks = input_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_PLAYER_INPUTS_ADDRESS,
            .bytes = 2u, .callbacks = input_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SYSTEM_INPUTS_ADDRESS,
            .bytes = 2u, .callbacks = input_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_TILE_BANK_2_ADDRESS,
            .bytes = MYRAIDENII_CORE_TILE_BANK_2_BYTES, .callbacks = tile_bank_2_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_HOLE_START,
            .bytes = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_HOLE_BYTES,
            .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_CONSTANT_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_CONSTANT_BYTES,
            .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_OFFSET_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES,
            .callbacks = sprite_protection_latch_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_SOURCE_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES,
            .callbacks = sprite_protection_latch_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_DESTINATION_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES,
            .callbacks = sprite_protection_latch_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_X_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES,
            .callbacks = sprite_protection_latch_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_Y_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES,
            .callbacks = sprite_protection_latch_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_MAX_X_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES,
            .callbacks = sprite_protection_latch_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_SOURCE_LATCH_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_LATCH_BYTES,
            .callbacks = sprite_protection_latch_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0690,
            .bytes = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES,
            .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0692,
            .bytes = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES,
            .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_0696,
            .bytes = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES,
            .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_069A,
            .bytes = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES,
            .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_069C,
            .bytes = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_BYTES,
            .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_NOOP_START,
            .bytes = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_NOOP_BYTES,
            .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SELECTED_CONTROL_WRITE_ADDRESS_06CA,
            .bytes = 1u, .callbacks = selected_control_write_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_FLAGS_START,
            .bytes = MYRAIDENII_CORE_SPRITE_FLAGS_BYTES,
            .callbacks = sprite_flags_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_TILE_BANK_01_ADDRESS,
            .bytes = 1u, .callbacks = tile_bank_01_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_TILE_BANK_01_NOOP_ADDRESS,
            .bytes = 1u, .callbacks = tile_bank_01_noop_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_OBJECT_WRITE_START,
            .bytes = MYRAIDENII_CORE_SPRITE_OBJECT_WRITE_BYTES,
            .callbacks = sprite_object_write_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_CRTC_START,
            .bytes = MYRAIDENII_CORE_CRTC_BYTES, .callbacks = crtc_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_START,
            .bytes = MYRAIDENII_CORE_SPRITE_PROTECTION_UPLOAD_BYTES,
            .callbacks = sprite_protection_upload_callbacks,
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT},
        {.physical_start = 0x400u, .bytes = 0x400u,
            .callbacks = {myraidenii_core_board_control_read,
                myraidenii_core_board_control_write, myraidenii_core_board_control_query},
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_FALLBACK}
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
        status = myraidenii_core_cop_create(&board->video, &board->cop);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_install_memory_device_routes(board->machine, routes,
            sizeof(routes) / sizeof(routes[0]), LIB_NULL, LIB_NULL, board);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_install(board->cop, board->machine);
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
        myraidenii_core_cop_destroy(board->cop);
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
    myraidenii_core_cop_destroy(board->cop);
    lib_release(board->program);
    lib_release(board);
}

x86_core_machine *myraidenii_core_board_machine_get(myraidenii_core_board *board)
{
    return board == LIB_NULL ? LIB_NULL : board->machine;
}

lib_status myraidenii_core_board_cop_failure_get(const myraidenii_core_board *board,
    myraidenii_core_cop_failure_observation *out_observation)
{
    if (board == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return myraidenii_core_cop_failure_get(board->cop, out_observation);
}

lib_status myraidenii_core_board_control_failure_get(const myraidenii_core_board *board,
    myraidenii_core_board_control_failure_observation *out_observation)
{
    if (board == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_observation = board->control_failure;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_set_control(myraidenii_core_board *board,
    myraidenii_core_board_control control, lib_bool pressed)
{
    lib_u16 *register_value;
    lib_u16 mask;

    if (board == LIB_NULL || control >= MYRAIDENII_CORE_BOARD_CONTROL_COUNT)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (control <= MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_BUTTON_2) {
        register_value = &board->player_inputs;
        mask = (lib_u16)(1u << control);
    } else if (control <= MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_2_BUTTON_2) {
        register_value = &board->player_inputs;
        mask = (lib_u16)(1u << (control + 2u));
    } else if (control <= MYRAIDENII_CORE_BOARD_CONTROL_SERVICE) {
        register_value = &board->system_inputs;
        mask = (lib_u16)(1u << (control == MYRAIDENII_CORE_BOARD_CONTROL_SERVICE ? 3u :
            control - MYRAIDENII_CORE_BOARD_CONTROL_START_1));
    } else {
        lib_u8 coin_mask = (lib_u8)(1u <<
            (control - MYRAIDENII_CORE_BOARD_CONTROL_COIN_1));

        if (pressed) board->coin_inputs &= (lib_u8)~coin_mask;
        else board->coin_inputs |= coin_mask;
        return LIB_STATUS_OK;
    }
    if (pressed) *register_value &= (lib_u16)~mask;
    else *register_value |= mask;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_set_dip_switches(myraidenii_core_board *board,
    lib_u16 value)
{
    if (board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    board->dip_switches = value;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_get_coin_inputs(const myraidenii_core_board *board,
    lib_u8 *out_value)
{
    if (board == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = board->coin_inputs;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_request_main_interrupt(myraidenii_core_board *board)
{
    if (board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    board->main_interrupt_pending = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_sound_command_get(const myraidenii_core_board *board,
    lib_u8 index, lib_u8 *out_value)
{
    if (board == LIB_NULL || out_value == LIB_NULL || index >= 2u)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = board->sound_command[index];
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_sound_command_acknowledge(myraidenii_core_board *board)
{
    if (board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    board->sound_command_pending = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_sound_interrupt_is_pending(
    const myraidenii_core_board *board, lib_bool *out_pending)
{
    if (board == LIB_NULL || out_pending == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_pending = board->sound_interrupt_pending;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_sound_interrupt_acknowledge(myraidenii_core_board *board)
{
    if (board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    board->sound_interrupt_pending = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_sound_response_set(myraidenii_core_board *board,
    lib_u8 index, lib_u8 value)
{
    if (board == LIB_NULL || index >= 2u) return LIB_STATUS_INVALID_ARGUMENT;
    board->sound_response[index] = value;
    return LIB_STATUS_OK;
}
