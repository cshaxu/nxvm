#ifndef CORE_MACHINE_XT_PPI_KEYBOARD_H
#define CORE_MACHINE_XT_PPI_KEYBOARD_H

#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"

#define CORE_MACHINE_XT_PPI_KEYBOARD_QUEUE_CAPACITY 16u

typedef type_bool (*core_machine_xt_ppi_nmi_request)(C_VOID *owner);
typedef C_VOID (*core_machine_xt_ppi_speaker_update)(C_VOID *owner,
    type_bool timer_gate, type_bool data_enabled);

/* This owner models the selected IBM XT 8255A Mode-0 board attachment.  It is
 * intentionally not a generic 8255 abstraction: unselected Mode-1/2 board
 * wiring would otherwise become invented guest behavior. */
typedef struct core_machine_xt_ppi_keyboard {
    core_machine_xt_ppi_keyboard_config config;
    t_port *port;
    core_machine_pic_irq_source irq1_source;
    type_unsigned_8 mode_control;
    type_unsigned_8 port_a_latch;
    type_unsigned_8 port_b_latch;
    type_unsigned_8 port_c_latch;
    type_unsigned_8 queue[CORE_MACHINE_XT_PPI_KEYBOARD_QUEUE_CAPACITY];
    type_unsigned_8 queue_head;
    type_unsigned_8 queue_count;
    type_unsigned_8 current_byte;
    type_bool byte_ready;
    type_bool irq1_asserted;
    type_bool io_check_asserted;
    type_bool ram_parity_asserted;
    type_bool nmi_signaled;
    core_machine_xt_ppi_nmi_request nmi_request;
    C_VOID *nmi_owner;
    core_machine_xt_ppi_speaker_update speaker_update;
    C_VOID *speaker_owner;
} core_machine_xt_ppi_keyboard;

C_INT core_machine_xt_ppi_keyboard_config_is_valid(
    const core_machine_xt_ppi_keyboard_config *config);
type_status core_machine_xt_ppi_keyboard_initialize(
    core_machine_xt_ppi_keyboard *keyboard,
    const core_machine_xt_ppi_keyboard_config *config, t_port *port);
C_VOID core_machine_xt_ppi_keyboard_bind_pic(core_machine_xt_ppi_keyboard *keyboard,
    t_pic *master, t_pic *slave);
C_VOID core_machine_xt_ppi_keyboard_bind_nmi(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_nmi_request request, C_VOID *owner);
C_VOID core_machine_xt_ppi_keyboard_bind_speaker(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_speaker_update update, C_VOID *owner);
C_VOID core_machine_xt_ppi_keyboard_reset(core_machine_xt_ppi_keyboard *keyboard);
C_VOID core_machine_xt_ppi_keyboard_finalize(core_machine_xt_ppi_keyboard *keyboard);
type_status core_machine_xt_ppi_keyboard_set_fault_input(
    core_machine_xt_ppi_keyboard *keyboard, core_machine_xt_ppi_fault_input input,
    C_INT asserted);
C_VOID core_machine_xt_ppi_keyboard_refresh_nmi(core_machine_xt_ppi_keyboard *keyboard);
type_status core_machine_xt_ppi_keyboard_submit_native_byte(
    core_machine_xt_ppi_keyboard *keyboard, type_unsigned_8 native_byte);
type_status core_machine_xt_ppi_keyboard_submit_native_bytes(
    core_machine_xt_ppi_keyboard *keyboard, const type_unsigned_8 *native_bytes,
    STD_SIZE_T count);

#endif
