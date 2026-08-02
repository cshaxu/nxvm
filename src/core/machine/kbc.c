/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "core/machine/port.h"
#include "core/machine/kbc.h"

static t_kbc *coreMachineKbc;
t_kbc *core_machine_kbc_current(void) { return coreMachineKbc; }
void core_machine_kbc_bind_live(t_kbc *controller) { coreMachineKbc=controller; }
void core_machine_kbc_unbind_live(void) { coreMachineKbc=NULL; }

void io_read_0064() {
    vport.data.ioByte = VKBC_STATUS_KE;
}

void vkbcInit() {
    vportAddRead(0x0064, (t_faddrcc) io_read_0064);
}
void vkbcReset() {}
void vkbcRefresh() {}
void vkbcFinal() {}
