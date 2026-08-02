/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "core/machine/video_adapter.h"

static t_vadp *coreMachineVideoAdapter;
t_vadp *core_machine_video_adapter_current(void) { return coreMachineVideoAdapter; }
void core_machine_video_adapter_bind_live(t_vadp *adapter) { coreMachineVideoAdapter=adapter; }
void core_machine_video_adapter_unbind_live(void) { coreMachineVideoAdapter=NULL; }

void vvadpInit() {
    MEMSET((void *)(&vvadp), Zero8, sizeof(t_vadp));
}
void vvadpReset() {
    MEMSET((void *)(&vvadp.data), Zero8, sizeof(t_vadp_data));
}
void vvadpRefresh() {}
void vvadpFinal() {}
