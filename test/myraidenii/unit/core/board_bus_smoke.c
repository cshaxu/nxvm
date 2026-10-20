#include "myraidenii/core/board_interface.h"
#include "myraidenii/core/cop.h"
#include "myraidenii/core/video.h"
#include "lib/types/file.h"
#include "x86-core/machine/debug_interface.h"

static lib_u8 program_even[MYRAIDENII_CORE_PROGRAM_LANE_BYTES];
static lib_u8 program_odd[MYRAIDENII_CORE_PROGRAM_LANE_BYTES];

/* COP failure matrices need arbitrary RAM beside its real register routes,
 * not fictitious RAM inside the selected Board's control window. */
typedef struct cop_fixture {
    x86_core_machine *machine;
    myraidenii_core_cop *cop;
} cop_fixture;

static void cop_fixture_advance(void *owner, lib_u64 ticks, lib_u64 due_tick)
{
    cop_fixture *fixture = owner;
    lib_status status = myraidenii_core_cop_advance(fixture->cop);

    (void)ticks;
    (void)due_tick;
    if (status != LIB_STATUS_OK)
        (void)x86_core_machine_report_fault(fixture->machine, (lib_u32)status);
}

static void cop_fixture_reset(void *owner)
{
    cop_fixture *fixture = owner;

    myraidenii_core_cop_reset(fixture->cop);
}

static void cop_fixture_finalize(void *owner)
{
    cop_fixture *fixture = owner;

    myraidenii_core_cop_destroy(fixture->cop);
    fixture->cop = LIB_NULL;
}

static void cop_fixture_destroy(cop_fixture *fixture)
{
    x86_core_machine_destroy(fixture->machine);
    myraidenii_core_cop_destroy(fixture->cop);
}

static lib_status cop_fixture_create(cop_fixture *fixture)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 0x100000u,
        .cpu_profile = X86_CORE_CPU_PROFILE_V30,
        .time_axis = {X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 16000000u}
    };
    const x86_core_machine_attachment attachment = {
        .context = fixture,
        .before_internal_advance = cop_fixture_advance,
        .reset_devices = cop_fixture_reset,
        .finalize_devices = cop_fixture_finalize
    };
    lib_status status = x86_core_machine_neutral_create(&config, &fixture->machine);

    if (status == LIB_STATUS_OK) status = myraidenii_core_cop_create(&fixture->cop);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_bind_attachment(fixture->machine, &attachment);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_install(fixture->cop, fixture->machine);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_freeze_execution_providers(fixture->machine);
    if (status == LIB_STATUS_OK) status = x86_core_machine_reset(fixture->machine);
    if (status != LIB_STATUS_OK) cop_fixture_destroy(fixture);
    return status;
}

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

static lib_i32 board_control_window(void)
{
    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const lib_u32 unmodelled[] = {
        0x400u,0x426u,0x470u,0x5ffu,0x600u,0x68eu,0x694u,0x698u,0x699u,0x69eu,0x6cau,0x6ccu,0x762u,0x7ffu
    };
    const lib_u8 data[8] = {0x11u,0x22u,0x33u,0x44u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    const lib_u8 guest[] = {0xb8u,1u,0u,0xa3u,0u,4u,0xf4u};
    const lib_u8 unused_write_guest[] = {0xb8u,1u,0u,0xa3u,0x3eu,4u,0xf4u};
    const lib_u8 unused_write_word_guest[] = {0xb8u,1u,0u,0xa3u,0xc8u,6u,0xf4u};
    const lib_u8 unused_write_696_guest[] = {0xb8u,1u,0u,0xa3u,0x96u,6u,0xf4u};
    const lib_u8 unused_write_690_guest[] = {0xb8u,1u,0u,0xa3u,0x90u,6u,0xf4u};
    const lib_u8 unused_write_692_guest[] = {0xb8u,1u,0u,0xa3u,0x92u,6u,0xf4u};
    const lib_u8 unused_write_69a_guest[] = {0xb8u,1u,0u,0xa3u,0x9au,6u,0xf4u};
    const lib_u8 unused_write_69c_guest[] = {0xb8u,1u,0u,0xa3u,0x9cu,6u,0xf4u};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_run_result result = {0};
    x86_core_machine_memory_route route;
    lib_u8 value[8];
    lib_i32 failed = 1;

    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK) return 1;
    machine = myraidenii_core_board_machine_get(board);
    for (lib_u8 phase = 0u; phase < 2u; ++phase) {
        if (phase != 0u && x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
        for (lib_u32 address = 0x400u; address < 0x800u; ++address) {
            if (x86_core_machine_memory_query(machine, address, 1u,
                    X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
                route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
                x86_core_machine_memory_query(machine, address, 1u,
                    X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
                route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER)
                goto done;
        }
        for (lib_size index = 0u; index < sizeof(unmodelled) / sizeof(unmodelled[0]); ++index)
        for (lib_size width = 1u; width <= sizeof(value); width *= 2u) {
            if (x86_core_machine_memory_read(machine, unmodelled[index], value, width) !=
                    LIB_STATUS_INVALID_STATE ||
                x86_core_machine_memory_inspect(machine, unmodelled[index], value, width) !=
                    LIB_STATUS_INVALID_STATE ||
                x86_core_machine_memory_write(machine, unmodelled[index], data, width) !=
                    LIB_STATUS_INVALID_STATE)
                goto done;
        }
        /* A broad fallback must not swallow two distinct valid register words. */
        if (x86_core_machine_memory_write(machine, 0x4c0u, data, 4u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x4c0u, value, 4u) != LIB_STATUS_OK ||
            lib_memory_compare(value, data, 4u) != 0 ||
            x86_core_machine_memory_inspect(machine, 0x4c0u, value, 4u) != LIB_STATUS_OK ||
            lib_memory_compare(value, data, 4u) != 0 ||
            x86_core_machine_memory_write(machine, 0x3ffu, data, 2u) != LIB_STATUS_INVALID_STATE ||
            x86_core_machine_memory_inspect(machine, 0x3ffu, value, 1u) != LIB_STATUS_OK ||
            value[0] != data[0] ||
            x86_core_machine_memory_write(machine, 0x800u, data + 2u, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x7ffu, data, 2u) != LIB_STATUS_INVALID_STATE ||
            x86_core_machine_memory_inspect(machine, 0x800u, value, 1u) != LIB_STATUS_OK ||
            value[0] != data[2])
            goto done;
    }
    if (x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) ==
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_FAULT)
        goto done;
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, unused_write_guest,
            sizeof(unused_write_guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        x86_core_machine_memory_write(machine, 0x43eu, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x43eu, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x43eu, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x43eu, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x6c8u, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x6c8u, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x6c9u, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x6c9u, data, 2u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_read(machine, 0x6c8u, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x6c8u, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x696u, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x696u, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x697u, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x697u, data, 2u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_read(machine, 0x696u, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x696u, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x690u, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x690u, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x691u, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x691u, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x690u, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x690u, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x692u, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x692u, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x693u, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x693u, data, 2u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_read(machine, 0x692u, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x692u, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x69au, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x69au, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x69bu, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x69bu, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x69au, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x69au, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x69cu, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x69cu, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x69du, data, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x69du, data, 2u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_read(machine, 0x69cu, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x69cu, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x6ceu, data, 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x6cfu, data, 2u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_read(machine, 0x6ceu, value, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x6ceu, value, 1u) != LIB_STATUS_INVALID_STATE)
        goto done;
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, unused_write_word_guest,
            sizeof(unused_write_word_guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
        goto done;
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, unused_write_690_guest,
            sizeof(unused_write_690_guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
        goto done;
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, unused_write_696_guest,
            sizeof(unused_write_696_guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
        goto done;
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, unused_write_692_guest,
            sizeof(unused_write_692_guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
        goto done;
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, unused_write_69a_guest,
            sizeof(unused_write_69a_guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
        goto done;
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, unused_write_69c_guest,
            sizeof(unused_write_69c_guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
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

static lib_i32 board_video_sprite_flags(void)
{
    myraidenii_core_video video = {0};

    if (myraidenii_core_video_sprite_flags_write(LIB_NULL, 0u, 0u) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_video_sprite_flags_write(&video, 2u, 0u) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_video_sprite_flags_write(&video, 0u, 0x34u) != LIB_STATUS_OK ||
        video.sprite_flags != 0x0034u ||
        myraidenii_core_video_sprite_flags_write(&video, 1u, 0x80u) != LIB_STATUS_OK ||
        video.sprite_flags != 0x8034u) {
        return 1;
    }
    myraidenii_core_video_reset(&video);
    return video.sprite_flags == 0u ? 0 : 1;
}

static lib_i32 board_cop_random_result(x86_core_machine *machine, lib_u16 expected)
{
    x86_core_machine_timeline_observation before, after;
    lib_u8 value[2];

    if (x86_core_machine_get_timeline_observation(machine, &before) != LIB_STATUS_OK)
        return 1;
    for (lib_u32 address = 0x5a0u; address < 0x5a8u; ++address) {
        const lib_u8 byte = (lib_u8)(expected >> ((address & 1u) * 8u));

        if (x86_core_machine_memory_read(machine, address, value, 1u) != LIB_STATUS_OK ||
            value[0] != byte ||
            x86_core_machine_memory_inspect(machine, address, value, 1u) != LIB_STATUS_OK ||
            value[0] != byte)
            return 1;
        if (address < 0x5a7u &&
            (x86_core_machine_memory_read(machine, address, value, 2u) != LIB_STATUS_OK ||
             value[0] != byte || value[1] !=
                (lib_u8)(expected >> (((address + 1u) & 1u) * 8u))))
            return 1;
    }
    return x86_core_machine_get_timeline_observation(machine, &after) != LIB_STATUS_OK ||
        before.now != after.now;
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
    const lib_u16 limits[] = {65535u, 256u, 250u, 0u, 1u};
    const lib_u16 random_results[3][5] = {
        {49152u,68u,31u,0u,0u},
        {10240u,225u,149u,0u,0u},
        {36864u,125u,16u,0u,0u}
    };
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
            time.elapsed_ticks != edge || time.next_deadline_tick != edge + 288768u)
            goto done;
        /* Literal RNG oracles at the existing three independently scheduled edges. */
        for (lib_size index = 0u; index < sizeof(limits) / sizeof(limits[0]); ++index) {
            const lib_u8 bytes[2] = {(lib_u8)limits[index], (lib_u8)(limits[index] >> 8u)};

            if (x86_core_machine_memory_write(machine, 0x42cu, bytes, sizeof(bytes)) !=
                    LIB_STATUS_OK || board_cop_random_result(machine, random_results[frame][index]))
                goto done;
        }
        if (x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) !=
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
        0x0041cu, 0x0041eu, 0x00428u, 0x0042cu, 0x00432u, 0x0043cu, 0x00444u, 0x00476u,
        0x0047eu, 0x004a0u, 0x004acu, 0x004c0u, 0x004ccu
    };
    const struct {
        lib_u32 first;
        lib_u8 bytes;
    } unmodelled[] = {
        {0x45au,4u}
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    const lib_u8 guest[] = {0xb8u,1u,0u,0xa3u,0xfcu,6u,0xf4u};
    const lib_u8 mode_bytes[] = {0x11u,0x22u};
    const lib_u8 configuration_write[] = {0xa5u, 0x5au};
    const lib_u8 command_write[] = {0x96u, 0x69u};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_memory_route route;
    x86_core_machine_run_result result = {0};
    x86_core_machine_lifecycle lifecycle;
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
        const lib_bool readable = configuration_addresses[index] == 0x47eu ||
            configuration_addresses[index] >= 0x4a0u;

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
                sizeof(value)) != (readable ?
                    LIB_STATUS_OK : LIB_STATUS_INVALID_STATE) ||
            (readable &&
                (value[0] != 0xa5u || value[1] != 0x5au)))
            goto done;
    }

    if (x86_core_machine_memory_write(machine, 0x47eu, mode_bytes, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x47eu, value, 2u) != LIB_STATUS_OK ||
        value[0] != 0x11u || value[1] != 0x5au ||
        x86_core_machine_memory_write(machine, 0x47fu, mode_bytes + 1u, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x47eu, value, 2u) != LIB_STATUS_OK ||
        value[0] != 0x11u || value[1] != 0x22u ||
        x86_core_machine_memory_inspect(machine, 0x47fu, value, 1u) != LIB_STATUS_OK ||
        value[0] != 0x22u)
        goto done;
    for (lib_u8 phase = 0u; phase < 2u; ++phase) {
        if (phase != 0u && (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x47eu, value, 2u) != LIB_STATUS_OK ||
            value[0] != 0u || value[1] != 0u))
            goto done;
        for (index = 0u; index < sizeof(unmodelled) / sizeof(unmodelled[0]); ++index) {
            lib_u8 bulk[14] = {0x55u,0xaau,0x33u};

            for (lib_u8 lane = 0u; lane < unmodelled[index].bytes; ++lane) {
                for (lib_u8 width = 1u; width <= 2u; ++width) {
                    const lib_u32 address = unmodelled[index].first + lane;

                    if (lane + width > unmodelled[index].bytes) continue;
                    if (x86_core_machine_memory_query(machine, address, width,
                            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
                        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
                        x86_core_machine_memory_query(machine, address, width,
                            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
                        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
                        x86_core_machine_memory_read(machine, address, value, width) !=
                            LIB_STATUS_INVALID_STATE ||
                        x86_core_machine_memory_inspect(machine, address, value, width) !=
                            LIB_STATUS_INVALID_STATE ||
                        x86_core_machine_memory_write(machine, address, configuration_write, width) !=
                            LIB_STATUS_INVALID_STATE)
                        goto done;
                }
            }
            if (unmodelled[index].bytes > sizeof(bulk) ||
                x86_core_machine_memory_query(machine, unmodelled[index].first,
                    unmodelled[index].bytes, X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) !=
                    LIB_STATUS_OK || route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
                x86_core_machine_memory_read(machine, unmodelled[index].first, bulk,
                    unmodelled[index].bytes) != LIB_STATUS_INVALID_STATE ||
                x86_core_machine_memory_inspect(machine, unmodelled[index].first, bulk,
                    unmodelled[index].bytes) != LIB_STATUS_INVALID_STATE ||
                x86_core_machine_memory_write(machine, unmodelled[index].first, bulk,
                    unmodelled[index].bytes) != LIB_STATUS_INVALID_STATE ||
                x86_core_machine_memory_write(machine, unmodelled[index].first - 1u, bulk, 3u) !=
                    LIB_STATUS_INVALID_STATE ||
                x86_core_machine_memory_inspect(machine, unmodelled[index].first - 1u,
                    value, 1u) != LIB_STATUS_INVALID_STATE)
                goto done;
        }
    }

    if (x86_core_machine_memory_query(machine, 0x00426u, 1u,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
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
    if (x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) ==
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_FAULT ||
        x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
        lifecycle != X86_CORE_MACHINE_FAULTED)
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
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    const lib_u8 source_data[] = {0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66u, 0x77u, 0x88u};
    const lib_u8 sentinel[] = {0xaau, 0xbbu, 0xccu, 0xddu};
    lib_i32 failures = 0;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        cop_fixture fixture = {0};
        x86_core_machine *machine;
        x86_core_machine_run_result result = {0};
        lib_u8 guest[] = {0xb8u, (lib_u8)cases[index].command,
            (lib_u8)(cases[index].command >> 8u), 0xa3u,
            (lib_u8)((index % 3u) * 2u), 0x05u, 0xf4u};
        lib_u8 actual[4];
        lib_status status;
        lib_i32 failed = 1;

        if (cop_fixture_create(&fixture) != LIB_STATUS_OK) return 1;
        machine = fixture.machine;
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
        cop_fixture_destroy(&fixture);
        failures += failed;
    }
    return failures != 0;
}

static lib_i32 board_cop_reset_and_admission(void)
{
    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    const lib_u8 guest[] = {0xb8u,0x05u,0x52u,0xa3u,4u,5u,0xf4u};
    const lib_u8 source[] = {0x12u,0x34u,0x56u,0x78u};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_run_result result = {0};
    lib_u8 data = 0xabu;
    lib_u8 mode[2];
    lib_u8 actual[4];
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
        board_cop_word_write(machine, 0x47eu, 0xffffu) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_read(machine, 0x47eu, mode, 2u) != LIB_STATUS_OK ||
        mode[0] != 0u || mode[1] != 0u ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, 0x5205u) != LIB_STATUS_INVALID_STATE ||
        board_cop_upload(machine, 0x5205u, 248u, 8u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x4c0u, 0x2000u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x4c2u, 0x3000u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x2000u, source, sizeof(source)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){3u,0u}, &result) !=
            LIB_STATUS_OK || result.executed != 3u ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        x86_core_machine_memory_inspect(machine, 0x3000u, actual, sizeof(actual)) != LIB_STATUS_OK ||
        lib_memory_compare(actual, source, sizeof(actual)) != 0)
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
        {0x0205u,0x466u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0205u,0x464u,0xffffu,1u,7u,0xffffu,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x0205u,0x45eu,0xffffu,1u,7u,0x10000u,7u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
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
        {0x0905u,0x102000u,0u,0u,0u,0u,0u,LIB_TRUE,LIB_FALSE,LIB_FALSE}
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
        cop_fixture fixture = {0};
        x86_core_machine *machine;
        x86_core_machine_run_result result = {0};
        const lib_u8 guest[] = {0xb8u, (lib_u8)cases[index].command,
            (lib_u8)(cases[index].command >> 8u), 0xa3u, (lib_u8)(slot * 2u), 5u, 0xf4u};
        lib_u8 data[4];
        lib_status status;
        lib_i32 failed = 1;

        if (cop_fixture_create(&fixture) != LIB_STATUS_OK) return 1;
        machine = fixture.machine;
        if (cases[index].base < 0x100000u &&
            (board_cop_dword_write(machine, cases[index].base + position + slot * 4u,
                cases[index].previous) != LIB_STATUS_OK ||
             board_cop_dword_write(machine, cases[index].base + increment + slot * 4u,
                cases[index].increment) != LIB_STATUS_OK ||
             ((cases[index].command == 0x0205u || cases[index].inspect_coordinate) &&
                board_cop_word_write(machine, cases[index].base + 30u + slot * 4u,
                    cases[index].coordinate) != LIB_STATUS_OK)))
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
        cop_fixture_destroy(&fixture);
        failures += failed;
    }
    return failures != 0;
}

static lib_status board_cop_address_write(x86_core_machine *machine, lib_u8 index,
    lib_u32 value)
{
    lib_status status = board_cop_word_write(machine, 0x4a0u + index * 2u,
        (lib_u16)(value >> 16u));

    return status == LIB_STATUS_OK ? board_cop_word_write(machine, 0x4c0u + index * 2u,
        (lib_u16)value) : status;
}

static lib_status board_cop_run_commands(x86_core_machine *machine,
    const lib_u16 *commands, lib_u8 count, lib_u8 port,
    x86_core_machine_run_result *result);

static lib_i32 board_cop_offset_add(void)
{
    const lib_u16 command = 0x2a05u;
    const lib_u32 primary = 0x2000u, secondary = 0x2400u;
    lib_i32 failed = 1;
    cop_fixture fixture = {0};
    x86_core_machine *machine;
    x86_core_machine_run_result result = {0};
    lib_u8 actual[4];

    if (cop_fixture_create(&fixture) != LIB_STATUS_OK) return 1;
    machine = fixture.machine;
    for (lib_u8 slot = 0u; slot < 3u; ++slot) {
        const lib_u16 delta = (lib_u16)(slot + 2u);
        const lib_u16 first = (lib_u16)(0xffffu - slot);
        const lib_u16 second = (lib_u16)(0xfffeu - slot);
        const lib_u32 first_expected = (lib_u32)first + delta;
        const lib_u32 second_expected = (lib_u32)second + delta;

        if (board_cop_word_write(machine, secondary + 0x1eu + slot * 4u, delta) !=
                LIB_STATUS_OK ||
            board_cop_word_write(machine, primary + 6u + slot * 4u, first) !=
                LIB_STATUS_OK ||
            board_cop_word_write(machine, primary + 0x1eu + slot * 4u, second) !=
                LIB_STATUS_OK)
            goto done;
        if (slot == 0u &&
            (board_cop_upload(machine, command, 0u, 8u) != LIB_STATUS_OK ||
             board_cop_address_write(machine, 0u, primary) != LIB_STATUS_OK ||
             board_cop_address_write(machine, 1u, secondary) != LIB_STATUS_OK))
            goto done;
        if (board_cop_run_commands(machine, &command, 1u, slot, &result) != LIB_STATUS_OK ||
            result.executed != 2u ||
            x86_core_machine_memory_inspect(machine, primary + 6u + slot * 4u,
                actual, sizeof(actual)) != LIB_STATUS_OK ||
            ((lib_u32)actual[0] | (lib_u32)actual[1] << 8u | (lib_u32)actual[2] << 16u |
                (lib_u32)actual[3] << 24u) != first_expected ||
            x86_core_machine_memory_inspect(machine, primary + 0x1eu + slot * 4u,
                actual, sizeof(actual)) != LIB_STATUS_OK ||
            ((lib_u32)actual[0] | (lib_u32)actual[1] << 8u | (lib_u32)actual[2] << 16u |
                (lib_u32)actual[3] << 24u) != second_expected)
            goto done;
    }

    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_upload(machine, command, 0u, 8u) != LIB_STATUS_OK ||
        board_cop_address_write(machine, 0u, primary) != LIB_STATUS_OK ||
        board_cop_address_write(machine, 1u, primary - 24u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, primary + 6u, 2u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, primary + 0x1eu, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, command) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, command) != LIB_STATUS_INVALID_STATE ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, primary + 6u, actual, sizeof(actual)) !=
            LIB_STATUS_OK ||
        ((lib_u32)actual[0] | (lib_u32)actual[1] << 8u | (lib_u32)actual[2] << 16u |
            (lib_u32)actual[3] << 24u) != 4u ||
        x86_core_machine_memory_read(machine, primary + 0x1eu, actual, sizeof(actual)) !=
            LIB_STATUS_OK ||
        ((lib_u32)actual[0] | (lib_u32)actual[1] << 8u | (lib_u32)actual[2] << 16u |
            (lib_u32)actual[3] << 24u) != 3u)
        goto done;

    if (x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    if (board_cop_word_write(machine, 0x500u, command) != LIB_STATUS_INVALID_STATE) goto done;
    if (board_cop_upload(machine, command, 0u, 8u) != LIB_STATUS_OK) goto done;
    if (board_cop_address_write(machine, 0u, primary) != LIB_STATUS_OK) goto done;
    if (board_cop_address_write(machine, 1u, LIB_UINT32_MAX - 0x1du) != LIB_STATUS_OK) goto done;
    if (board_cop_word_write(machine, primary + 6u, 0xa5a5u) != LIB_STATUS_OK) goto done;
    if (board_cop_word_write(machine, primary + 0x1eu, 0x5a5au) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, primary + 6u, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != 0xa5u || actual[1] != 0xa5u) goto done;
    if (x86_core_machine_memory_read(machine, 0x4a2u, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != 0xffu || actual[1] != 0xffu ||
        x86_core_machine_memory_read(machine, 0x4c2u, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != 0xe2u || actual[1] != 0xffu ||
        board_cop_word_write(machine, 0x500u, command) != LIB_STATUS_OK) goto done;
    if (myraidenii_core_cop_advance(fixture.cop) == LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, primary + 6u, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != 0xa5u || actual[1] != 0xa5u) goto done;
    if (x86_core_machine_memory_read(machine, primary + 0x1eu, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != 0x5au || actual[1] != 0x5au) goto done;

    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_upload(machine, command, 0u, 8u) != LIB_STATUS_OK ||
        board_cop_address_write(machine, 0u, 0xfffe0u) != LIB_STATUS_OK ||
        board_cop_address_write(machine, 1u, secondary) != LIB_STATUS_OK ||
        board_cop_word_write(machine, secondary + 0x1eu, 2u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0xfffe6u, 0xffffu) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0xffffeu, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, command) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) == LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0xfffe6u, actual, sizeof(actual)) !=
            LIB_STATUS_OK ||
        ((lib_u32)actual[0] | (lib_u32)actual[1] << 8u | (lib_u32)actual[2] << 16u |
            (lib_u32)actual[3] << 24u) != 0x10001u ||
        /* The first dword lane commits before the second crosses installed RAM. */
        x86_core_machine_memory_read(machine, 0xffffeu, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != 3u || actual[1] != 0u)
        goto done;

    failed = 0;
done:
    cop_fixture_destroy(&fixture);
    return failed;
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

static lib_i32 board_cop_angle_approach(void)
{
    static const struct {
        lib_u32 base;
        lib_u16 flags;
        lib_u8 angle, target, step, next;
        lib_u16 expected_flags;
        lib_bool failure, inspect_flags, inspect_angle;
    } cases[] = {
        {0x2000u,0xa5a5u,10u,20u,3u,13u,0xa5a1u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a5u,20u,10u,3u,17u,0xa5a1u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a1u,10u,13u,3u,13u,0xa5a5u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a1u,13u,10u,3u,10u,0xa5a5u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a5u,250u,5u,4u,254u,0xa5a1u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a5u,5u,250u,4u,1u,0xa5a1u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a5u,0u,128u,1u,1u,0xa5a1u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a5u,128u,0u,1u,129u,0xa5a1u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a1u,77u,77u,0u,77u,0xa5a5u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a5u,77u,78u,0u,77u,0xa5a1u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a1u,77u,78u,255u,78u,0xa5a5u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a1u,255u,0u,1u,0u,0xa5a5u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        {0x2000u,0xa5a1u,0u,255u,1u,255u,0xa5a5u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        /* The last installed RAM byte cannot tolerate a word read or write. */
        {0xfffcbu,0xa5a5u,10u,20u,3u,13u,0xa5a1u,LIB_FALSE,LIB_TRUE,LIB_TRUE},
        /* Angle read, flags read/write, then late angle-write failure. */
        {0x3f8u,0xa5a5u,10u,20u,3u,10u,0xa5a5u,LIB_TRUE,LIB_TRUE,LIB_FALSE},
        {0x434u,0x00a1u,10u,20u,3u,10u,0x00a1u,LIB_TRUE,LIB_FALSE,LIB_TRUE},
        {0x4a2u,0xa5a5u,10u,20u,3u,10u,0xa5a5u,LIB_TRUE,LIB_TRUE,LIB_TRUE},
        {0x4bfu,0xbfa5u,10u,20u,3u,10u,0xbfa1u,LIB_TRUE,LIB_TRUE,LIB_TRUE},
        {0x499u,0xa5a5u,10u,20u,3u,10u,0xa5a1u,LIB_TRUE,LIB_TRUE,LIB_TRUE},
        {LIB_UINT32_MAX-0x20u,0u,0u,0u,0u,0u,0u,LIB_TRUE,LIB_FALSE,LIB_FALSE},
        {0x102000u,0u,0u,0u,0u,0u,0u,LIB_TRUE,LIB_FALSE,LIB_FALSE}
    };
    const lib_u16 command = 0x6200u;
    lib_i32 failures = 0;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    for (lib_u8 port = 0u; port < (cases[index].failure ? 1u : 3u); ++port) {
        cop_fixture fixture = {0};
        x86_core_machine *machine;
        x86_core_machine_run_result result = {0};
        lib_u8 angle[] = {cases[index].angle,0x5au};
        const lib_size angle_bytes = cases[index].base == 0xfffcbu ? 1u : 2u;
        lib_u8 actual[2];
        lib_status status;
        lib_i32 failed = 1;

        if (cop_fixture_create(&fixture) != LIB_STATUS_OK) return 1;
        machine = fixture.machine;
        if (board_cop_upload(machine, command, 0u, 8u) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4a0u, (lib_u16)(cases[index].base >> 16u)) !=
                LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4c0u, (lib_u16)cases[index].base) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x41cu, 0xab00u | cases[index].target) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x41eu, 0xcd00u | cases[index].step) != LIB_STATUS_OK)
            goto done;
        if (cases[index].base < 0x100000u &&
            (board_cop_word_write(machine, cases[index].base, cases[index].flags) !=
                LIB_STATUS_OK ||
             x86_core_machine_memory_write(machine, cases[index].base + 0x34u,
                angle, angle_bytes) != LIB_STATUS_OK))
            goto done;
        status = board_cop_run_commands(machine, &command, 1u, port, &result);
        if (cases[index].failure) {
            x86_core_machine_lifecycle lifecycle;

            if (status == LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_FAULT ||
                x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
                lifecycle != X86_CORE_MACHINE_FAULTED)
                goto done;
        } else if (status != LIB_STATUS_OK || result.executed != 2u) {
            goto done;
        }
        if (cases[index].inspect_flags &&
            (x86_core_machine_memory_inspect(machine, cases[index].base, actual, 2u) !=
                LIB_STATUS_OK ||
             (lib_u16)(actual[0] | (lib_u16)actual[1] << 8u) != cases[index].expected_flags))
            goto done;
        if (cases[index].inspect_angle &&
            (x86_core_machine_memory_inspect(machine, cases[index].base + 0x34u,
                actual, angle_bytes) != LIB_STATUS_OK || actual[0] != cases[index].next ||
             (angle_bytes == 2u && actual[1] != 0x5au)))
            goto done;
        failed = 0;
done:
        if (failed) lib_c_printf("COP:ANGLE-APPROACH:FAIL case=%u port=%u reason=%u detail=%u\n",
            (lib_u32)index, (lib_u32)port, (lib_u32)result.reason, result.detail);
        cop_fixture_destroy(&fixture);
        failures += failed;
    }
    if (failures != 0) return 1;

    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const lib_u8 next[] = {254u,2u,5u};
    const lib_u8 angle[] = {250u,0x5au};
    const lib_u8 target = 5u, step = 4u;
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_run_result result = {0};
    lib_u8 actual[2];
    lib_i32 failed = 1;

    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK) return 1;
    machine = myraidenii_core_board_machine_get(board);
    if (board_cop_upload(machine, command, 0u, 8u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x4c0u, 0x2000u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x2000u, 0xa5a5u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x2034u, angle, sizeof(angle)) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x41cu, 0xabffu) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x41cu, &target, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x41eu, 0xcd00u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x41eu, &step, 1u) != LIB_STATUS_OK)
        goto board_done;
    for (lib_u8 port = 0u; port < 3u; ++port) {
        if (board_cop_run_commands(machine, &command, 1u, port, &result) != LIB_STATUS_OK ||
            x86_core_machine_memory_inspect(machine, 0x2034u, actual, 2u) != LIB_STATUS_OK ||
            actual[0] != next[port] || actual[1] != 0x5au ||
            x86_core_machine_memory_inspect(machine, 0x2000u, actual, 2u) != LIB_STATUS_OK ||
            actual[0] != (port == 2u ? 0xa5u : 0xa1u) || actual[1] != 0xa5u)
            goto board_done;
    }
    if (board_cop_word_write(machine, 0x500u, command) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x41cu, 0u) != LIB_STATUS_INVALID_STATE ||
        board_cop_word_write(machine, 0x41eu, 0u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_read(machine, 0x5b0u, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != 0u || actual[1] != 0u ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, command) != LIB_STATUS_INVALID_STATE ||
        board_cop_upload(machine, command, 0u, 8u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x4c0u, 0x2000u) != LIB_STATUS_OK ||
        board_cop_run_commands(machine, &command, 1u, 0u, &result) != LIB_STATUS_OK ||
        x86_core_machine_memory_inspect(machine, 0x2000u, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != 4u || actual[1] != 0u ||
        x86_core_machine_memory_inspect(machine, 0x2034u, actual, 1u) != LIB_STATUS_OK ||
        actual[0] != 0u)
        goto board_done;
    failed = 0;
board_done:
    myraidenii_core_board_destroy(board);
    return failed;
}

static lib_i32 board_cop_numeric_result(x86_core_machine *machine,
    const lib_u16 expected[3])
{
    lib_u8 bytes[6], observed[6], byte;
    x86_core_machine_lifecycle lifecycle;

    if (x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
        x86_core_machine_memory_inspect(machine, 0x5b0u, bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x5b0u, observed, sizeof(observed)) !=
            (lifecycle == X86_CORE_MACHINE_FAULTED ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK) ||
        (lifecycle != X86_CORE_MACHINE_FAULTED &&
         lib_memory_compare(bytes, observed, sizeof(bytes)) != 0))
        return 1;
    for (lib_u8 index = 0u; index < 6u; ++index) {
        if (bytes[index] != (lib_u8)(expected[index / 2u] >> ((index & 1u) * 8u)) ||
            x86_core_machine_memory_inspect(machine, 0x5b0u + index, &byte, 1u) != LIB_STATUS_OK ||
            byte != bytes[index]) {
            lib_c_printf("COP:NUMERIC-RESULT:FAIL byte=%u actual=%u expected=%u\n",
                (lib_u32)index, (lib_u32)bytes[index],
                (lib_u32)(lib_u8)(expected[index / 2u] >> ((index & 1u) * 8u)));
            return 1;
        }
    }
    return 0;
}

static lib_status board_cop_numeric_seed(x86_core_machine *machine)
{
    const lib_u16 commands[] = {0x130eu,0x2208u,0x338eu,0x3bb0u,0x42c2u,
        0x4aa0u,0x6200u,0x8100u,0x8900u};
    x86_core_machine_run_result result = {0};

    for (lib_u8 index = 0u; index < 9u; ++index)
        if (board_cop_upload(machine, commands[index], index * 8u, 8u) != LIB_STATUS_OK)
            return LIB_STATUS_INTERNAL_ERROR;
    if (board_cop_word_write(machine, 0x4c0u, 0x2000u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x4c2u, 0x2100u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x2106u, 3u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x210au, 4u) != LIB_STATUS_OK ||
        board_cop_run_commands(machine, commands + 3u, 1u, 0u, &result) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x2106u, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x210au, 1u) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    return board_cop_run_commands(machine, commands, 1u, 0u, &result);
}

static lib_i32 board_cop_numeric_vectors(void)
{
    static const struct {
        lib_u16 command;
        lib_u32 x, y;
        lib_u16 scale, expected[3];
        lib_u32 output;
        lib_u8 offset, bytes;
    } cases[] = {
        {0x130eu,0x10000u,0x10000u,0u,{7u,5u,32u},0x5aa5u,0x34u,2u},
        {0x130eu,0xffff0000u,0x10000u,0u,{7u,5u,224u},0x5aa5u,0x34u,2u},
        {0x130eu,0x10000u,0xffff0000u,0u,{7u,5u,96u},0x5aa5u,0x34u,2u},
        {0x130eu,0xffff0000u,0xffff0000u,0u,{7u,5u,160u},0x5aa5u,0x34u,2u},
        {0x130eu,0u,0xffff0000u,0u,{7u,5u,128u},0x5aa5u,0x34u,2u},
        {0x130eu,1u,0u,0u,{0x8007u,5u,0u},0x5aa5u,0x34u,2u},
        {0x338eu,0x80000000u,0x80000000u,0u,{7u,5u,160u},0x5aa0u,0x34u,2u},
        {0x338eu,0x10000u,0x10000u,0u,{7u,5u,32u},0x5a20u,0x34u,2u},
        {0x338eu,1u,0u,0u,{0x8007u,5u,0u},0x5a00u,0x34u,2u},
        {0x2208u,0xffffu,0xffffu,0u,{7u,5u,32u},0x5aa5u,0x34u,2u},
        {0x2208u,0x8000u,1u,0u,{7u,5u,63u},0x5aa5u,0x34u,2u},
        {0x2208u,1u,0xffffu,0u,{7u,5u,0u},0x5aa5u,0x34u,2u},
        {0x2208u,1u,0u,0u,{0x8007u,5u,0u},0x5aa5u,0x34u,2u},
        {0x3bb0u,0x30000u,0x40000u,0u,{7u,5u,32u},5u,0x3au,2u},
        {0x3bb0u,0xffffffffu,0u,0u,{7u,1u,32u},1u,0x3au,2u},
        {0x3bb0u,0x80000000u,0x80000000u,0u,{7u,46340u,32u},46340u,0x3au,2u},
        {0x3bb0u,0x7fffffffu,0x7fffffffu,0u,{7u,46339u,32u},46339u,0x3au,2u},
        {0x3bb0u,0u,0u,0u,{7u,0u,32u},0u,0x3au,2u},
        {0x42c2u,3u,0u,0u,{7u,5u,32u},53u,0x38u,2u},
        {0x42c2u,3u,0u,1u,{7u,5u,32u},26u,0x38u,2u},
        {0x42c2u,3u,0u,2u,{7u,5u,32u},13u,0x38u,2u},
        {0x42c2u,3u,0u,3u,{7u,5u,32u},6u,0x38u,2u},
        {0x42c2u,0u,0u,0u,{0x8007u,5u,32u},0u,0x38u,2u},
        {0x42c2u,0xffffu,0u,0u,{7u,5u,32u},0u,0x38u,2u},
        {0x4aa0u,3u,0u,0u,{7u,5u,32u},53u,0x36u,2u},
        {0x4aa0u,0u,0u,3u,{7u,5u,32u},20u,0x36u,2u},
        {0x8100u,0xab40u,0xcd01u,0u,{7u,5u,32u},2048u,0x10u,4u},
        {0x8100u,0u,0xffu,3u,{7u,5u,32u},0u,0x10u,4u},
        {0x8100u,0xc0u,0xffu,3u,{7u,5u,32u},(lib_u32)-8355840,0x10u,4u},
        {0x8100u,0x20u,0xffu,1u,{7u,5u,32u},738558u,0x10u,4u},
        {0x8100u,0xe0u,0xffu,0u,{7u,5u,32u},(lib_u32)-369279,0x10u,4u},
        {0x8900u,0u,1u,2u,{7u,5u,32u},8192u,0x14u,4u},
        {0x8900u,0x80u,0xffu,3u,{7u,5u,32u},(lib_u32)-8355840,0x14u,4u},
        {0x8900u,0x20u,0xffu,0u,{7u,5u,32u},369279u,0x14u,4u},
        {0x8900u,0x40u,0xffu,0u,{7u,5u,32u},0u,0x14u,4u},
        {0x8900u,0x7fu,1u,0u,{7u,5u,32u},(lib_u32)-2047,0x14u,4u},
        {0x8900u,0x81u,1u,0u,{7u,5u,32u},(lib_u32)-2047,0x14u,4u},
        {0x8100u,0xbfu,1u,0u,{7u,5u,32u},(lib_u32)-2047,0x10u,4u},
        {0x8100u,0xc1u,1u,0u,{7u,5u,32u},(lib_u32)-2047,0x10u,4u}
    };
    lib_i32 failures = 0;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    for (lib_u8 port = 0u; port < 3u; ++port) {
        cop_fixture fixture = {0};
        x86_core_machine_run_result result = {0};
        lib_u8 actual[4], sentinel[] = {0xa5u,0x5au,0x69u,0x96u};
        lib_i32 failed = 1;

        if (cop_fixture_create(&fixture) != LIB_STATUS_OK) return 1;
        x86_core_machine *machine = fixture.machine;
        if (board_cop_numeric_seed(machine) != LIB_STATUS_OK ||
            board_cop_numeric_result(machine, (lib_u16[3]){7u,5u,32u}) ||
            board_cop_word_write(machine, 0x444u, 0xabc0u | cases[index].scale) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x2000u + cases[index].offset,
                sentinel, 4u) != LIB_STATUS_OK)
            goto done;
        if (cases[index].command == 0x130eu || cases[index].command == 0x338eu ||
            cases[index].command == 0x3bb0u) {
            for (lib_u8 axis = 0u; axis < 2u; ++axis) {
                const lib_u32 second = 0xffff0000u + (axis == 0u ? cases[index].x : cases[index].y);

                if (board_cop_word_write(machine, 0x2004u + axis * 4u, 0u) != LIB_STATUS_OK ||
                    board_cop_word_write(machine, 0x2006u + axis * 4u, 0xffffu) != LIB_STATUS_OK ||
                    board_cop_word_write(machine, 0x2104u + axis * 4u, (lib_u16)second) != LIB_STATUS_OK ||
                    board_cop_word_write(machine, 0x2106u + axis * 4u, (lib_u16)(second >> 16u)) != LIB_STATUS_OK)
                    goto done;
            }
        } else {
            const lib_u32 operand = cases[index].command == 0x2208u ? 0x2012u :
                cases[index].command == 0x42c2u ? 0x2036u :
                cases[index].command == 0x4aa0u ? 0x2038u : 0x2034u;

            if (board_cop_word_write(machine, operand, (lib_u16)cases[index].x) != LIB_STATUS_OK ||
                (cases[index].command == 0x2208u &&
                 board_cop_word_write(machine, 0x2016u, (lib_u16)cases[index].y) != LIB_STATUS_OK) ||
                ((cases[index].command == 0x8100u || cases[index].command == 0x8900u) &&
                 board_cop_word_write(machine, 0x2036u, (lib_u16)cases[index].y) != LIB_STATUS_OK))
                goto done;
        }
        if (board_cop_run_commands(machine, &cases[index].command, 1u, port, &result) != LIB_STATUS_OK ||
            result.executed != 2u || board_cop_numeric_result(machine, cases[index].expected) ||
            x86_core_machine_memory_inspect(machine, 0x2000u + cases[index].offset,
                actual, cases[index].bytes) != LIB_STATUS_OK)
            goto done;
        for (lib_u8 byte = 0u; byte < cases[index].bytes; ++byte)
            if (actual[byte] != (lib_u8)(cases[index].output >> (byte * 8u))) goto done;
        if (cases[index].bytes == 2u &&
            (x86_core_machine_memory_inspect(machine, 0x2002u + cases[index].offset,
                actual, 2u) != LIB_STATUS_OK ||
             (lib_u16)(actual[0] | (lib_u16)actual[1] << 8u) !=
                (cases[index].command == 0x4aa0u ? (lib_u16)cases[index].x : 0x9669u)))
            goto done;
        failed = 0;
done:
        if (failed) lib_c_printf("COP:NUMERIC:FAIL case=%u port=%u reason=%u detail=%u\n",
            (lib_u32)index, (lib_u32)port, (lib_u32)result.reason, result.detail);
        cop_fixture_destroy(&fixture);
        failures += failed;
    }
    return failures != 0;
}

static lib_i32 board_cop_numeric_failures(void)
{
    const struct {
        lib_u16 command;
        lib_u32 base, other;
    } reads[] = {
        {0x130eu,0x2000u,0x424u}, {0x130eu,0x2000u,0x422u},
        {0x130eu,0x2000u,0x46eu}, {0x130eu,0x2000u,0x46cu},
        {0x130eu,0x424u,0x2100u}, {0x130eu,0x422u,0x2100u},
        {0x130eu,0x46eu,0x2100u}, {0x130eu,0x46cu,0x2100u},
        {0x3bb0u,0x2000u,0x424u}, {0x3bb0u,0x2000u,0x422u},
        {0x3bb0u,0x2000u,0x46eu}, {0x3bb0u,0x2000u,0x46cu},
        {0x3bb0u,0x424u,0x2100u}, {0x3bb0u,0x422u,0x2100u},
        {0x3bb0u,0x46eu,0x2100u}, {0x3bb0u,0x46cu,0x2100u},
        {0x2208u,0x416u,0x2100u}, {0x2208u,0x460u,0x2100u},
        {0x8100u,0x3f4u,0x2100u}, {0x8100u,0x3f2u,0x2100u},
        {0x8900u,0x3f4u,0x2100u}, {0x8900u,0x3f2u,0x2100u},
        {0x42c2u,0x3f2u,0x2100u}, {0x4aa0u,0x3f0u,0x2100u},
        {0x130eu,LIB_UINT32_MAX-2u,0x2100u}, {0x3bb0u,0x2000u,0x102000u}
    };
    const struct {
        lib_u16 command;
        lib_u32 base;
        lib_u16 expected[3];
        lib_u32 inspect, output;
        lib_u8 bytes;
    } writes[] = {
        {0x338eu,0x3f4u,{7u,5u,224u},0u,0u,0u},
        {0x3bb0u,0x3eeu,{7u,1u,32u},0u,0u,0u},
        {0x3bb0u,0x575u,{7u,1u,32u},0x5afu,0x0701u,2u},
        {0x8100u,0x4aeu,{7u,5u,32u},0x4beu,0x04ae0800u,4u},
        {0x8900u,0x4aau,{7u,5u,32u},0x4beu,0x04aa0800u,4u},
        {0x8100u,0x5a0u,{7u,5u,32u},0u,0u,0u},
        {0x42c2u,0x3f0u,{0x8007u,5u,32u},0u,0u,0u},
        {0x4aa0u,0x57au,{7u,5u,32u},0u,0u,0u}
    };
    const lib_size read_count = sizeof(reads) / sizeof(reads[0]);
    lib_i32 failures = 0;

    for (lib_size index = 0u; index < read_count + sizeof(writes) / sizeof(writes[0]); ++index) {
        const lib_bool input_failure = index < read_count;
        const lib_size row = input_failure ? index : index - read_count;
        const lib_u16 command = input_failure ? reads[row].command : writes[row].command;
        const lib_u32 base = input_failure ? reads[row].base : writes[row].base;
        const lib_u32 other = input_failure ? reads[row].other : 0x2100u;
        const lib_u16 prior[3] = {7u,5u,32u};
        const lib_u16 zero_divide = 0x42c2u;
        cop_fixture fixture = {0};
        x86_core_machine_run_result result = {0};
        lib_u8 actual[4];
        lib_i32 failed = 1;

        if (cop_fixture_create(&fixture) != LIB_STATUS_OK) return 1;
        x86_core_machine *machine = fixture.machine;
        if (board_cop_numeric_seed(machine) != LIB_STATUS_OK ||
            board_cop_run_commands(machine, &zero_divide, 1u, 0u, &result) != LIB_STATUS_OK ||
            board_cop_numeric_result(machine, (lib_u16[3]){0x8007u,5u,32u}) ||
            board_cop_word_write(machine, 0x4a0u, (lib_u16)(base >> 16u)) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4c0u, (lib_u16)base) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4a2u, (lib_u16)(other >> 16u)) != LIB_STATUS_OK ||
            board_cop_word_write(machine, 0x4c2u, (lib_u16)other) != LIB_STATUS_OK)
            goto done;
        if (!input_failure) {
            if (board_cop_word_write(machine, 0x2106u, 0xffffu) != LIB_STATUS_OK)
                goto done;
            if ((command == 0x8100u || command == 0x8900u) &&
                (board_cop_word_write(machine, base + 0x34u, command == 0x8100u ? 64u : 0u) != LIB_STATUS_OK ||
                 board_cop_word_write(machine, base + 0x36u, 1u) != LIB_STATUS_OK))
                goto done;
        }
        if (board_cop_run_commands(machine, &command, 1u, (lib_u8)(index % 3u), &result) ==
                LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_FAULT ||
            board_cop_numeric_result(machine, input_failure ? prior : writes[row].expected))
            goto done;
        if (!input_failure && writes[row].bytes != 0u) {
            if (x86_core_machine_memory_inspect(machine, writes[row].inspect, actual,
                    writes[row].bytes) != LIB_STATUS_OK)
                goto done;
            for (lib_u8 byte = 0u; byte < writes[row].bytes; ++byte)
                if (actual[byte] != (lib_u8)(writes[row].output >> (byte * 8u))) goto done;
        }
        failed = 0;
done:
        if (failed) lib_c_printf("COP:NUMERIC-FAILURE:FAIL case=%u reason=%u detail=%u\n",
            (lib_u32)index, (lib_u32)result.reason, result.detail);
        cop_fixture_destroy(&fixture);
        failures += failed;
    }
    return failures != 0;
}

static lib_i32 board_cop_numeric_chain(void)
{
    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const lib_u16 chain[] = {0x42c2u,0x2208u,0x42c2u,0x6200u,0x42c2u,0x5205u};
    const lib_u8 guest_results[] = {0xa1u,0xb0u,5u,0xa3u,0u,0x30u,
        0xa1u,0xb2u,5u,0xa3u,2u,0x30u,0xa1u,0xb4u,5u,0xa3u,4u,0x30u};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine_run_result result = {0};
    lib_u8 actual[2];
    lib_i32 failed = 1;

    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK) return 1;
    x86_core_machine *machine = myraidenii_core_board_machine_get(board);
    if (board_cop_numeric_seed(machine) != LIB_STATUS_OK ||
        board_cop_upload(machine, 0x5205u, 72u, 8u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x2012u, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x2016u, 1u) != LIB_STATUS_OK)
        goto done;
    for (lib_u8 index = 0u; index < 6u; ++index) {
        if (board_cop_run_commands(machine, chain + index, 1u, index % 3u, &result) != LIB_STATUS_OK ||
            board_cop_numeric_result(machine, (lib_u16[3]){index % 2u == 0u ? 0x8007u : 7u,5u,32u}))
            goto done;
        if (index == 0u &&
            (board_cop_word_write(machine, 0x500u, 0x6105u) != LIB_STATUS_INVALID_STATE ||
             board_cop_numeric_result(machine, (lib_u16[3]){0x8007u,5u,32u})))
            goto done;
    }
    if (board_cop_word_write(machine, 0x2106u, 0x8000u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x210au, 0x8000u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x2036u, 1u) != LIB_STATUS_OK ||
        board_cop_run_commands(machine, (lib_u16[2]){0x3bb0u,0x42c2u}, 2u, 0u, &result) != LIB_STATUS_OK ||
        board_cop_numeric_result(machine, (lib_u16[3]){7u,46340u,32u}) ||
        x86_core_machine_memory_inspect(machine, 0x2038u, actual, 2u) != LIB_STATUS_OK ||
        (lib_u16)(actual[0] | (lib_u16)actual[1] << 8u) != 41088u)
        goto done;
    if (x86_core_machine_memory_write(machine, 0x800u, guest_results, sizeof(guest_results)) !=
            LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &(x86_core_cpu_debug_register_patch){
            .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
            .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}}) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){6u,0u}, &result) !=
            LIB_STATUS_OK || result.executed != 6u)
        goto done;
    for (lib_u8 index = 0u; index < 3u; ++index) {
        const lib_u16 expected[] = {7u,46340u,32u};

        if (x86_core_machine_memory_inspect(machine, 0x3000u + index * 2u, actual, 2u) !=
                LIB_STATUS_OK || (lib_u16)(actual[0] | (lib_u16)actual[1] << 8u) != expected[index])
            goto done;
    }
    for (lib_u8 index = 0u; index < 6u; ++index)
        if (x86_core_machine_memory_write(machine, 0x5b0u + index, actual, 1u) !=
                LIB_STATUS_INVALID_STATE ||
            (index % 2u == 0u && board_cop_word_write(machine, 0x5b0u + index, 0xffffu) !=
                LIB_STATUS_INVALID_STATE))
            goto done;
    if (board_cop_word_write(machine, 0x2036u, 0u) != LIB_STATUS_OK ||
        board_cop_run_commands(machine, chain, 1u, 0u, &result) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, 0x130eu) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x444u, 3u) != LIB_STATUS_INVALID_STATE ||
        board_cop_numeric_result(machine, (lib_u16[3]){0x8007u,46340u,32u}) ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_numeric_result(machine, (lib_u16[3]){0u,0u,0u}) ||
        board_cop_word_write(machine, 0x500u, 0x130eu) != LIB_STATUS_INVALID_STATE ||
        board_cop_upload(machine, 0x2208u, 0u, 8u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x12u, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x16u, 1u) != LIB_STATUS_OK ||
        board_cop_run_commands(machine, chain + 1u, 1u, 0u, &result) != LIB_STATUS_OK ||
        board_cop_numeric_result(machine, (lib_u16[3]){0u,0u,32u}))
        goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("COP:NUMERIC-CHAIN:FAIL reason=%u detail=%u\n",
        (lib_u32)result.reason, result.detail);
    myraidenii_core_board_destroy(board);
    return failed;
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
    const lib_u16 commands[] = {0xa100u,0xa900u,0xb100u,0xb900u};
    const lib_u16 reverse[] = {0xa900u,0xa100u,0xb900u,0xb100u};
    lib_i32 failures = 0;

    /* Normal vectors plus each position flags/axis and pointer/box read failure. */
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]) + 17u; ++index) {
        const lib_bool failure = index >= sizeof(cases) / sizeof(cases[0]);
        const lib_size vector = failure ? 0u : index;
        const lib_u32 high_base = index == 1u ? 0x10000u : 0u;
        cop_fixture fixture = {0};
        x86_core_machine *machine;
        x86_core_machine_run_result result = {0};
        lib_status status;
        lib_i32 failed = 1;

        if (cop_fixture_create(&fixture) != LIB_STATUS_OK) return 1;
        machine = fixture.machine;
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
                const lib_u16 position_base[] = {0x426u,0x42cu,0x46cu,0x468u};

                if (board_cop_word_write(machine, 0x4c0u + slot * 2u,
                        position_base[fault % 4u]) != LIB_STATUS_OK)
                    goto done;
            } else if (fault == 16u) {
                if (board_cop_word_write(machine, 0x436u, 0x10u) != LIB_STATUS_OK) goto done;
            } else if ((fault - 8u) % 4u == 0u) {
                if (board_cop_word_write(machine, 0x4c4u + slot * 2u, 0x428u) != LIB_STATUS_OK)
                    goto done;
            } else {
                const lib_u16 box_pointer[] = {0x428u,0x426u,0x42eu};

                if (board_cop_word_write(machine, 0x3000u + slot * 2u,
                        box_pointer[((fault - 8u) % 4u) - 1u]) != LIB_STATUS_OK)
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
        cop_fixture_destroy(&fixture);
        failures += failed;
    }
    return failures != 0;
}

static lib_i32 board_cop_decimal_result(x86_core_machine *machine, const char expected[10])
{
    lib_u8 actual[2];

    for (lib_u8 index = 0u; index < 10u; ++index) {
        if (x86_core_machine_memory_read(machine, 0x590u + index, actual, 1u) != LIB_STATUS_OK ||
            actual[0] != (lib_u8)expected[index])
            return 1;
        if (index < 9u && (x86_core_machine_memory_inspect(machine, 0x590u + index,
                actual, 2u) != LIB_STATUS_OK || actual[0] != (lib_u8)expected[index] ||
                actual[1] != (lib_u8)expected[index + 1u]))
            return 1;
    }
    return 0;
}

static lib_i32 board_cop_decimal(void)
{
    static const struct {
        lib_u32 number;
        lib_u16 mode;
        char expected[10];
    } cases[] = {
        {0u,0u,"0        "}, {1u,0u,"1        "}, {65535u,1u,"53556    "},
        {65536u,2u,"63556    "}, {123456789u,0u,"987654321"},
        {999999999u,0u,"999999999"}, {1000000000u,0u,"000000000"},
        {LIB_UINT32_MAX,0u,"592769492"}, {0x12345678u,0u,"698914503"},
        {0u,3u,"000000000"}, {42u,3u,"240000000"},
        {1u,0x103u,"1        "}, {1u,0xffffu,"1        "}
    };
    const myraidenii_core_board_config config = {
        program_even, program_odd, MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    const lib_u8 guest[] = {
        0xb8u,3u,0u, 0xa3u,0x24u,4u, 0xb8u,0x78u,0x56u, 0xa3u,0x20u,4u,
        0xb8u,0x34u,0x12u, 0xa3u,0x22u,4u, 0xa1u,0x90u,5u,
        0xa3u,0u,0x30u, 0xf4u
    };
    const char empty[10] = {0};
    const lib_u16 collision_empty[5] = {0};
    const lib_u8 lanes[] = {0x78u,0x56u,0x34u,0x12u};
    const lib_u8 mode_bytes[] = {3u,0u,1u};
    const char lane_digits[][10] = {"021      ","63122    ","8000343  ","698914503"};
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_run_result result = {0};
    lib_u8 actual[2];
    lib_i32 failed = 1;

    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK) return 1;
    machine = myraidenii_core_board_machine_get(board);
    if (board_cop_decimal_result(machine, empty)) goto done;
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (board_cop_word_write(machine, 0x424u, cases[index].mode) != LIB_STATUS_OK ||
            board_cop_dword_write(machine, 0x420u, cases[index].number) != LIB_STATUS_OK ||
            board_cop_decimal_result(machine, cases[index].expected)) {
            lib_c_printf("COP:DECIMAL:VECTOR:FAIL case=%u\n", (lib_u32)index);
            goto done;
        }
    }
    /* Mode changes alone preserve the completed bytes, including padding. */
    if (board_cop_word_write(machine, 0x424u, 3u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "1        ") ||
        board_cop_word_write(machine, 0x420u, 1u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "100000000") ||
        board_cop_word_write(machine, 0x424u, 0x103u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "100000000") ||
        board_cop_word_write(machine, 0x422u, 0u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "1        ") ||
        x86_core_machine_memory_write(machine, 0x424u, mode_bytes, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x425u, mode_bytes + 1u, 1u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "1        ") ||
        board_cop_word_write(machine, 0x420u, 1u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "100000000") ||
        x86_core_machine_memory_write(machine, 0x425u, mode_bytes + 2u, 1u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "100000000") ||
        board_cop_word_write(machine, 0x422u, 0u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "1        ") ||
        board_cop_dword_write(machine, 0x420u, 0u) != LIB_STATUS_OK)
        goto done;
    for (lib_u8 index = 0u; index < sizeof(lanes); ++index) {
        if (x86_core_machine_memory_write(machine, 0x420u + index, &lanes[index], 1u) !=
                LIB_STATUS_OK || board_cop_decimal_result(machine, lane_digits[index]))
            goto done;
    }
    if (board_cop_word_write(machine, 0x421u, 0xcdabu) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "466864513") ||
        board_cop_collision_result(machine, collision_empty))
        goto done;
    for (lib_u8 index = 0u; index < 3u; ++index) {
        if (x86_core_machine_memory_read(machine, 0x420u + index * 2u, actual, 2u) !=
                LIB_STATUS_INVALID_STATE ||
            x86_core_machine_memory_inspect(machine, 0x420u + index * 2u, actual, 2u) !=
                LIB_STATUS_INVALID_STATE)
            goto done;
    }
    for (lib_u8 index = 0u; index < 5u; ++index) {
        if (board_cop_word_write(machine, 0x590u + index * 2u, 0xffffu) != LIB_STATUS_INVALID_STATE)
            goto done;
    }
    if (x86_core_machine_memory_write(machine, 0x599u, lanes, 1u) != LIB_STATUS_INVALID_STATE ||
        board_cop_decimal_result(machine, "466864513") ||
        x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine, (x86_core_machine_run_budget){9u,0u}, &result) !=
            LIB_STATUS_OK || result.executed != 9u ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        board_cop_decimal_result(machine, "698914503") ||
        x86_core_machine_memory_inspect(machine, 0x3000u, actual, 2u) != LIB_STATUS_OK ||
        actual[0] != '6' || actual[1] != '9' ||
        board_cop_upload(machine, 0x5205u, 0u, 8u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, 0x5205u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x420u, 1u) != LIB_STATUS_INVALID_STATE ||
        board_cop_word_write(machine, 0x424u, 0u) != LIB_STATUS_INVALID_STATE ||
        board_cop_decimal_result(machine, "698914503") ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, empty) ||
        board_cop_word_write(machine, 0x420u, 0u) != LIB_STATUS_OK ||
        board_cop_decimal_result(machine, "0        "))
        goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("COP:DECIMAL:FAIL reason=%u detail=%u\n",
        (lib_u32)result.reason, result.detail);
    myraidenii_core_board_destroy(board);
    return failed;
}

static lib_i32 board_cop_random(void)
{
    const myraidenii_core_board_config config = {
        .program_even = program_even,
        .program_odd = program_odd,
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x800u}
    };
    const lib_u8 guest[] = {0x90u, 0x90u, 0xa1u, 0xa4u, 0x05u, 0xa3u, 0x00u, 0x30u};
    const lib_u8 low = 0u, high = 1u;
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    x86_core_machine_run_result result = {0};
    x86_core_machine_timeline_observation before;
    lib_u8 value[2];
    lib_i32 failed = 1;

    if (myraidenii_core_board_create(&config, &board) != LIB_STATUS_OK ||
        (machine = myraidenii_core_board_machine_get(board)) == LIB_NULL ||
        board_cop_random_result(machine, 0u) ||
        board_cop_word_write(machine, 0x42cu, 65535u) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x800u, guest, sizeof(guest)) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
        result.executed != 2u ||
        x86_core_machine_get_timeline_observation(machine, &before) != LIB_STATUS_OK ||
        before.now == 0u ||
        x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
        result.executed != 2u ||
        x86_core_machine_memory_read(machine, 0x3000u, value, sizeof(value)) != LIB_STATUS_OK ||
        (lib_u16)(value[0] | (lib_u16)value[1] << 8u) != (lib_u16)before.now)
        goto done;

    if (x86_core_machine_memory_write(machine, 0x42cu, &low, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x42du, &high, 1u) != LIB_STATUS_OK ||
        x86_core_machine_get_timeline_observation(machine, &before) != LIB_STATUS_OK ||
        board_cop_random_result(machine, (lib_u16)(before.now % 257u)) ||
        x86_core_machine_memory_read(machine, 0x42cu, value, sizeof(value)) !=
            LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_inspect(machine, 0x42cu, value, sizeof(value)) !=
            LIB_STATUS_INVALID_STATE)
        goto done;
    for (lib_u32 address = 0x5a0u; address < 0x5a8u; ++address) {
        if (x86_core_machine_memory_write(machine, address, &high, 1u) !=
                LIB_STATUS_INVALID_STATE ||
            ((address & 1u) == 0u && board_cop_word_write(machine, address, 1u) !=
                LIB_STATUS_INVALID_STATE))
            goto done;
    }
    if (board_cop_upload(machine, 0x5205u, 0u, 8u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x500u, 0x5205u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x42cu, 0u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x42du, &low, 1u) != LIB_STATUS_INVALID_STATE ||
        board_cop_random_result(machine, (lib_u16)(before.now % 257u)) ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
        result.executed != 2u ||
        x86_core_machine_get_timeline_observation(machine, &before) != LIB_STATUS_OK ||
        before.now == 0u ||
        board_cop_random_result(machine, 0u))
        goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("COP:RANDOM:FAIL reason=%u detail=%u\n",
        (lib_u32)result.reason, result.detail);
    myraidenii_core_board_destroy(board);
    return failed;
}

static lib_status board_cop_dma_context_write(x86_core_machine *machine, lib_u16 mode,
    lib_u16 source, lib_u16 size, lib_u16 destination)
{
    lib_status status = board_cop_word_write(machine, 0x47eu, mode);

    if (status == LIB_STATUS_OK) status = board_cop_word_write(machine, 0x478u, source);
    if (status == LIB_STATUS_OK) status = board_cop_word_write(machine, 0x47au, size);
    if (status == LIB_STATUS_OK) status = board_cop_word_write(machine, 0x47cu, destination);
    return status;
}

static lib_i32 board_cop_dma_and_sort(void)
{
    cop_fixture fixture = {0};
    x86_core_machine *machine;
    lib_u8 source[224];
    lib_u8 actual[224];
    const lib_u8 fill[] = {0x44u,0x33u,0x22u,0x11u};
    const lib_u16 offsets[] = {0x10u,0x04u,0x18u,0x0cu};
    const lib_u16 ascending[] = {0x10u,0x0cu,0x04u,0x18u};
    const lib_u16 descending[] = {0x04u,0x18u,0x0cu,0x10u};
    const lib_i32 keys[] = {-1,5,5,0};
    lib_u8 value[4] = {0u};
    lib_i32 failed = 1;

    for (lib_size index = 0u; index < sizeof(source); ++index)
        source[index] = (lib_u8)index;
    if (cop_fixture_create(&fixture) != LIB_STATUS_OK) return 1;
    machine = fixture.machine;

    /* Each mode owns its own byte-merged setup, not a flat register mirror. */
    if (board_cop_word_write(machine, 0x47eu, 0x009u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x478u, (lib_u8[]){0x80u}, 1u) !=
            LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x479u, (lib_u8[]){0u}, 1u) !=
            LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x47au, 0x201u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x47cu, 0x100u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x2000u, source, 64u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x4000u, (lib_u8[64]){0}, 64u) !=
            LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x478u, 0u) != LIB_STATUS_INVALID_STATE ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x4000u, actual, 64u) != LIB_STATUS_OK ||
        lib_memory_compare(actual, source, 64u) != 0)
        goto done;

    if (board_cop_dma_context_write(machine, 0x00eu, 0x081u, 0x206u, 0x102u) !=
            LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x2040u, source + 64u, 128u) !=
            LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x4080u, actual, 96u) != LIB_STATUS_OK ||
        lib_memory_compare(actual, source + 64u, 96u) != 0 ||
        board_cop_word_write(machine, 0x47eu, 0x009u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x4000u, actual, 64u) != LIB_STATUS_OK ||
        lib_memory_compare(actual, source, 64u) != 0)
        goto done;

    if (board_cop_word_write(machine, 0x47eu, 0x200u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x478u, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_memory_write(machine, 0x6fcu, (lib_u8[]){0u}, 1u) !=
            LIB_STATUS_INVALID_STATE ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_INVALID_STATE ||
        board_cop_dma_context_write(machine, 0x009u, 0x080u, 1u, 1u) != LIB_STATUS_OK ||
        value[0] != 0u || x86_core_machine_memory_write(machine, 0x40u,
            (lib_u8[]){0xa5u}, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x40u, value, 1u) != LIB_STATUS_OK ||
        value[0] != 0xa5u ||
        board_cop_dma_context_write(machine, 0x009u, 0u, 0u, 0x100u) !=
            LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x4000u, (lib_u8[]){0xa5u}, 1u) !=
            LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) == LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x4000u, actual, 1u) != LIB_STATUS_OK ||
        actual[0] != 0xa5u)
        goto done;

    if (board_cop_dma_context_write(machine, 0x009u, 0x080u, 260u, 0x081u) !=
            LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x2000u, source, 96u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x2040u, actual, 96u) != LIB_STATUS_OK ||
        lib_memory_compare(actual, source, 64u) != 0 ||
        lib_memory_compare(actual + 64u, source, 32u) != 0)
        goto done;

    if (board_cop_word_write(machine, 0x428u, 0x3344u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x42au, 0x1122u) != LIB_STATUS_OK ||
        board_cop_dma_context_write(machine, 0x116u, 0x120u, 1u, 0x777u) !=
            LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x4800u, actual, 32u) != LIB_STATUS_OK)
        goto done;
    for (lib_size index = 0u; index < 32u; ++index)
        if (actual[index] != fill[index % sizeof(fill)]) goto done;
    if (board_cop_dma_context_write(machine, 0x118u, 0x140u, 0u, 0u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x5000u, actual, 32u) != LIB_STATUS_OK)
        goto done;
    for (lib_size index = 0u; index < 32u; ++index)
        if (actual[index] != fill[index % sizeof(fill)]) goto done;
    if (board_cop_dma_context_write(machine, 0x118u, 0x140u, 0u, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) == LIB_STATUS_OK)
        goto done;

    if (board_cop_word_write(machine, 0x450u, 0u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x452u, 0x3000u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x454u, 0u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x456u, 0x3200u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x3200u, offsets, sizeof(offsets)) !=
            LIB_STATUS_OK)
        goto done;
    for (lib_size index = 0u; index < sizeof(offsets) / sizeof(offsets[0]); ++index) {
        const lib_u32 key_value = (lib_u32)keys[index];
        const lib_u8 key[4] = {(lib_u8)key_value, (lib_u8)(key_value >> 8u),
            (lib_u8)(key_value >> 16u), (lib_u8)(key_value >> 24u)};

        if (x86_core_machine_memory_write(machine, 0x3000u + offsets[index], key,
                sizeof(key)) != LIB_STATUS_OK)
            goto done;
    }
    if (board_cop_word_write(machine, 0x458u, 1u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6feu, 4u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x450u, 1u) != LIB_STATUS_INVALID_STATE ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x3200u, actual, sizeof(offsets)) !=
            LIB_STATUS_OK || lib_memory_compare(actual, ascending, sizeof(ascending)) != 0 ||
        board_cop_word_write(machine, 0x458u, 2u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6feu, 4u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x3200u, actual, sizeof(offsets)) !=
            LIB_STATUS_OK || lib_memory_compare(actual, descending, sizeof(descending)) != 0 ||
        board_cop_word_write(machine, 0x458u, 0u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6feu, 4u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x3200u, actual, sizeof(offsets)) !=
            LIB_STATUS_OK || lib_memory_compare(actual, descending, sizeof(descending)) != 0 ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_dma_context_write(machine, 0x00eu, 0xffffu, 0u, 0u) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x47eu, 0x00eu) != LIB_STATUS_OK ||
        board_cop_word_write(machine, 0x6fcu, 0u) != LIB_STATUS_OK ||
        myraidenii_core_cop_advance(fixture.cop) != LIB_STATUS_OK)
        goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("COP:DMA-SORT:FAIL\n");
    cop_fixture_destroy(&fixture);
    return failed;
}

lib_i32 main(void)
{
    return board_invalid_arguments() || board_program_and_ram_routes() || board_control_window() ||
        board_interrupt_input_and_sound_routes() || board_video_phase() || board_video_sprite_flags() ||
        board_video_interrupts() || board_cop_routes() || board_cop_reset_and_admission() ||
        board_cop_transfers() || board_cop_updates() || board_cop_angle_approach() ||
        board_cop_offset_add() ||
        board_cop_numeric_vectors() || board_cop_numeric_failures() || board_cop_numeric_chain() ||
        board_cop_collisions() || board_cop_decimal() || board_cop_random() ||
        board_cop_dma_and_sort();
}
