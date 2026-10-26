#ifndef X86_NXVM_BOARD_STATE_H
#define X86_NXVM_BOARD_STATE_H

#include "x86-core/machine/clock_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-nxvm/board-base/pit_bus_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"

#include "x86-nxvm/chips/rtc146818/rtc146818_interface.h"
#include "x86-nxvm/board-base/fdc_interface.h"
#include "x86-nxvm/board-base/hdc_interface.h"
#include "x86-nxvm/board-at/kbc_interface.h"
#include "x86-nxvm/board-at/parity_interface.h"
#include "x86-nxvm/board-xt/xt_ppi_keyboard_interface.h"
#include "x86-nxvm/chips/xtkeyboard/xtkeyboard_interface.h"
#include "x86-nxvm/board-base/vadp_interface.h"

typedef struct x86_nxvm_board_fdc_topology {
    const x86_nxvm_board_media_registry *media_registry;
    x86_nxvm_board_fdc_drive_bindings drives;
    x86_nxvm_board_dma_request_binding dma_request;
    x86_nxvm_board_fdc_config config;
    x86_nxvm_board_fdc_terminal_observation_provider observation_provider;
} x86_nxvm_board_fdc_topology;

typedef struct x86_nxvm_board_hdc_topology {
    const x86_nxvm_board_media_registry *media_registry;
    x86_nxvm_board_media_id media_id;
    x86_nxvm_board_media_id slave_media_id;
    x86_nxvm_board_hdc_config config;
} x86_nxvm_board_hdc_topology;

struct x86_nxvm_board_plan {
    x86_nxvm_board_config configuration;
    x86_nxvm_board_plan_topology topology;
    x86_nxvm_board_timing_declaration declarations[
        X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT];
    lib_size declaration_count;
    x86_nxvm_board_controller_timing_rules controller_timing;
    x86_nxvm_board_profile_factory profile_factory;
    void *profile_factory_context;
    const x86_nxvm_board_media_registry *media_registry;
    x86_nxvm_board_display_provider_slot *display_provider;
    x86_nxvm_board_fdc_terminal_observation_provider fdc_observation_provider;
};

typedef struct x86_nxvm_board_absent_memory {
    x86_nxvm_board_absent_memory_config config;
    lib_u8 configured;
} x86_nxvm_board_absent_memory;

/* One board-owned lifetime. Fields move here by their actual owner; Core
 * retains only the opaque attachment pointer and bounded callbacks. */
struct x86_nxvm_board_state {
    /* Execution is Core-owned; this handle is borrowed until finalization. */
    x86_core_machine *core;
    x86_nxvm_board_timing_declaration timing_declarations[
        X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT];
    lib_bool timing_declarations_copied;
    x86_nxvm_board_pic_bus *shared_pic_master;
    x86_nxvm_board_pic_bus *shared_pic_slave;
    x86_nxvm_board_pic_irq_source *shared_pit_irq0_source;
    x86_nxvm_board_pic_irq_source *rtc_irq_source;
    x86_pit *shared_pit;
    x86_pit *auxiliary_pit;
    lib_u8 auxiliary_pit_configured;
    x86_nxvm_board_dma_bus *shared_dma;
    x86_rtc *shared_rtc;
    lib_u8 rtc_selected_register;
    x86_nxvm_board_fdc *fdc;
    x86_nxvm_board_hdc *hdc;
    x86_nxvm_board_controller_timing_rules controller_timing;
    t_kbc *shared_kbc;
    x86_nxvm_board_pic_irq_source *keyboard_irq1_source;
    x86_nxvm_board_pic_irq_source *keyboard_irq12_source;
    x86_nxvm_board_xt_ppi_keyboard *xt_ppi_keyboard;
    x86_xt_keyboard *xt_keyboard;
    x86_nxvm_board_at_parity *planar_parity;
    x86_nxvm_board_profile_binding profile_binding;
    lib_u8 xt_ppi_speaker_configured;
    lib_u8 speaker_output;
    lib_u8 speaker_lines;
    x86_nxvm_board_absent_memory absent_memory[X86_NXVM_BOARD_ABSENT_MEMORY_WINDOW_COUNT];
    t_vadp *shared_vadp;
    x86_core_machine_clock_domain dma_clock;
    lib_u8 dma_clock_explicit;
    x86_core_machine_clock_domain pit_clock;
    x86_core_machine_clock_domain auxiliary_pit_clock;
    x86_core_machine_clock_domain rtc_clock;
    x86_core_machine_clock_domain vadp_clock;
    x86_core_machine_clock_domain kbc_clock;
    lib_u32 kbc_typematic_initial_ticks;
    lib_u32 kbc_typematic_repeat_ticks;
    lib_u32 kbc_command_response_ticks;
    lib_u8 kbc_command_response_status_polls;
    lib_u32 kbc_serial_delivery_ticks;
    lib_u8 kbc_input_port_configured;
    lib_u8 kbc_input_port;
    lib_u8 kbc_reset_system_flag_configured;
    lib_u8 kbc_reset_system_flag;
    x86_nxvm_board_keyboard_topology keyboard_topology;
    x86_nxvm_board_display_topology display_topology;
    x86_nxvm_board_system_facade system_facade;
    lib_u8 system_control;
    lib_bool keyboard_a20_enabled;
    x86_nxvm_board_display_port_topology display_ports;
    lib_u8 display_configured;
    x86_nxvm_board_dma_wiring dma_wiring;
    x86_nxvm_board_dma_request_binding fdc_dma_request;
    x86_nxvm_board_dma_request_binding hdc_dma_request;
    x86_nxvm_board_dma_request_binding refresh_dma_request;
    lib_bool refresh_hold_pending;
    lib_u8 refresh_row;
    lib_u8 dma_configured;
    x86_nxvm_board_rtc_cmos_config rtc_cmos_config;
    lib_u8 rtc_cmos_configured;
    x86_nxvm_board_fdc_topology fdc_topology;
    lib_u8 fdc_configured;
    x86_nxvm_board_hdc_topology hdc_topology;
    lib_u8 hdc_configured;
};

lib_status x86_nxvm_board_configure_fdc(x86_nxvm_board_state *board,
    const x86_nxvm_board_fdc_topology *topology);
lib_status x86_nxvm_board_configure_hdc(x86_nxvm_board_state *board,
    const x86_nxvm_board_hdc_topology *topology);
void x86_nxvm_board_reset_devices(void *owner);
/* Board owns projection/preflight; no candidate exists before it succeeds. */
lib_status x86_nxvm_board_prepare_executor(
    const x86_nxvm_board_config *config, x86_core_machine_executor_config *out_executor);
lib_i32 x86_nxvm_board_clock_plan_is_valid(const x86_nxvm_board_clock_plan *plan);
lib_i32 x86_nxvm_board_config_is_valid(const x86_nxvm_board_config *config);
lib_status x86_nxvm_board_attach_machine(x86_core_machine *machine,
    const x86_nxvm_board_config *config, x86_nxvm_board_state **out_board);
void x86_nxvm_board_finalize_devices(void *owner);
lib_bool x86_nxvm_board_shutdown_resets(void *owner);
lib_status x86_nxvm_board_initialize_clocks(x86_nxvm_board_state *board,
    const x86_nxvm_board_clock_plan *plan);
void x86_nxvm_board_reset_clocks(void *owner);
void x86_nxvm_board_deadline_observe(void *owner, lib_u64 now,
    x86_core_machine_attachment_deadline_observation *out_observation);
lib_bool x86_nxvm_board_refresh_request(void *owner, lib_u8 *out_address);
void x86_nxvm_board_refresh_complete(void *owner);
lib_bool x86_nxvm_board_cpu_hold_asserted(void *owner);
lib_u64 x86_nxvm_board_dma_ticks(void *owner, lib_u64 source_ticks);
lib_bool x86_nxvm_board_dma_request(void *owner);
void x86_nxvm_board_dma_advance(void *owner, lib_u64 dma_ticks);
void x86_nxvm_board_before_internal_advance(void *owner,
    lib_u64 source_ticks, lib_u64 due_tick);
lib_bool x86_nxvm_board_pic_pending(void *owner);
lib_u8 x86_nxvm_board_pic_acknowledge(void *owner);
void x86_nxvm_board_after_internal_advance(void *owner,
    lib_u64 source_ticks, lib_u64 due_tick);
lib_status x86_nxvm_board_register_system_control_port(x86_nxvm_board_state *board);
void x86_nxvm_board_after_pit_reset(x86_nxvm_board_state *board);
void x86_nxvm_board_refresh_nmi(void *owner);
void x86_nxvm_board_configure_xt_ppi_speaker(x86_nxvm_board_state *board);
void x86_nxvm_board_set_xt_ppi_speaker(x86_nxvm_board_state *board,
    lib_u8 timer_gate, lib_u8 data_enabled);
lib_status x86_nxvm_board_plan_validate(const x86_nxvm_board_plan *plan);
lib_status x86_nxvm_board_plan_apply_topology(x86_core_machine *machine,
    x86_nxvm_board_state *board,
    const x86_nxvm_board_plan *plan);

#endif
