/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "vbios.h"
#include "vport.h"
#include "vkbc.h"

void io_read_0064() {
    vport.data.ioByte = VKBC_STATUS_KE;
}

void vkbcInit() {
    vportAddRead(0x0064, (t_faddrcc) io_read_0064);
    vbiosAddInt("qdx 09\niret", 0x09);
    vbiosAddInt("qdx 16\niret", 0x16);
}
void vkbcReset() {}
void vkbcRefresh() {}
void vkbcFinal() {}
