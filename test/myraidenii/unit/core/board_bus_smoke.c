#include "myraidenii/core/board_interface.h"
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
                sizeof(value)) != LIB_STATUS_INVALID_STATE)
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

lib_i32 main(void)
{
    return board_invalid_arguments() || board_program_and_ram_routes() ||
        board_interrupt_input_and_sound_routes() || board_cop_routes();
}
