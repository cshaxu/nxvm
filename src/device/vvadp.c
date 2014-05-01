/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "../utils.h"

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
#include "vvadp.h"

t_vadp vvadp;

static void init() {
	MEMSET(&vvadp, 0x00, sizeof(t_vadp));
	vvadp.bufcomp = (t_vaddrcc) MALLOC(0x00040000);
    vbiosAddInt("qdx 10\niret", 0x10);
}

static void reset() {}

static void refresh() {}

static void final() {
	if (vvadp.bufcomp) {
		FREE((void *) vvadp.bufcomp);
	}
	vvadp.bufcomp = (t_vaddrcc) NULL;
}

void vvadpRegister() {vmachineAddMe;}
