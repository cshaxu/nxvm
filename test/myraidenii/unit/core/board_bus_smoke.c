#include "myraidenii/core/board_interface.h"
#include "myraidenii/core/video.h"
#include "lib/types/file.h"
#include "x86-core/machine/debug_interface.h"

static lib_u8 program_even[MYRAIDENII_CORE_PROGRAM_LANE_BYTES];
static lib_u8 program_odd[MYRAIDENII_CORE_PROGRAM_LANE_BYTES];

static lib_i32 board_invalid_arguments(void)
{
    myraidenii_core_board *board = (myraidenii_core_board *)1;
    const myraidenii_core_board_config config = {
        .program_even = program_even,
        .program_odd = program_odd,
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES - 1u
    };

    return myraidenii_core_board_create(LIB_NULL, &board) != LIB_STATUS_INVALID_ARGUMENT ||
        board != LIB_NULL || myraidenii_core_board_create(&config, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || board != LIB_NULL ||
        myraidenii_core_board_create(&config, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
}

static lib_i32 board_program_and_ram_routes(void)
{
    const myraidenii_core_board_config config = {
        .program_even = program_even,
        .program_odd = program_odd,
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const lib_u32 ram_addresses[] = {0x00000u, 0x00800u, 0x0c000u, 0x10000u, 0x1f000u};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_memory_route route;
    x86_core_cpu_profile profile;
    lib_u8 value[2] = {0u, 0u};
    lib_u8 write = 0x80u;
    lib_i32 failed = 1;
    lib_size index;

    lib_memory_set(program_even, 0, sizeof(program_even));
    lib_memory_set(program_odd, 0, sizeof(program_odd));
    program_even[0x00000u] = 0x10u;
    program_odd[0x00000u] = 0x11u;
    program_even[0x10000u] = 0x20u;
    program_odd[0x10000u] = 0x21u;
    program_even[0x0ffffu] = 0x50u;
    program_odd[0x0ffffu] = 0x51u;
    program_even[0x20000u] = 0x40u;
    program_odd[0x20000u] = 0x41u;
    program_even[0x1ffffu] = 0x30u;
    program_odd[0x1ffffu] = 0x31u;
    program_even[MYRAIDENII_CORE_PROGRAM_LANE_BYTES - 1u] = 0xfeu;
    program_odd[MYRAIDENII_CORE_PROGRAM_LANE_BYTES - 1u] = 0xffu;
    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK ||
        (machine = myraidenii_core_board_machine_get(board)) == LIB_NULL ||
        x86_core_machine_get_cpu_profile(machine, &profile) != LIB_STATUS_OK ||
        profile != X86_CORE_CPU_PROFILE_V30)
        goto done;

    if (x86_core_machine_memory_query(machine, MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            sizeof(value), X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            value, sizeof(value)) != LIB_STATUS_OK || value[0] != 0x20u || value[1] != 0x21u ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_STATIC_PROGRAM_START,
            value, sizeof(value)) != LIB_STATUS_OK || value[0] != 0x40u || value[1] != 0x41u ||
        x86_core_machine_memory_read(machine, 0x3fffeu, value, sizeof(value)) !=
            LIB_STATUS_OK || value[0] != 0x30u || value[1] != 0x31u ||
        x86_core_machine_memory_read(machine, 0xffffeu, value, sizeof(value)) !=
            LIB_STATUS_OK || value[0] != 0xfeu || value[1] != 0xffu)
        goto done;

    value[0] = 0xa5u;
    if (x86_core_machine_memory_write(machine, MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            value, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, MYRAIDENII_CORE_STATIC_PROGRAM_START,
            value, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            value, 1u) != LIB_STATUS_OK || value[0] != 0x20u ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_STATIC_PROGRAM_START,
            value, 1u) != LIB_STATUS_OK || value[0] != 0x40u ||
        x86_core_machine_memory_write(machine, MYRAIDENII_CORE_PROGRAM_BANK_SELECT_ADDRESS,
            &write, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            value, sizeof(value)) != LIB_STATUS_OK || value[0] != 0x10u || value[1] != 0x11u ||
        x86_core_machine_memory_read(machine, 0x3fffeu, value, sizeof(value)) !=
            LIB_STATUS_OK || value[0] != 0x50u || value[1] != 0x51u)
        goto done;

    for (index = 0u; index < sizeof(ram_addresses) / sizeof(ram_addresses[0]); ++index) {
        write = (lib_u8)(0x30u + index);
        if (x86_core_machine_memory_query(machine, ram_addresses[index], 1u,
                X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
            route != X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM ||
            x86_core_machine_memory_write(machine, ram_addresses[index], &write, 1u) !=
                LIB_STATUS_OK || x86_core_machine_memory_read(machine, ram_addresses[index],
                value, 1u) != LIB_STATUS_OK || value[0] != write)
            goto done;
    }
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_PROGRAM_WINDOW_START,
            value, sizeof(value)) != LIB_STATUS_OK || value[0] != 0x20u || value[1] != 0x21u)
        goto done;
    failed = 0;
done:
    myraidenii_core_board_destroy(board);
    return failed;
}

static lib_i32 board_interrupt_input_and_sound_routes(void)
{
    const myraidenii_core_board_config config = {
        .program_even = program_even,
        .program_odd = program_odd,
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = {[X86_CORE_CPU_DEBUG_CS] = 0x2000u,
            [X86_CORE_CPU_DEBUG_EIP] = 0u, [X86_CORE_CPU_DEBUG_ESP] = 0x6000u}
    };
    const lib_u8 guest[] = {0xfbu, 0x90u, 0xf4u};
    const lib_u8 handler[] = {0x46u, 0xcfu};
    const lib_u8 vector[] = {0x00u, 0x02u, 0x00u, 0x20u};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot cpu;
    lib_u16 word = 0u;
    lib_u8 byte = 0u;
    lib_u8 coin = 0u;
    lib_bool sound_interrupt = LIB_FALSE;
    lib_i32 failed = 1;

    lib_memory_set(program_even, 0, sizeof(program_even));
    lib_memory_set(program_odd, 0, sizeof(program_odd));
    for (lib_size index = 0u; index < sizeof(guest); ++index)
        (index & 1u ? program_odd : program_even)[0x10000u + index / 2u] = guest[index];
    for (lib_size index = 0u; index < sizeof(handler); ++index)
        (index & 1u ? program_odd : program_even)[0x10100u + index / 2u] = handler[index];
    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK ||
        (machine = myraidenii_core_board_machine_get(board)) == LIB_NULL ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, MYRAIDENII_CORE_MAIN_INTERRUPT_VECTOR * 4u,
            vector, sizeof(vector)) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){3u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT || result.executed != 3u ||
        myraidenii_core_board_request_main_interrupt(board) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) !=
            LIB_STATUS_OK || result.executed != 0u ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        cpu.cs.selector != 0x2000u || cpu.eip != 0x200u ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){2u, 0u}, &result) !=
            LIB_STATUS_OK || result.executed != 2u ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK || cpu.esi != 1u)
        goto done;

    if (myraidenii_core_board_set_dip_switches(board, 0x5aa5u) != LIB_STATUS_OK ||
        myraidenii_core_board_set_control(board, MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_UP,
            LIB_TRUE) != LIB_STATUS_OK ||
        myraidenii_core_board_set_control(board, MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_2_BUTTON_2,
            LIB_TRUE) != LIB_STATUS_OK ||
        myraidenii_core_board_set_control(board, MYRAIDENII_CORE_BOARD_CONTROL_START_1,
            LIB_TRUE) != LIB_STATUS_OK ||
        myraidenii_core_board_set_control(board, MYRAIDENII_CORE_BOARD_CONTROL_SERVICE,
            LIB_TRUE) != LIB_STATUS_OK ||
        myraidenii_core_board_set_control(board, MYRAIDENII_CORE_BOARD_CONTROL_COIN_2,
            LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_DIP_SWITCHES_ADDRESS, &word,
            sizeof(word)) != LIB_STATUS_OK || word != 0x5aa5u ||
        (word = 0u, x86_core_machine_memory_write(machine,
            MYRAIDENII_CORE_DIP_SWITCHES_ADDRESS, &word, sizeof(word)) != LIB_STATUS_OK) ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_DIP_SWITCHES_ADDRESS, &word,
            sizeof(word)) != LIB_STATUS_OK || word != 0x5aa5u ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_PLAYER_INPUTS_ADDRESS, &word,
            sizeof(word)) != LIB_STATUS_OK || word != 0xdffeu ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_SYSTEM_INPUTS_ADDRESS, &word,
            sizeof(word)) != LIB_STATUS_OK || word != 0xfff6u ||
        myraidenii_core_board_get_coin_inputs(board, &coin) != LIB_STATUS_OK || coin != 0xfdu)
        goto done;

    byte = 0x12u;
    if (x86_core_machine_memory_write(machine, MYRAIDENII_CORE_SOUND_START, &byte, 1u) !=
            LIB_STATUS_OK || (byte = 0x34u,
            x86_core_machine_memory_write(machine, MYRAIDENII_CORE_SOUND_START + 2u, &byte, 1u) !=
                LIB_STATUS_OK) || (byte = 0u,
            x86_core_machine_memory_write(machine, MYRAIDENII_CORE_SOUND_START + 8u, &byte, 1u) !=
                LIB_STATUS_OK) ||
        myraidenii_core_board_sound_interrupt_is_pending(board, &sound_interrupt) != LIB_STATUS_OK ||
        !sound_interrupt || myraidenii_core_board_sound_interrupt_acknowledge(board) != LIB_STATUS_OK ||
        myraidenii_core_board_sound_interrupt_is_pending(board, &sound_interrupt) != LIB_STATUS_OK ||
        sound_interrupt || (byte = 0u,
            x86_core_machine_memory_write(machine, MYRAIDENII_CORE_SOUND_START + 12u, &byte, 1u) !=
                LIB_STATUS_OK) ||
        myraidenii_core_board_sound_command_get(board, 0u, &byte) != LIB_STATUS_OK ||
        byte != 0x12u || myraidenii_core_board_sound_command_get(board, 1u, &byte) !=
            LIB_STATUS_OK || byte != 0x34u ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_SOUND_START + 10u, &byte, 1u) !=
            LIB_STATUS_OK || byte != 1u ||
        myraidenii_core_board_sound_command_acknowledge(board) != LIB_STATUS_OK ||
        myraidenii_core_board_sound_response_set(board, 0u, 0x56u) != LIB_STATUS_OK ||
        myraidenii_core_board_sound_response_set(board, 1u, 0x78u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_SOUND_START + 4u, &byte, 1u) !=
            LIB_STATUS_OK || byte != 0x56u ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_SOUND_START + 6u, &byte, 1u) !=
            LIB_STATUS_OK || byte != 0x78u ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_SOUND_START + 1u, &byte, 1u) !=
            LIB_STATUS_OK || byte != 0xffu ||
        (byte = 0x9au, x86_core_machine_memory_write(machine,
            MYRAIDENII_CORE_SOUND_START + 1u, &byte, 1u) != LIB_STATUS_OK) ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_SOUND_START + 4u, &byte, 1u) !=
            LIB_STATUS_OK || byte != 0x56u ||
        myraidenii_core_board_request_main_interrupt(board) != LIB_STATUS_OK ||
        myraidenii_core_board_sound_interrupt_is_pending(board, &sound_interrupt) != LIB_STATUS_OK ||
        sound_interrupt ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        myraidenii_core_board_sound_interrupt_is_pending(board, &sound_interrupt) != LIB_STATUS_OK ||
        sound_interrupt ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_SOUND_START + 10u, &byte, 1u) !=
            LIB_STATUS_OK || byte != 0u ||
        x86_core_machine_memory_read(machine, MYRAIDENII_CORE_PLAYER_INPUTS_ADDRESS, &word,
            sizeof(word)) != LIB_STATUS_OK || word != 0xffffu)
        goto done;
    failed = 0;
done:
    myraidenii_core_board_destroy(board);
    return failed;
}

static lib_i32 board_video_phase(void)
{
    myraidenii_core_video whole = {0};
    myraidenii_core_video sliced = {0};
    lib_bool edge = LIB_FALSE;

    if (myraidenii_core_video_advance(&whole, 245759u) ||
        myraidenii_core_video_ticks_until_vblank(&whole) != 1u ||
        !myraidenii_core_video_advance(&whole, 1u) ||
        myraidenii_core_video_ticks_until_vblank(&whole) != 288768u)
        return 1;
    for (lib_u64 ticks = 0u; ticks < 245760u;) {
        lib_u64 part = 245760u - ticks;

        if (part > 997u) part = 997u;
        edge = myraidenii_core_video_advance(&sliced, part);
        ticks += part;
        if (edge != (ticks == 245760u)) return 1;
    }
    if (myraidenii_core_video_ticks_until_vblank(&sliced) !=
        myraidenii_core_video_ticks_until_vblank(&whole)) return 1;
    myraidenii_core_video_reset(&sliced);
    return myraidenii_core_video_ticks_until_vblank(&sliced) != 245760u;
}

static lib_i32 board_video_interrupts(void)
{
    const myraidenii_core_board_config config = {
        .program_even = program_even,
        .program_odd = program_odd,
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u,
            [X86_CORE_CPU_DEBUG_ESP] = 0x6000u}
    };
    const lib_u8 guest[] = {0xfbu, 0xf4u, 0xebu, 0xfdu};
    const lib_u8 handler[] = {0xffu, 0x06u, 0x00u, 0x10u, 0xcfu};
    const lib_u8 vector[] = {0x00u, 0x09u, 0x00u, 0x00u};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_run_result result;
    x86_core_machine_time_observation time;
    lib_u8 advanced;
    lib_u16 count = 0u;
    lib_i32 failed = 1;

    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK ||
        (machine = myraidenii_core_board_machine_get(board)) == LIB_NULL ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x900u, handler, sizeof(handler)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, MYRAIDENII_CORE_MAIN_INTERRUPT_VECTOR * 4u,
            vector, sizeof(vector)) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
        goto done;

    /* Independent raster oracle: 240 active lines, 512 pixels and two CPU
     * model ticks per pixel. Later frames contain 282 lines. */
    for (lib_u16 frame = 0u; frame < 3u; ++frame) {
        const lib_u64 edge = 245760u + (lib_u64)frame * 288768u;

        if (x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
            !time.next_deadline_valid || time.next_deadline_tick != edge ||
            !time.pacing_time_available || time.pacing_ticks_per_second != 16000000u ||
            time.physical_time_available)
            goto done;
        if (x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) !=
                LIB_STATUS_OK || result.executed != 0u || result.ticks != 0u ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
            !advanced ||
            x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
            time.elapsed_ticks != edge || time.next_deadline_tick != edge + 288768u ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) !=
                LIB_STATUS_OK || result.executed != 0u ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){2u, 0u}, &result) !=
                LIB_STATUS_OK || result.executed != 2u ||
            x86_core_machine_memory_read(machine, 0x1000u, &count, sizeof(count)) !=
                LIB_STATUS_OK || count != frame + 1u ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){2u, 0u}, &result) !=
                LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
            goto done;
    }
    if (x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
        !advanced || x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        time.elapsed_ticks != 0u || !time.next_deadline_valid ||
        time.next_deadline_tick != 245760u ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
        result.executed != 2u || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
        goto done;
    failed = 0;
done:
    myraidenii_core_board_destroy(board);
    return failed;
}

static lib_i32 board_cop_routes(void)
{
    const myraidenii_core_board_config config = {
        .program_even = program_even,
        .program_odd = program_odd,
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const lib_u32 configuration_addresses[] = {
        0x00428u, 0x0042cu, 0x00432u, 0x0043cu, 0x00444u, 0x00476u,
        0x0047eu, 0x004a0u, 0x004acu, 0x004c0u, 0x004ccu
    };
    const lib_u8 configuration_write[] = {0xa5u, 0x5au};
    const lib_u8 command_write[] = {0x96u, 0x69u};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_memory_route route;
    lib_u8 value[2] = {0u, 0u};
    lib_size index;
    lib_i32 failed = 1;

    lib_memory_set(program_even, 0, sizeof(program_even));
    lib_memory_set(program_odd, 0, sizeof(program_odd));
    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK ||
        (machine = myraidenii_core_board_machine_get(board)) == LIB_NULL)
        goto done;

    for (index = 0u; index < sizeof(configuration_addresses) /
        sizeof(configuration_addresses[0]); ++index) {
        if (x86_core_machine_memory_query(machine, configuration_addresses[index],
                sizeof(configuration_write), X86_CORE_MACHINE_MEMORY_ACCESS_WRITE,
                &route) != LIB_STATUS_OK || route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
            x86_core_machine_memory_write(machine, configuration_addresses[index],
                configuration_write, sizeof(configuration_write)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, configuration_addresses[index],
                configuration_write, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_query(machine, configuration_addresses[index],
                sizeof(value), X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
            route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
            x86_core_machine_memory_read(machine, configuration_addresses[index], value,
                sizeof(value)) != (configuration_addresses[index] >= 0x4a0u ?
                    LIB_STATUS_OK : LIB_STATUS_INVALID_STATE) ||
            (configuration_addresses[index] >= 0x4a0u &&
                (value[0] != 0xa5u || value[1] != 0x5au)))
            goto done;
    }

    if (x86_core_machine_memory_query(machine, 0x00426u, 2u,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM ||
        x86_core_machine_memory_query(machine, 0x00500u, sizeof(command_write),
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
        x86_core_machine_memory_write(machine, 0x00500u, command_write,
            sizeof(command_write)) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x00500u, command_write, 1u) !=
            LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_read(machine, 0x00500u, value, sizeof(value)) !=
            LIB_STATUS_INVALID_STATE || x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_query(machine, 0x00428u, sizeof(value),
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER)
        goto done;
    failed = 0;
done:
    myraidenii_core_board_destroy(board);
    return failed;
}

static lib_status board_cop_word_write(x86_core_machine *machine,
    lib_u32 address, lib_u16 value)
{
    const lib_u8 bytes[2] = {(lib_u8)value, (lib_u8)(value >> 8u)};

    return x86_core_machine_memory_write(machine, address, bytes, sizeof(bytes));
}

static lib_status board_cop_upload(x86_core_machine *machine, lib_u16 command,
    lib_u16 base, lib_u8 words)
{
    lib_status status = board_cop_word_write(machine, 0x438u, 0x1234u);

    if (status == LIB_STATUS_OK) status = board_cop_word_write(machine, 0x43au, 0xffffu);
    if (status == LIB_STATUS_OK) status = board_cop_word_write(machine, 0x43cu, command);
    for (lib_u8 index = 0u; index < words && status == LIB_STATUS_OK; ++index) {
        status = board_cop_word_write(machine, 0x434u, (lib_u16)(base + index));
        if (status == LIB_STATUS_OK)
            status = board_cop_word_write(machine, 0x432u, (lib_u16)(0x6000u + index));
    }
    return status;
}

static lib_i32 board_cop_transfers(void)
{
    static const struct {
        lib_u16 command;
        lib_u32 source;
        lib_u32 destination;
        lib_u16 upload_base;
        lib_u8 upload_words;
        lib_bool failure;
        lib_bool accepted_write_prefix;
    } cases[] = {
        {0x5205u, 0x2000u, 0x3000u, 0u, 8u, LIB_FALSE, LIB_FALSE},
        {0x5a05u, 0x2000u, 0x3000u, 0u, 8u, LIB_FALSE, LIB_FALSE},
        {0xf205u, 0x2000u, 0x3000u, 0u, 8u, LIB_FALSE, LIB_FALSE},
        {0x5205u, 0x2000u, 0x2002u, 248u, 8u, LIB_FALSE, LIB_FALSE},
        {0x5205u, 0x428u, 0x3000u, 0u, 8u, LIB_TRUE, LIB_FALSE},
        {0x5205u, 0x426u, 0x3000u, 0u, 8u, LIB_TRUE, LIB_FALSE},
        {0x5205u, 0x2000u, 0x428u, 0u, 8u, LIB_TRUE, LIB_FALSE},
        {0x5205u, 0x2000u, 0x426u, 0u, 8u, LIB_TRUE, LIB_TRUE},
        {0x5205u, 0x2000u, 0x3000u, 0u, 7u, LIB_TRUE, LIB_FALSE},
        {0x5205u, 0x2000u, 0x3000u, 0u, 0u, LIB_TRUE, LIB_FALSE},
        {0x6105u, 0x2000u, 0x3000u, 0u, 8u, LIB_TRUE, LIB_FALSE},
        {0x5205u, 0x102000u, 0x3000u, 0u, 8u, LIB_TRUE, LIB_FALSE},
        {0xf205u, LIB_UINT32_MAX - 3u, 0x3000u, 0u, 8u, LIB_TRUE, LIB_FALSE}
    };
    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    const lib_u8 source_data[] = {0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66u, 0x77u, 0x88u};
    const lib_u8 sentinel[] = {0xaau, 0xbbu, 0xccu, 0xddu};
    lib_i32 failures = 0;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        myraidenii_core_board *board = LIB_NULL;
        x86_core_machine *machine;
        x86_core_machine_run_result result = {0};
        lib_u8 guest[] = {0xb8u, (lib_u8)cases[index].command,
            (lib_u8)(cases[index].command >> 8u), 0xa3u,
            (lib_u8)((index % 3u) * 2u), 0x05u, 0xf4u};
        lib_u8 actual[4];
        lib_status status;
        lib_i32 failed = 1;

        if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK) return 1;
        machine = myraidenii_core_board_machine_get(board);
        if (x86_core_machine_memory_write(machine, 0x2000u, source_data,
                sizeof(source_data)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x3000u, sentinel,
                sizeof(sentinel)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
            x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            board_cop_upload(machine, cases[index].command, cases[index].upload_base,
                cases[index].upload_words) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4c0u, (lib_u16)cases[index].source) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4a0u, (lib_u16)(cases[index].source >> 16u)) !=
                LIB_STATUS_OK ||
            board_cop_word_write(machine, cases[index].command == 0xf205u ? 0x4c4u : 0x4c2u,
                (lib_u16)cases[index].destination) != LIB_STATUS_OK)
            goto done;
        status = x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){cases[index].failure ? 2u : 3u, 0u}, &result);
        if (cases[index].failure) {
            x86_core_machine_lifecycle lifecycle;

            if (status == LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_FAULT ||
                x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
                lifecycle != X86_CORE_MACHINE_FAULTED)
                goto done;
            if (x86_core_machine_memory_inspect(machine, 0x3000u, actual, sizeof(actual)) !=
                    LIB_STATUS_OK || lib_memory_compare(actual, sentinel, sizeof(actual)) != 0)
                goto done;
            if (cases[index].accepted_write_prefix &&
                (x86_core_machine_memory_inspect(machine, 0x426u, actual, 2u) !=
                    LIB_STATUS_OK || actual[0] != 0x11u || actual[1] != 0x22u))
                goto done;
        } else if (status != LIB_STATUS_OK || result.executed != 3u ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_memory_inspect(machine, cases[index].destination,
                actual, sizeof(actual)) != LIB_STATUS_OK ||
            lib_memory_compare(actual, source_data + (cases[index].command == 0xf205u ? 4u : 0u),
                sizeof(actual)) != 0) {
            goto done;
        }
        failed = 0;
done:
        if (failed) lib_c_printf("COP:TRANSFER:FAIL case=%u reason=%u detail=%u executed=%llu\n",
            (lib_u32)index, (lib_u32)result.reason, result.detail, result.executed);
        myraidenii_core_board_destroy(board);
        failures += failed;
    }
    return failures != 0;
}

static lib_i32 board_cop_reset_and_admission(void)
{
    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    lib_u8 data = 0xabu;
    lib_i32 failed = 1;

    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK) return 1;
    machine = myraidenii_core_board_machine_get(board);
    if (board_cop_word_write(machine, 0x434u, 256u) != LIB_STATUS_INVALID_ARGUMENT ||
        board_cop_upload(machine, 0x5205u, 0u, 7u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x434u, 7u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x432u, &data, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, 0x5205u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x433u, &data, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, 0x5205u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, 0x5205u) != LIB_STATUS_INVALID_STATE ||
        board_cop_word_write(machine, 0x4c0u, 0x2000u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, 0x5205u) != LIB_STATUS_INVALID_STATE ||
        board_cop_upload(machine, 0x5205u, 248u, 8u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x504u, 0x5205u) != LIB_STATUS_OK)
        goto done;
    failed = 0;
done:
    myraidenii_core_board_destroy(board);
    return failed;
}

static lib_status board_cop_dword_write(x86_core_machine *machine,
    lib_u32 address, lib_u32 value)
{
    lib_status status = board_cop_word_write(machine, address, (lib_u16)value);

    return status == LIB_STATUS_OK ?
        board_cop_word_write(machine, address + 2u, (lib_u16)(value >> 16u)) : status;
}

static lib_i32 board_cop_updates(void)
{
    static const struct {
        lib_u16 command;
        lib_u32 base;
        lib_u32 previous;
        lib_u32 increment;
        lib_u16 coordinate;
        lib_u32 expected;
        lib_u16 expected_coordinate;
        lib_bool failure;
        lib_bool inspect_position;
        lib_bool inspect_coordinate;
    } cases[] = {
        {0x0205u,0x2000u,0x0001ffffu,1u,7u,0x00020000u,8u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x0205u,0x2000u,0x00020000u,0xffffffffu,7u,0x0001ffffu,6u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x0205u,0x2000u,0xffff8000u,0x10000u,0xffffu,0x8000u,0u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x0205u,0x2000u,0x7fff0000u,0x10000u,0xffffu,0x80000000u,0u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x0205u,0x2000u,0u,0xffff0000u,0u,0xffff0000u,0xffffu,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x0905u,0x2000u,0xffffffffu,2u,7u,1u,7u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x0905u,0x2000u,0x80000000u,0x80000000u,7u,0u,7u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x0905u,0x2000u,5u,0xfffffffeu,7u,3u,7u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        /* Actual write-only COP routes refuse each source lane. */
        {0x0205u,0x424u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_FALSE,LIB_FALSE},
        {0x0205u,0x422u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_FALSE,LIB_FALSE},
        {0x0205u,0x418u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0205u,0x416u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0205u,0x40au,0xffffu,1u,7u,0x10000u,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        /* GPR readback is legal, but a pending operation cannot rewrite it. */
        {0x0205u,0x4a0u,0x1ffffu,1u,7u,0x1ffffu,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0205u,0x49au,0xffffu,1u,7u,0u,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0205u,0x486u,0x1ffffu,1u,7u,0x20000u,7u,LIB_TRUE,LIB_TRUE,LIB_TRUE},
        {0x0905u,0x418u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_FALSE,LIB_FALSE},
        {0x0905u,0x416u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_FALSE,LIB_FALSE},
        {0x0905u,0x400u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0905u,0x3feu,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0905u,0x494u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0905u,0x48eu,0xffffu,1u,7u,0u,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0205u,LIB_UINT32_MAX-2u,0u,0u,0u,0u,0u,LIB_TRUE,LIB_FALSE,LIB_FALSE},
        {0x0905u,0x102000u,0u,0u,0u,0u,0u,LIB_TRUE,LIB_FALSE,LIB_FALSE},
        {0x2a05u,0x2000u,5u,1u,7u,5u,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE}
    };
    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    lib_i32 failures = 0;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    for (lib_u8 slot = 0u; slot < (cases[index].failure ? 1u : 3u); ++slot) {
        const lib_u32 position = cases[index].command == 0x0905u ? 16u : 4u;
        const lib_u32 increment = cases[index].command == 0x0905u ? 40u : 16u;
        myraidenii_core_board *board = LIB_NULL;
        x86_core_machine *machine;
        x86_core_machine_run_result result = {0};
        const lib_u8 guest[] = {0xb8u, (lib_u8)cases[index].command,
            (lib_u8)(cases[index].command >> 8u), 0xa3u, (lib_u8)(slot * 2u), 5u, 0xf4u};
        lib_u8 data[4];
        lib_status status;
        lib_i32 failed = 1;

        if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK) return 1;
        machine = myraidenii_core_board_machine_get(board);
        if (cases[index].base < 0x100000u &&
            (board_cop_dword_write(machine, cases[index].base + position + slot * 4u,
                cases[index].previous) != LIB_STATUS_OK ||
             board_cop_dword_write(machine, cases[index].base + increment + slot * 4u,
                cases[index].increment) != LIB_STATUS_OK ||
             board_cop_word_write(machine, cases[index].base + 30u + slot * 4u,
                cases[index].coordinate) != LIB_STATUS_OK))
            goto done;
        if (!cases[index].failure && board_cop_word_write(machine,
                cases[index].base + 32u + slot * 4u, 0x5aa5u) != LIB_STATUS_OK)
            goto done;
        if (board_cop_upload(machine, cases[index].command, 0u, 8u) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4a0u, (lib_u16)(cases[index].base >> 16u)) !=
                LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4c0u, (lib_u16)cases[index].base) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
            x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK)
            goto done;
        status = x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){cases[index].failure ? 2u : 3u, 0u}, &result);
        if (cases[index].failure) {
            x86_core_machine_lifecycle lifecycle;

            if (status != LIB_STATUS_INTERNAL_ERROR || result.reason != X86_CORE_MACHINE_STOP_FAULT ||
                x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
                lifecycle != X86_CORE_MACHINE_FAULTED)
                goto done;
        } else if (status != LIB_STATUS_OK || result.executed != 3u ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            goto done;
        }
        if (cases[index].inspect_position &&
            (x86_core_machine_memory_inspect(machine, cases[index].base + position + slot * 4u,
                data, sizeof(data)) != LIB_STATUS_OK ||
             ((lib_u32)data[0] | (lib_u32)data[1] << 8u | (lib_u32)data[2] << 16u |
                (lib_u32)data[3] << 24u) != cases[index].expected))
            goto done;
        /* A refused auxiliary write must retain its old readback value. */
        if (cases[index].inspect_coordinate &&
            (x86_core_machine_memory_inspect(machine, cases[index].base + 30u + slot * 4u,
                data, cases[index].failure ? 2u : 4u) != LIB_STATUS_OK ||
             (lib_u16)(data[0] | (lib_u16)data[1] << 8u) != cases[index].expected_coordinate ||
             (!cases[index].failure && (data[2] != 0xa5u || data[3] != 0x5au))))
            goto done;
        failed = 0;
done:
        if (failed) lib_c_printf("COP:UPDATE:FAIL case=%u slot=%u reason=%u detail=%u\n",
            (lib_u32)index, (lib_u32)slot, (lib_u32)result.reason, result.detail);
        myraidenii_core_board_destroy(board);
        failures += failed;
    }
    return failures != 0;
}

static lib_status board_cop_run_commands(x86_core_machine *machine,
    const lib_u16 *commands, lib_u8 count, lib_u8 port,
    x86_core_machine_run_result *result)
{
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    lib_u8 guest[24];
    lib_status status;

    if (count == 0u || count > 4u || port > 2u) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u8 index = 0u; index < count; ++index) {
        const lib_u8 instruction[] = {0xb8u, (lib_u8)commands[index],
            (lib_u8)(commands[index] >> 8u), 0xa3u, (lib_u8)(port * 2u), 5u};

        lib_memory_copy(guest + index * 6u, instruction, sizeof(instruction));
    }
    status = x86_core_machine_memory_write(machine, 0x800u, guest, count * 6u);
    if (status == LIB_STATUS_OK) status = x86_core_machine_debug_patch_registers(machine, &entry);
    return status == LIB_STATUS_OK ? x86_core_machine_debug_continue(machine,
        (x86_core_machine_run_budget){count * 2u, 0u}, result) : status;
}

static lib_i32 board_cop_collision_result(x86_core_machine *machine,
    const lib_u16 expected[5])
{
    lib_u8 bytes[2];

    for (lib_u8 index = 0u; index < 5u; ++index) {
        if (x86_core_machine_memory_inspect(machine, 0x580u + index * 2u,
                bytes, sizeof(bytes)) != LIB_STATUS_OK ||
            (lib_u16)(bytes[0] | (lib_u16)bytes[1] << 8u) != expected[index])
            return 1;
    }
    if (x86_core_machine_memory_inspect(machine, 0x589u, bytes, 1u) != LIB_STATUS_OK ||
        bytes[0] != (lib_u8)(expected[0] >> 8u))
        return 1;
    return x86_core_machine_memory_inspect(machine, 0x587u, bytes, sizeof(bytes)) !=
        LIB_STATUS_OK || bytes[0] != (lib_u8)(expected[3] >> 8u) ||
        bytes[1] != (lib_u8)expected[0];
}

static lib_i32 board_cop_collisions(void)
{
    static const struct {
        lib_i16 first[3], second[3];
        lib_u16 pair_first, pair_second;
        lib_u16 expected[5];
    } cases[] = {
        {{10,20,30},{15,25,35},0x0a00u,0x0a00u,{0u,0xfffbu,0xfffbu,0xfffbu,0u}},
        {{10,20,30},{30,25,35},0x0a00u,0x0a00u,{1u,0xffecu,0xfffbu,0xfffbu,1u}},
        {{10,20,30},{20,25,35},0x0a00u,0x0a00u,{1u,0xfff6u,0xfffbu,0xfffbu,1u}},
        {{10,20,30},{15,40,35},0x0a00u,0x0a00u,{2u,0xfffbu,0xffecu,0xfffbu,2u}},
        {{10,20,30},{15,25,60},0x0a00u,0x0a00u,{4u,0xfffbu,0xfffbu,0xffe2u,4u}},
        {{10,20,30},{30,40,60},0x0a00u,0x0a00u,{7u,0xffecu,0xffecu,0xffe2u,7u}},
        {{10,10,10},{5,5,5},0x0af6u,0x0a00u,{0u,5u,5u,5u,0u}},
        {{10,10,10},{5,5,5},0u,0x0a00u,{0u,5u,5u,5u,0u}},
        {{10,10,10},{10,10,10},0u,0u,{7u,0u,0u,0u,7u}},
        {{5,5,5},{5,5,5},0u,0x0a00u,{7u,0u,0u,0u,7u}},
        {{32767,32767,32767},{-32768,-32768,-32768},0x0a01u,0x0a00u,
            {0u,0xffffu,0xffffu,0xffffu,0u}},
        {{32760,32760,32760},{32765,32765,32765},0x1400u,0x0100u,
            {7u,0xfffbu,0xfffbu,0xfffbu,7u}},
        {{0,0,0},{100,100,100},0xff80u,0x0100u,{0u,0xff9cu,0xff9cu,0xff9cu,0u}},
        {{-30,-30,-30},{-25,-25,-25},0x0a00u,0x0a00u,{0u,0xfffbu,0xfffbu,0xfffbu,0u}}
    };
    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const lib_u16 commands[] = {0xa100u,0xa900u,0xb100u,0xb900u};
    const lib_u16 reverse[] = {0xa900u,0xa100u,0xb900u,0xb100u};
    lib_i32 failures = 0;

    /* Normal vectors plus each position flags/axis and pointer/box read failure. */
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]) + 17u; ++index) {
        const lib_bool failure = index >= sizeof(cases) / sizeof(cases[0]);
        const lib_size vector = failure ? 0u : index;
        const lib_u32 high_base = index == 1u ? 0x10000u : 0u;
        myraidenii_core_board *board = LIB_NULL;
        x86_core_machine *machine;
        x86_core_machine_run_result result = {0};
        lib_status status;
        lib_i32 failed = 1;

        if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK) return 1;
        machine = myraidenii_core_board_machine_get(board);
        for (lib_u8 slot = 0u; slot < 4u; ++slot) {
            if (board_cop_upload(machine, commands[slot], slot * 8u, 8u) != LIB_STATUS_OK ||
                board_cop_word_write(machine, 0x4c0u + slot * 2u,
                    (lib_u16)(slot < 2u ? 0x2000u + slot * 0x100u :
                        0x3000u + (slot - 2u) * 2u)) != LIB_STATUS_OK)
                goto done;
        }
        if (board_cop_word_write(machine, 0x436u, (lib_u16)(high_base >> 16u)) != LIB_STATUS_OK)
            goto done;
        for (lib_u8 slot = 0u; slot < 2u; ++slot) {
            if (board_cop_word_write(machine, 0x2002u + slot * 0x100u, 0xffffu) != LIB_STATUS_OK ||
                board_cop_word_write(machine, 0x3000u + slot * 2u,
                    (lib_u16)(0x4000u + slot * 0x10u)) != LIB_STATUS_OK)
                goto done;
            for (lib_u8 axis = 0u; axis < 3u; ++axis) {
                if (board_cop_word_write(machine, 0x2006u + slot * 0x100u + axis * 4u,
                        (lib_u16)(slot == 0u ? cases[vector].first[axis] :
                            cases[vector].second[axis])) != LIB_STATUS_OK ||
                    board_cop_word_write(machine, high_base + 0x4000u + slot * 0x10u + axis * 2u,
                        slot == 0u ? cases[vector].pair_first : cases[vector].pair_second) !=
                            LIB_STATUS_OK)
                    goto done;
            }
        }
        if (board_cop_run_commands(machine, index % 2u == 0u ? commands : reverse,
                4u, (lib_u8)(index % 3u), &result) != LIB_STATUS_OK || result.executed != 8u ||
            board_cop_collision_result(machine, cases[vector].expected))
            goto done;
        if (failure) {
            const lib_size fault = index - sizeof(cases) / sizeof(cases[0]);
            const lib_u8 slot = (lib_u8)(fault < 8u ? fault / 4u : (fault - 8u) / 4u);
            const lib_u16 command = commands[fault < 8u ? slot : 2u + (slot % 2u)];

            if (fault < 8u) {
                if (board_cop_word_write(machine, 0x4c0u + slot * 2u,
                        (lib_u16)(0x426u - (fault % 4u) * 4u)) != LIB_STATUS_OK)
                    goto done;
            } else if (fault == 16u) {
                if (board_cop_word_write(machine, 0x436u, 0x10u) != LIB_STATUS_OK) goto done;
            } else if ((fault - 8u) % 4u == 0u) {
                if (board_cop_word_write(machine, 0x4c4u + slot * 2u, 0x428u) != LIB_STATUS_OK)
                    goto done;
            } else if (board_cop_word_write(machine, 0x3000u + slot * 2u,
                    (lib_u16)(0x428u - (((fault - 8u) % 4u) - 1u) * 2u)) != LIB_STATUS_OK) {
                goto done;
            }
            status = board_cop_run_commands(machine, &command, 1u, 0u, &result);
            if (status != LIB_STATUS_INTERNAL_ERROR || result.reason != X86_CORE_MACHINE_STOP_FAULT ||
                board_cop_collision_result(machine, cases[0].expected))
                goto done;
        } else if (index == 0u) {
            const lib_u16 zero[5] = {0};
            const lib_u16 changed[5] = {1u,15u,0xfffbu,0xfffbu,1u};

            if (board_cop_word_write(machine, 0x580u, 0xffffu) != LIB_STATUS_INVALID_STATE ||
                board_cop_word_write(machine, 0x588u, 0xffffu) != LIB_STATUS_INVALID_STATE ||
                board_cop_word_write(machine, 0x2006u, 30u) != LIB_STATUS_OK ||
                board_cop_run_commands(machine, commands + 2u, 1u, 0u, &result) != LIB_STATUS_OK ||
                board_cop_collision_result(machine, cases[0].expected) ||
                board_cop_run_commands(machine, commands, 1u, 0u, &result) != LIB_STATUS_OK ||
                board_cop_run_commands(machine, commands + 3u, 1u, 0u, &result) != LIB_STATUS_OK)
                goto done;
            /* Position reload changes differences, not the previously captured bounds. */
            if (board_cop_collision_result(machine, (lib_u16[5]){0u,15u,0xfffbu,0xfffbu,0u}) ||
                board_cop_run_commands(machine, commands + 2u, 1u, 0u, &result) != LIB_STATUS_OK ||
                board_cop_collision_result(machine, changed) ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK ||
                board_cop_collision_result(machine, zero) ||
                board_cop_word_write(machine, 0x500u, commands[0]) != LIB_STATUS_INVALID_STATE)
                goto done;
            for (lib_u8 alternate = 0u; alternate < 3u; ++alternate) {
                const lib_u16 unsupported[] = {0xa180u,0xa980u,0xb080u};

                if (board_cop_upload(machine, unsupported[alternate], 0u, 8u) != LIB_STATUS_OK ||
                    board_cop_word_write(machine, 0x500u, unsupported[alternate]) !=
                        LIB_STATUS_INVALID_STATE)
                    goto done;
            }
            /* Reset must clear latches, not merely the visible result words. */
            for (lib_u8 slot = 0u; slot < 3u; ++slot) {
                if (board_cop_upload(machine, commands[slot], slot * 8u, 8u) != LIB_STATUS_OK ||
                    board_cop_word_write(machine, 0x4c0u + slot * 2u,
                        slot == 2u ? 0x3000u : (lib_u16)(0x2000u + slot * 0x100u)) != LIB_STATUS_OK)
                    goto done;
                if (board_cop_word_write(machine, 0x2006u + slot * 4u, 20u) != LIB_STATUS_OK)
                    goto done;
            }
            if (board_cop_run_commands(machine, commands, 1u, 0u, &result) != LIB_STATUS_OK ||
                board_cop_run_commands(machine, commands + 2u, 1u, 0u, &result) != LIB_STATUS_OK ||
                board_cop_collision_result(machine, (lib_u16[5]){7u,20u,20u,20u,7u}))
                goto done;
        }
        failed = 0;
done:
        if (failed) lib_c_printf("COP:COLLISION:FAIL case=%u reason=%u detail=%u\n",
            (lib_u32)index, (lib_u32)result.reason, result.detail);
        myraidenii_core_board_destroy(board);
        failures += failed;
    }
    return failures != 0;
}

lib_i32 main(void)
{
    return board_invalid_arguments() || board_program_and_ram_routes() ||
        board_interrupt_input_and_sound_routes() || board_video_phase() ||
        board_video_interrupts() || board_cop_routes() || board_cop_reset_and_admission() ||
        board_cop_transfers() || board_cop_updates() || board_cop_collisions();
}
