#ifndef X86_NXVM_BOARD_XT_PPI_KEYBOARD_H
#define X86_NXVM_BOARD_XT_PPI_KEYBOARD_H
#include "lib/types/types_interface.h"


#include "x86-nxvm/board-xt/xt_ppi_keyboard_interface.h"
#include "x86-core/machine/port_interface.h"
#include "x86-nxvm/chips/ppi8255/ppi8255_interface.h"


/* This owner models the selected IBM XT 8255A Mode-0 board attachment.  It is
 * intentionally not a generic 8255 abstraction: unselected Mode-1/2 board
 * wiring would otherwise become invented guest behavior. */
struct x86_nxvm_board_xt_ppi_keyboard {
    x86_nxvm_board_xt_ppi_keyboard_config config;
    x86_nxvm_board_xt_ppi_irq_line irq;
    void *irq_owner;
    x86_ppi8255 *ppi;
    lib_u8 current_byte;
    lib_bool byte_ready;
    lib_bool irq1_asserted;
    lib_bool io_check_asserted;
    lib_bool ram_parity_asserted;
    lib_bool nmi_signaled;
    x86_nxvm_board_xt_ppi_nmi_request nmi_request;
    void *nmi_owner;
    x86_nxvm_board_xt_ppi_speaker_update speaker_update;
    void *speaker_owner;
    lib_bool (*timer_output)(void *owner);
    void *timer_output_owner;
    x86_nxvm_board_xt_ppi_line_observer line_observer;
    void *line_observer_owner;
    x86_nxvm_board_xt_ppi_byte_released byte_released;
    void *byte_released_owner;
};

lib_i32 x86_nxvm_board_xt_ppi_keyboard_config_is_valid(
    const x86_nxvm_board_xt_ppi_keyboard_config *config);
lib_status x86_nxvm_board_xt_ppi_keyboard_initialize(
    x86_nxvm_board_xt_ppi_keyboard *keyboard,
    const x86_nxvm_board_xt_ppi_keyboard_config *config, x86_core_machine *machine);
void x86_nxvm_board_xt_ppi_keyboard_bind_nmi(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    x86_nxvm_board_xt_ppi_nmi_request request, void *owner);
void x86_nxvm_board_xt_ppi_keyboard_bind_speaker(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    x86_nxvm_board_xt_ppi_speaker_update update, void *owner);
void x86_nxvm_board_xt_ppi_keyboard_bind_keyboard_observer(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, x86_nxvm_board_xt_ppi_line_observer observer,
    void *owner, x86_nxvm_board_xt_ppi_byte_released released);
void x86_nxvm_board_xt_ppi_keyboard_reset(x86_nxvm_board_xt_ppi_keyboard *keyboard);
void x86_nxvm_board_xt_ppi_keyboard_finalize(x86_nxvm_board_xt_ppi_keyboard *keyboard);
lib_status x86_nxvm_board_xt_ppi_keyboard_set_fault_input(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, x86_nxvm_board_xt_ppi_fault_input input,
    lib_i32 asserted);
void x86_nxvm_board_xt_ppi_keyboard_refresh_nmi(x86_nxvm_board_xt_ppi_keyboard *keyboard);
lib_status x86_nxvm_board_xt_ppi_keyboard_receive_device_byte(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, lib_u8 native_byte);

#endif
