/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
#include "vkbc.h"

void io_read_0064() {vport.ioByte = VKBC_STATUS_KE;}

static void init() {
	vport.in[0x0064] = (t_faddrcc) io_read_0064;
    vbiosAddInt("qdx 09\niret", 0x09);
    vbiosAddInt("qdx 16\niret", 0x16);
}

static void reset() {}

static void refresh() {}

static void final() {}

void vkbcRegister() {vmachineAddMe;}
