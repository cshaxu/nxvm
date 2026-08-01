/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "core/product/utils.h"

#include "core/machine/vport.h"
#include "vm/machine/vvadp.h"

t_vadp vvadp;

void vvadpInit() {
    MEMSET((void *)(&vvadp), Zero8, sizeof(t_vadp));
}
void vvadpReset() {
    MEMSET((void *)(&vvadp.data), Zero8, sizeof(t_vadp_data));
}
void vvadpRefresh() {}
void vvadpFinal() {}
