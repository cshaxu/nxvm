/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */

#include "../utils.h"

#include "vport.h"

t_port vport;

void vportInit() {
    MEMSET((void *)(&vport), Zero8, sizeof(t_port));
}

void vportReset() {
    MEMSET((void *)(&vport.data), Zero8, sizeof(t_port_data));
}

void vportRefresh() {}

void vportFinal() {}

void vportAddRead(t_nubit16 portId, t_faddrcc fpIn) {
    vport.connect.fpIn[portId] = fpIn;
}
void vportAddWrite(t_nubit16 portId, t_faddrcc fpOut) {
    vport.connect.fpOut[portId] = fpOut;
}
void vportExecRead(t_nubit16 portId) {
    ExecFun(vport.connect.fpIn[portId]);
}
void vportExecWrite(t_nubit16 portId) {
    ExecFun(vport.connect.fpOut[portId]);
}
