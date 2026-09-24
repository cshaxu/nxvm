#ifndef CORE_MACHINE_XT_PPI_KEYBOARD_H
#define CORE_MACHINE_XT_PPI_KEYBOARD_H
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/port.h"

#define CORE_MACHINE_XT_PPI_KEYBOARD_QUEUE_CAPACITY 16u

typedef lib_u8 (*core_machine_xt_ppi_nmi_request)(void *owner);
typedef void (*core_machine_xt_ppi_speaker_update)(void *owner,
    lib_u8 timer_gate, lib_u8 data_enabled);
typedef void (*core_machine_xt_ppi_line_observer)(void *owner,
    lib_u8 clock_held, lib_u8 clear_asserted);
typedef void (*core_machine_xt_ppi_byte_released)(void *owner);

/* This owner models the selected IBM XT 8255A Mode-0 board attachment.  It is
 * intentionally not a generic 8255 abstraction: unselected Mode-1/2 board
 * wiring would otherwise become invented guest behavior. */
typedef struct core_machine_xt_ppi_keyboard {
    core_machine_xt_ppi_keyboard_config config;
    t_port *port;
    core_machine_pic_irq_source irq1_source;
    lib_u8 mode_control;
    lib_u8 port_a_latch;
    lib_u8 port_b_latch;
    lib_u8 port_c_latch;
    lib_u8 current_byte;
    lib_u8 byte_ready;
    lib_u8 irq1_asserted;
    lib_u8 io_check_asserted;
    lib_u8 ram_parity_asserted;
    lib_u8 nmi_signaled;
    core_machine_xt_ppi_nmi_request nmi_request;
    void *nmi_owner;
    core_machine_xt_ppi_speaker_update speaker_update;
    void *speaker_owner;
    core_machine_xt_ppi_line_observer line_observer;
    void *line_observer_owner;
    core_machine_xt_ppi_byte_released byte_released;
    void *byte_released_owner;
} core_machine_xt_ppi_keyboard;

lib_i32 core_machine_xt_ppi_keyboard_config_is_valid(
    const core_machine_xt_ppi_keyboard_config *config);
lib_status core_machine_xt_ppi_keyboard_initialize(
    core_machine_xt_ppi_keyboard *keyboard,
    const core_machine_xt_ppi_keyboard_config *config, t_port *port);
void core_machine_xt_ppi_keyboard_bind_pic(core_machine_xt_ppi_keyboard *keyboard,
    t_pic *master, t_pic *slave);
void core_machine_xt_ppi_keyboard_bind_nmi(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_nmi_request request, void *owner);
void core_machine_xt_ppi_keyboard_bind_speaker(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_speaker_update update, void *owner);
void core_machine_xt_ppi_keyboard_bind_keyboard_observer(
    core_machine_xt_ppi_keyboard *keyboard, core_machine_xt_ppi_line_observer observer,
    void *owner, core_machine_xt_ppi_byte_released released);
void core_machine_xt_ppi_keyboard_reset(core_machine_xt_ppi_keyboard *keyboard);
void core_machine_xt_ppi_keyboard_finalize(core_machine_xt_ppi_keyboard *keyboard);
lib_status core_machine_xt_ppi_keyboard_set_fault_input(
    core_machine_xt_ppi_keyboard *keyboard, core_machine_xt_ppi_fault_input input,
    lib_i32 asserted);
void core_machine_xt_ppi_keyboard_refresh_nmi(core_machine_xt_ppi_keyboard *keyboard);
lib_status core_machine_xt_ppi_keyboard_receive_device_byte(
    core_machine_xt_ppi_keyboard *keyboard, lib_u8 native_byte);

#endif
