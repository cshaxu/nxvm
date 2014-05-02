/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */

#include "../utils.h"

#include "vmachine.h"
#include "vport.h"

t_port vport;

/* Empty IO ports */
static void io_read_void() {}
static void io_write_void() {}

/* VPORT has to be initialized before all other devices */
static void init() {
	t_nsbitcc i;
	MEMSET(&vport, Zero8, sizeof(t_port));
	for (i = 0;i < 0x10000;++i) {
		vport.in[i] = (t_faddrcc) io_read_void;
		vport.out[i] = (t_faddrcc) io_write_void;
	}
}

static void reset() {
	vport.iobyte = Zero8;
	vport.ioword = Zero16;
	vport.iodword = Zero32;
}

static void refresh() {}

static void final() {}

void vportRegister() {vmachineAddMe;}
