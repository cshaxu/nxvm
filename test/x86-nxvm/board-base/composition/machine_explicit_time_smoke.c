#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/chips/rtc146818/rtc146818_interface.h"

static lib_u8 machine_explicit_time_second(x86_core_machine *machine)
{
    lib_u32 value = 0u;

    if (x86_core_machine_bus_write(machine, 0x70u, 0u) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x71u, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return (lib_u8)value;
}

static lib_i32 machine_explicit_time_prepare(x86_core_machine **out_machine,
    x86_nxvm_board_state **out_board)
{
    const x86_nxvm_board_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80286,
        .ticks_per_instruction = 1u
    };
    const x86_nxvm_board_rtc_cmos_config rtc = {
        .index_port = 0x70u,
        .data_port = 0x71u,
        .irq = 8u,
        .nmi_mask_bit = 0x80u,
        .ticks_per_second = 1u
    };
    const x86_core_machine_memory_alias_config reset_alias = {
        0x00fffff0u, 0x000ffff0u, 16u
    };
    x86_nxvm_board_plan *plan = LIB_NULL;
    lib_bool prepared;

    if (x86_nxvm_board_plan_create(&config, &plan) != LIB_STATUS_OK) return LIB_FALSE;
    prepared = x86_nxvm_board_create_from_plan(plan, out_machine, out_board) == LIB_STATUS_OK &&
        x86_nxvm_board_configure_rtc_cmos(*out_board, &rtc) == LIB_STATUS_OK &&
        x86_core_machine_install_memory_aliases(*out_machine, &reset_alias, 1u,
            LIB_FALSE) == LIB_STATUS_OK &&
        x86_core_machine_freeze_execution_providers(*out_machine) == LIB_STATUS_OK &&
        x86_core_machine_reset(*out_machine) == LIB_STATUS_OK;
    x86_nxvm_board_plan_destroy(plan);
    return prepared;
}

/* A real 1-Hz RTC supplies each deadline; the test does not publish ticks or
 * install another time provider merely to advance the Board's devices. */
static lib_i32 machine_explicit_time_advance_seconds(x86_core_machine *machine,
    lib_u32 seconds)
{
    /* RESET clears interrupt enables, not calendar phase. Program the guest
     * register through the same ports before requesting the next update. */
    if (x86_core_machine_bus_write(machine, 0x70u, X86_RTC_REG_B) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x71u,
            X86_RTC_REG_B_UIE | X86_RTC_REG_B_24H) != LIB_STATUS_OK) return 1;
    for (lib_u32 index = 0u; index < seconds; ++index) {
        lib_u8 advanced = LIB_FALSE;
        lib_u64 before = 0u, after = 0u;

        if (x86_core_machine_get_elapsed_ticks(machine, &before) != LIB_STATUS_OK ||
            x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
            !advanced ||
            x86_core_machine_get_elapsed_ticks(machine, &after) != LIB_STATUS_OK ||
            after != before + 1u) {
            return 1;
        }
    }
    return 0;
}

static lib_i32 machine_explicit_time_rejects_unstarted_lifecycle(void)
{
    const x86_nxvm_board_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80286
    };
    x86_core_machine *machine = LIB_NULL;
    lib_u64 elapsed = 1u;
    lib_u8 advanced = LIB_FALSE;
    lib_i32 failed = x86_nxvm_board_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_INVALID_STATE;

    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    const lib_u8 halt = 0xf4u;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result first = {0};
    x86_core_machine_run_result second = {0};
    x86_core_machine_timeline_observation before;
    x86_core_machine_timeline_observation after;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_u64 elapsed = 0u;
    lib_u8 second_before_explicit_time = 0u;
    lib_i32 failed = 0;

    if (machine_explicit_time_rejects_unstarted_lifecycle() ||
        !machine_explicit_time_prepare(&machine, &board) ||
        x86_core_machine_memory_write(machine, 0x00fffff0u, &halt, sizeof(halt)) !=
            LIB_STATUS_OK ||
        x86_core_machine_run(machine, budget, &first) != LIB_STATUS_OK ||
        first.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        first.executed != 1u || first.elapsed_ticks == 0u ||
        x86_core_machine_get_timeline_observation(machine, &before) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, budget, &second) != LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return 1;
    }
    second_before_explicit_time = machine_explicit_time_second(machine);

    failed |= second.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        second.executed != 0u || second.ticks != 0u ||
        second.elapsed_ticks != first.elapsed_ticks ||
        x86_core_machine_get_timeline_observation(machine, &after) != LIB_STATUS_OK ||
        after.now != before.now || after.next_sequence != before.next_sequence ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != first.elapsed_ticks;
    failed |= machine_explicit_time_advance_seconds(machine, 5u) ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != first.elapsed_ticks + 5u ||
        x86_core_machine_get_timeline_observation(machine, &after) != LIB_STATUS_OK ||
        after.now != before.now + 5u ||
        machine_explicit_time_second(machine) != second_before_explicit_time + 5u;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 0u || machine_explicit_time_second(machine) !=
            second_before_explicit_time + 5u ||
        machine_explicit_time_advance_seconds(machine, 1u) ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 1u;

    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("EXPLICIT-MACHINE-TIME:OK\n");
    return 0;
}
