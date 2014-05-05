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
    t_faddrcc in[VPORT_MAX_PORT_COUNT];
    t_faddrcc out[VPORT_MAX_PORT_COUNT];
    union {
        t_nubit8  ioByte;
        t_nubit16 ioWord;
        t_nubit32 ioDWord;
    };
} t_port;

extern t_port vport;

void vportRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
