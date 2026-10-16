#ifndef X86_NXVM_BOARD_XT_PPI_KEYBOARD_INTERFACE_H
#define X86_NXVM_BOARD_XT_PPI_KEYBOARD_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86-core/machine/port_interface.h"

typedef struct x86_nxvm_board_xt_ppi_keyboard x86_nxvm_board_xt_ppi_keyboard;
typedef enum x86_nxvm_board_xt_ppi_fault_input {
    X86_NXVM_BOARD_XT_PPI_FAULT_IO_CHECK = 0,
    X86_NXVM_BOARD_XT_PPI_FAULT_RAM_PARITY = 1
} x86_nxvm_board_xt_ppi_fault_input;

typedef struct x86_nxvm_board_xt_ppi_keyboard_config {
    lib_u16 port_a;
    lib_u16 port_b;
    lib_u16 port_c;
    lib_u16 control_port;
    lib_u8 irq;
    /* PB3 selects the low/high system-board DIP electrical nibble. */
    lib_u8 switches_low;
    lib_u8 switches_high;
} x86_nxvm_board_xt_ppi_keyboard_config;

typedef lib_u8 (*x86_nxvm_board_xt_ppi_nmi_request)(void *owner);
typedef void (*x86_nxvm_board_xt_ppi_irq_line)(void *owner, lib_bool asserted);
typedef void (*x86_nxvm_board_xt_ppi_speaker_update)(void *owner,
    lib_u8 timer_gate, lib_u8 data_enabled);
typedef void (*x86_nxvm_board_xt_ppi_line_observer)(void *owner,
    lib_u8 clock_held, lib_u8 clear_asserted);
typedef void (*x86_nxvm_board_xt_ppi_byte_released)(void *owner);

/* Serialized Mode-0 board attachment, not a generic 8255 chip. Creation
 * publishes all four Core port routes atomically. Composition owns the IRQ
 * lease and supplies its line sink. All sink contexts outlive this handle;
 * destroy only after dispatch has stopped, including construction rollback. */
lib_i32 x86_nxvm_board_xt_ppi_keyboard_config_is_valid(
    const x86_nxvm_board_xt_ppi_keyboard_config *config);
lib_status x86_nxvm_board_xt_ppi_keyboard_create(
    const x86_nxvm_board_xt_ppi_keyboard_config *config, x86_core_machine *machine,
    x86_nxvm_board_xt_ppi_keyboard **out_keyboard);
void x86_nxvm_board_xt_ppi_keyboard_destroy(x86_nxvm_board_xt_ppi_keyboard *keyboard);
void x86_nxvm_board_xt_ppi_keyboard_bind_irq(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    x86_nxvm_board_xt_ppi_irq_line irq, void *owner);
void x86_nxvm_board_xt_ppi_keyboard_bind_nmi(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    x86_nxvm_board_xt_ppi_nmi_request request, void *owner);
void x86_nxvm_board_xt_ppi_keyboard_bind_speaker(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    x86_nxvm_board_xt_ppi_speaker_update update, void *owner);
/* Borrowed PIT channel-2 OUT input for PC5; the context outlives this handle.
 * Sampled during serialized port reads, never a second timer state. */
void x86_nxvm_board_xt_ppi_keyboard_bind_timer_output(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    lib_bool (*output)(void *owner), void *owner);
void x86_nxvm_board_xt_ppi_keyboard_bind_keyboard_observer(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, x86_nxvm_board_xt_ppi_line_observer observer,
    void *owner, x86_nxvm_board_xt_ppi_byte_released released);
void x86_nxvm_board_xt_ppi_keyboard_reset(x86_nxvm_board_xt_ppi_keyboard *keyboard);
lib_status x86_nxvm_board_xt_ppi_keyboard_set_fault_input(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, x86_nxvm_board_xt_ppi_fault_input input,
    lib_i32 asserted);
void x86_nxvm_board_xt_ppi_keyboard_refresh_nmi(x86_nxvm_board_xt_ppi_keyboard *keyboard);
lib_status x86_nxvm_board_xt_ppi_keyboard_receive_device_byte(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, lib_u8 native_byte);
#endif
