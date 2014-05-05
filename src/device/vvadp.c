/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "../utils.h"

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
#include "vvadp.h"

t_vadp vvadp;

static void init() {
    MEMSET(&vvadp, Zero8, sizeof(t_vadp));
    vbiosAddInt("qdx 10\niret", 0x10);
}

static void reset() {}

static void refresh() {}

static void final() {}

void vvadpRegister() {
    vmachineAddMe;
}
