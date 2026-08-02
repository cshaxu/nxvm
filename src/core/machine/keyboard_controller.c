/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "core/machine/port.h"
#include "core/machine/keyboard_controller.h"

static t_kbc *coreMachineKeyboardController;
t_kbc *core_machine_keyboard_controller_current(void) { return coreMachineKeyboardController; }
void core_machine_keyboard_controller_bind_live(t_kbc *controller) { coreMachineKeyboardController=controller; }
void core_machine_keyboard_controller_unbind_live(void) { coreMachineKeyboardController=NULL; }

void io_read_0064() {
    vport.data.ioByte = VKBC_STATUS_KE;
}

void vkbcInit() {
    vportAddRead(0x0064, (t_faddrcc) io_read_0064);
}
void vkbcReset() {}
void vkbcRefresh() {}
void vkbcFinal() {}
