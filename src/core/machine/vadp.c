/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "core/machine/vadp.h"

static t_vadp *coreMachineVadp;
t_vadp *core_machine_vadp_current(void) { return coreMachineVadp; }
void core_machine_vadp_bind_live(t_vadp *adapter) { coreMachineVadp=adapter; }
void core_machine_vadp_unbind_live(void) { coreMachineVadp=NULL; }

void vvadpInit() {
    MEMSET((void *)(&vvadp), Zero8, sizeof(t_vadp));
}
void vvadpReset() {
    MEMSET((void *)(&vvadp.data), Zero8, sizeof(t_vadp_data));
}
void vvadpRefresh() {}
void vvadpFinal() {}
