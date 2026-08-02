/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "core/machine/vadp.h"

static t_vadp *coreMachineVadp;
t_vadp *core_machine_vadp_current(void) { return coreMachineVadp; }
void core_machine_vadp_bind_live(t_vadp *adapter) { coreMachineVadp=adapter; }
void core_machine_vadp_unbind_live(void) { coreMachineVadp=NULL; }

void vvadpInit() {
    core_machine_vadp_initialize(core_machine_vadp_current());
}
void vvadpReset() {
    core_machine_vadp_reset(core_machine_vadp_current());
}
void vvadpRefresh() { core_machine_vadp_refresh(core_machine_vadp_current()); }
void vvadpFinal() { core_machine_vadp_finalize(core_machine_vadp_current()); }

void core_machine_vadp_initialize(t_vadp *adapter)
{
    if (adapter == NULL) return;
    MEMSET((void *)adapter, Zero8, sizeof(*adapter));
}

void core_machine_vadp_reset(t_vadp *adapter)
{
    if (adapter == NULL) return;
    MEMSET((void *)&adapter->data, Zero8, sizeof(adapter->data));
}

void core_machine_vadp_refresh(t_vadp *adapter)
{
    (void)adapter;
}

void core_machine_vadp_finalize(t_vadp *adapter)
{
    (void)adapter;
}
