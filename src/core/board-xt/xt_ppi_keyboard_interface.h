#ifndef CORE_MACHINE_XT_PPI_KEYBOARD_INTERFACE_H
#define CORE_MACHINE_XT_PPI_KEYBOARD_INTERFACE_H
#include "lib/types/types_interface.h"
#include "core/x86/port_interface.h"

typedef struct core_machine_xt_ppi_keyboard core_machine_xt_ppi_keyboard;
typedef enum core_machine_xt_ppi_fault_input {
    CORE_MACHINE_XT_PPI_FAULT_IO_CHECK = 0,
    CORE_MACHINE_XT_PPI_FAULT_RAM_PARITY = 1
} core_machine_xt_ppi_fault_input;

typedef struct core_machine_xt_ppi_keyboard_config {
    lib_u16 port_a;
    lib_u16 port_b;
    lib_u16 port_c;
    lib_u16 control_port;
    lib_u8 irq;
    /* PB3 selects the low/high system-board DIP electrical nibble. */
    lib_u8 switches_low;
    lib_u8 switches_high;
} core_machine_xt_ppi_keyboard_config;

typedef lib_u8 (*core_machine_xt_ppi_nmi_request)(void *owner);
typedef void (*core_machine_xt_ppi_irq_line)(void *owner, lib_bool asserted);
typedef void (*core_machine_xt_ppi_speaker_update)(void *owner,
    lib_u8 timer_gate, lib_u8 data_enabled);
typedef void (*core_machine_xt_ppi_line_observer)(void *owner,
    lib_u8 clock_held, lib_u8 clear_asserted);
typedef void (*core_machine_xt_ppi_byte_released)(void *owner);

/* Serialized Mode-0 board attachment, not a generic 8255 chip. Creation
 * publishes all four Core port routes atomically. Composition owns the IRQ
 * lease and supplies its line sink. All sink contexts outlive this handle;
 * destroy only after dispatch has stopped, including construction rollback. */
lib_i32 core_machine_xt_ppi_keyboard_config_is_valid(
    const core_machine_xt_ppi_keyboard_config *config);
lib_status core_machine_xt_ppi_keyboard_create(
    const core_machine_xt_ppi_keyboard_config *config, core_machine *machine,
    core_machine_xt_ppi_keyboard **out_keyboard);
void core_machine_xt_ppi_keyboard_destroy(core_machine_xt_ppi_keyboard *keyboard);
void core_machine_xt_ppi_keyboard_bind_irq(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_irq_line irq, void *owner);
void core_machine_xt_ppi_keyboard_bind_nmi(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_nmi_request request, void *owner);
void core_machine_xt_ppi_keyboard_bind_speaker(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_speaker_update update, void *owner);
void core_machine_xt_ppi_keyboard_bind_keyboard_observer(
    core_machine_xt_ppi_keyboard *keyboard, core_machine_xt_ppi_line_observer observer,
    void *owner, core_machine_xt_ppi_byte_released released);
void core_machine_xt_ppi_keyboard_reset(core_machine_xt_ppi_keyboard *keyboard);
lib_status core_machine_xt_ppi_keyboard_set_fault_input(
    core_machine_xt_ppi_keyboard *keyboard, core_machine_xt_ppi_fault_input input,
    lib_i32 asserted);
void core_machine_xt_ppi_keyboard_refresh_nmi(core_machine_xt_ppi_keyboard *keyboard);
lib_status core_machine_xt_ppi_keyboard_receive_device_byte(
    core_machine_xt_ppi_keyboard *keyboard, lib_u8 native_byte);
#endif
