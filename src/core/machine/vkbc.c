/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "core/machine/port.h"
#include "core/machine/vkbc.h"

void io_read_0064() {
    vport.data.ioByte = VKBC_STATUS_KE;
}

void vkbcInit() {
    vportAddRead(0x0064, (t_faddrcc) io_read_0064);
}
void vkbcReset() {}
void vkbcRefresh() {}
void vkbcFinal() {}
