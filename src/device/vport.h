/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VPORT_H
#define NXVM_VPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_PORT "Unknown I/O Port"

typedef struct {
	t_faddrcc in[0x10000];
	t_faddrcc out[0x10000];
	union {
		t_nubit8  iobyte;
		t_nubit16 ioword;
		t_nubit32 iodword;
	};
} t_port;

extern t_port vport;

void vportRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
