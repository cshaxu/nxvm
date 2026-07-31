/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */

#include "../utils.h"

#include "vport.h"

t_port vport;

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

void vportInit() {
    MEMSET((void *)(&vport), Zero8, sizeof(t_port));
}
void vportReset() {
    MEMSET((void *)(&vport.data), Zero8, sizeof(t_port_data));
}
void vportRefresh() {}
void vportFinal() {}

void deviceConnectPortRead(uint16_t portId) {
    vportExecRead(portId);
}
void deviceConnectPortWrite(uint16_t portId) {
    vportExecWrite(portId);
}
uint32_t deviceConnectPortGetValue() {
    return vport.data.ioDWord;
}
void deviceConnectPortSetValue(uint32_t value) {
    vport.data.ioDWord = value;
}
