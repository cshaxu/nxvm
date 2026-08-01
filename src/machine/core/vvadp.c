/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "nxvm-baseline/utils.h"

#include "nxvm-baseline/device/vbios.h"
#include "core/machine/vport.h"
#include "machine/core/vvadp.h"

t_vadp vvadp;

void vvadpInit() {
    MEMSET((void *)(&vvadp), Zero8, sizeof(t_vadp));
    vbiosAddInt("qdx 10\niret", 0x10);
}
void vvadpReset() {
    MEMSET((void *)(&vvadp.data), Zero8, sizeof(t_vadp_data));
}
void vvadpRefresh() {}
void vvadpFinal() {}
