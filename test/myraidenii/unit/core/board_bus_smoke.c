#include "myraidenii/core/board_interface.h"

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

lib_i32 main(void)
{
    return board_invalid_arguments() || board_program_and_ram_routes();
}
