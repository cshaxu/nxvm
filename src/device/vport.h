/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VPORT_H
#define NXVM_VPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_PORT "Unknown I/O Port"

#define VPORT_MAX_PORT_COUNT 0x10000

typedef struct {
    t_faddrcc fpIn[VPORT_MAX_PORT_COUNT];
    t_faddrcc fpOut[VPORT_MAX_PORT_COUNT];
} t_port_connect;

typedef struct {
    union {
        t_nubit8  ioByte;
        t_nubit16 ioWord;
        t_nubit32 ioDWord;
    };
} t_port_data;

typedef struct {
    t_port_data data;
    t_port_connect connect;
} t_port;

extern t_port vport;

void vportAddRead(t_nubit16 portId, t_faddrcc fpIn);
void vportAddWrite(t_nubit16 portId, t_faddrcc fpOut);
void vportExecRead(t_nubit16 portId);
void vportExecWrite(t_nubit16 portId);

void vportInit();
void vportReset();
void vportRefresh();
void vportFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
