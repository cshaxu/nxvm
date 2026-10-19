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

lib_i32 main(void)
{
    return board_invalid_arguments() || board_program_and_ram_routes() ||
        board_interrupt_input_and_sound_routes() || board_video_phase() ||
        board_video_interrupts() || board_cop_routes() || board_cop_reset_and_admission() ||
        board_cop_transfers();
}
