#ifndef CORE_MACHINE_XT_PPI_KEYBOARD_H
#define CORE_MACHINE_XT_PPI_KEYBOARD_H
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/pic_bus.h"
#include "app-nxvm/devices/port_interface.h"
#include "x86/chips/ppi8255/ppi8255_interface.h"

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
    core_machine_pic_irq_source irq1_source;
    x86_ppi8255 *ppi;
    lib_u8 current_byte;
    lib_bool byte_ready;
    lib_bool irq1_asserted;
    lib_bool io_check_asserted;
    lib_bool ram_parity_asserted;
    lib_bool nmi_signaled;
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
    const core_machine_xt_ppi_keyboard_config *config, core_machine *machine);
void core_machine_xt_ppi_keyboard_bind_pic(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_pic_bus *master, core_machine_pic_bus *slave);
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
