#ifndef TEST_BOARD_COMPOSITION_FIXTURE_H
#define TEST_BOARD_COMPOSITION_FIXTURE_H
#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/ibmpc-common/dma_bus_interface.h"

typedef struct test_board_composition_observation {
    core_machine_fdc_config fdc;
    core_machine_hdc_config hdc;
    lib_u8 rtc_irq;
    core_machine_rtc_timing_provenance rtc_provenance;
    lib_u16 drive_cylinders;
    lib_bool dma_configured;
    core_machine_dma_wiring dma_wiring;
    core_machine_clock_ratio dma_clock;
    core_machine_clock_ratio pit_clock;
    core_machine_clock_ratio auxiliary_pit_clock;
    core_machine_clock_ratio rtc_clock;
    core_machine_clock_ratio vadp_clock;
    lib_u32 rtc_ticks_per_second;
    core_machine_controller_timing_rules controller_timing;
    lib_u32 kbc_typematic_initial_ticks;
    lib_u32 kbc_typematic_repeat_ticks;
    lib_u32 kbc_command_response_ticks;
    core_machine_fdc_drive_bindings drives;
    lib_bool auxiliary_pit_configured;
    lib_bool fdc_configured;
    lib_bool hdc_configured;
    lib_bool rtc_cmos_configured;
} test_board_composition_observation;

typedef struct test_board_plan_observation {
    lib_size memory_bytes;
    core_machine_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    core_machine_retirement_time_contract retirement_time_contract;
    core_machine_l1_compatibility_policy l1_compatibility_policy;
    lib_u8 cpu_80386_cr_mov_ignores_mod;
    core_machine_time_axis_kind time_axis_kind;
    core_machine_controller_timing_rules controller_timing;
} test_board_plan_observation;

test_board_plan_observation test_board_capture_plan(const core_machine_plan *plan);
lib_bool test_board_plan_timing_matches(const core_machine_plan *plan,
    const core_machine_config *expected,
    const core_machine_controller_timing_rules *controller_timing);
lib_bool test_board_instances_are_distinct(const core_machine_board_state *first,
    const core_machine_board_state *second);

typedef enum test_board_pic_source {
    TEST_BOARD_PIT_IRQ0,
    TEST_BOARD_KEYBOARD_IRQ1,
    TEST_BOARD_KEYBOARD_IRQ12
} test_board_pic_source;

test_board_composition_observation test_board_capture_composition(
    const core_machine_board_state *board);
lib_bool test_board_pic_source_matches(core_machine_board_state *board,
    test_board_pic_source source, lib_u8 irq);
lib_bool test_board_kbc_command_matches(core_machine_board_state *board,
    core_machine *machine, lib_u8 command, lib_u8 mask, lib_u8 expected);
lib_i32 test_board_pic_unmask_delay_matches(core_machine_board_state *board,
    lib_u64 expected_ticks);
lib_status test_board_dma_duplicate_bind(core_machine_board_state *board, lib_u8 channel,
    const core_machine_dma_channel_provider *provider, core_machine_dma_request_binding *request);
lib_status test_board_dma_bind_channel(core_machine_board_state *board, lib_u8 channel,
    const core_machine_dma_channel_provider *provider, void *context,
    core_machine_dma_request_binding *request);
lib_bool test_board_dma_has_pending_request(const core_machine_board_state *board);
void test_board_dma_request_assert(core_machine_board_state *board,
    const core_machine_dma_request_binding *request);
void test_board_pit_advance(core_machine_board_state *board, lib_u64 ticks);
lib_i32 test_board_kbc_read_reply(core_machine_board_state *board, core_machine *machine);
lib_status test_board_kbc_submit_native_byte(core_machine_board_state *board, lib_u8 byte);
lib_status test_board_kbc_ticks_until_event(const core_machine_board_state *board,
    lib_u64 *ticks);
void test_board_kbc_advance(core_machine_board_state *board, lib_u64 ticks);
#endif
