#ifndef CORE_MACHINE_BOARD_STATE_H
#define CORE_MACHINE_BOARD_STATE_H

#include "app-nxvm/devices/machine.h"

/* One board-owned lifetime. Fields move here by their actual owner; Core
 * retains only the opaque attachment pointer and bounded callbacks. */
struct core_machine_board_state {
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
    t_vadp shared_vadp;
    core_machine_clock_domain dma_clock;
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

#endif
