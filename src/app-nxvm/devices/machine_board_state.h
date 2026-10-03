#ifndef CORE_MACHINE_BOARD_STATE_H
#define CORE_MACHINE_BOARD_STATE_H

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/pic_bus.h"
#include "app-nxvm/devices/pit_bus.h"
#include "app-nxvm/devices/dma_bus.h"
#include "app-nxvm/devices/d4_memory.h"
#include "x86/chips/rtc146818/rtc146818_interface.h"
#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/devices/hdc.h"
#include "app-nxvm/devices/kbc.h"
#include "app-nxvm/devices/xt_ppi_keyboard.h"
#include "x86/chips/xtkeyboard/xtkeyboard_interface.h"
#include "app-nxvm/devices/vadp.h"

typedef struct core_machine_fdc_topology {
    const core_machine_media_registry *media_registry;
    core_machine_fdc_drive_bindings drives;
    core_machine_dma_request_binding dma_request;
    core_machine_fdc_config config;
    core_machine_fdc_terminal_observation_provider observation_provider;
} core_machine_fdc_topology;

typedef struct core_machine_hdc_topology {
    const core_machine_media_registry *media_registry;
    core_machine_media_id media_id;
    core_machine_media_id slave_media_id;
    core_machine_hdc_config config;
} core_machine_hdc_topology;

struct core_machine_plan {
    core_machine_config configuration;
    core_machine_plan_topology topology;
    core_machine_timing_declaration declarations[
        CORE_MACHINE_TIMING_CAPABILITY_COUNT];
    lib_size declaration_count;
    core_machine_controller_timing_rules controller_timing;
    core_machine_d4_memory_config d4_memory;
    const core_machine_media_registry *media_registry;
    core_machine_display_provider_slot *display_provider;
    core_machine_fdc_terminal_observation_provider fdc_observation_provider;
};

typedef struct core_machine_absent_memory {
    core_machine_absent_memory_config config;
    lib_u8 configured;
} core_machine_absent_memory;

/* One board-owned lifetime. Fields move here by their actual owner; Core
 * retains only the opaque attachment pointer and bounded callbacks. */
struct core_machine_board_state {
    /* Execution is Core-owned; this handle is borrowed until finalization. */
    core_machine *core;
    core_machine_pic_bus shared_pic_master;
    core_machine_pic_bus shared_pic_slave;
    core_machine_pic_irq_source shared_pit_irq0_source;
    core_machine_pic_irq_source rtc_irq_source;
    core_machine_pit_bus shared_pit;
    core_machine_pit_bus auxiliary_pit;
    lib_u8 auxiliary_pit_configured;
    t_latch shared_dma_latch;
    t_dma shared_dma_primary;
    t_dma shared_dma_secondary;
    x86_rtc *shared_rtc;
    lib_u8 rtc_selected_register;
    core_machine_fdc fdc;
    core_machine_hdc hdc;
    core_machine_d4_memory d4_memory;
    core_machine_controller_timing_rules controller_timing;
    t_kbc shared_kbc;
    core_machine_xt_ppi_keyboard xt_ppi_keyboard;
    x86_xt_keyboard *xt_keyboard;
    core_machine_planar_parity_config planar_parity_config;
    lib_u8 planar_parity_port_b;
    lib_u8 planar_parity_configured;
    lib_u8 planar_parity_latched;
    lib_u8 planar_parity_nmi_signaled;
    core_machine_d4_platform_config d4_platform_config;
    lib_u8 d4_platform_port_b;
    lib_u8 d4_platform_configured;
    lib_u8 d4_platform_iochk_latched;
    lib_u8 d4_platform_failsafe_latched;
    lib_u8 d4_platform_nmi_signaled;
    /* D4 refresh signal state; Core services its request at arbitration. */
    lib_u8 d4_refresh_hold_pending;
    lib_u8 d4_refresh_pulse_active;
    lib_u8 d4_refresh_address;
    lib_u8 xt_ppi_speaker_configured;
    lib_u8 xt_ppi_speaker_gate;
    lib_u8 xt_ppi_speaker_data_enabled;
    lib_u8 speaker_output;
    core_machine_absent_memory absent_memory[CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT];
    t_vadp shared_vadp;
    core_machine_clock_domain dma_clock;
    lib_u8 dma_clock_explicit;
    core_machine_clock_domain pit_clock;
    core_machine_clock_domain auxiliary_pit_clock;
    core_machine_clock_domain rtc_clock;
    core_machine_clock_domain vadp_clock;
    core_machine_clock_domain kbc_clock;
    lib_u32 kbc_typematic_initial_ticks;
    lib_u32 kbc_typematic_repeat_ticks;
    lib_u32 kbc_command_response_ticks;
    lib_u8 kbc_command_response_status_polls;
    lib_u32 kbc_serial_delivery_ticks;
    lib_u8 kbc_input_port_configured;
    lib_u8 kbc_input_port;
    core_machine_keyboard_topology keyboard_topology;
    core_machine_display_port_topology display_ports;
    lib_u8 display_configured;
    core_machine_dma_wiring dma_wiring;
    core_machine_dma_request_binding fdc_dma_request;
    core_machine_dma_request_binding hdc_dma_request;
    core_machine_dma_request_binding refresh_dma_request;
    lib_u8 dma_configured;
    core_machine_rtc_cmos_config rtc_cmos_config;
    lib_u8 rtc_cmos_configured;
    core_machine_fdc_topology fdc_topology;
    lib_u8 fdc_configured;
    core_machine_hdc_topology hdc_topology;
    lib_u8 hdc_configured;
};

lib_status core_machine_configure_fdc(core_machine_board_state *board,
    const core_machine_fdc_topology *topology);
lib_status core_machine_configure_hdc(core_machine_board_state *board,
    const core_machine_hdc_topology *topology);
void core_machine_board_reset_devices(void *owner);
/* Private board construction failure seams use the one production factory. */
lib_status core_machine_create_internal(const core_machine_config *config,
    core_machine **out_machine,
    core_machine_memory_test_allocation *test_allocation,
    core_machine_port_test_allocation *port_test_allocation,
    core_machine_board_state **out_board);
lib_status core_machine_create_with_test_memory_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_memory_test_allocation *test_allocation,
    core_machine_board_state **out_board);
lib_status core_machine_create_with_test_port_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_port_test_allocation *test_allocation,
    core_machine_board_state **out_board);
lib_i32 core_machine_clock_plan_is_valid(const core_machine_clock_plan *plan);
lib_i32 core_machine_board_config_is_valid(const core_machine_config *config);
lib_status core_machine_board_create(core_machine *machine,
    const core_machine_config *config);
void core_machine_board_finalize_devices(void *owner);
lib_bool core_machine_board_shutdown_resets(void *owner);
lib_status core_machine_board_initialize_clocks(core_machine_board_state *board,
    const core_machine_clock_plan *plan);
void core_machine_board_reset_clocks(void *owner);
void core_machine_board_deadline_observe(void *owner, lib_u64 now,
    lib_bool timing_qualified,
    core_machine_attachment_deadline_observation *out_observation);
lib_bool core_machine_board_refresh_request(void *owner, lib_u8 *out_address);
void core_machine_board_refresh_complete(void *owner);
lib_u64 core_machine_board_dma_ticks(void *owner, lib_u64 source_ticks);
lib_bool core_machine_board_dma_request(void *owner);
void core_machine_board_dma_advance(void *owner, lib_u64 dma_ticks);
core_machine_attachment_pit_ticks core_machine_board_pit_ticks_advance(void *owner,
    lib_u64 source_ticks);
void core_machine_board_pit_pic_advance(void *owner,
    core_machine_attachment_pit_ticks ticks);
lib_bool core_machine_board_pic_pending(void *owner);
lib_u8 core_machine_board_pic_acknowledge(void *owner);
void core_machine_board_media_advance(void *owner, lib_u64 source_ticks,
    lib_u64 due_tick);
void core_machine_board_rtc_advance(void *owner, lib_u64 source_ticks);
void core_machine_board_peripheral_advance(void *owner, lib_u64 source_ticks);
lib_status core_machine_board_register_a20_port(core_machine *machine);
void core_machine_board_after_pit_reset(core_machine_board_state *board);
void core_machine_board_refresh_nmi(void *owner);
void core_machine_board_configure_xt_ppi_speaker(core_machine_board_state *board);
void core_machine_board_set_xt_ppi_speaker(core_machine_board_state *board,
    lib_u8 timer_gate, lib_u8 data_enabled);
lib_status core_machine_plan_validate(const core_machine_plan *plan);
lib_status core_machine_plan_apply_topology(core_machine *machine,
    core_machine_board_state *board,
    const core_machine_plan *plan);

#endif
