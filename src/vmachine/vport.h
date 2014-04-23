/* This file is a part of NXVM project. */

/* In/Out Ports */

#ifndef NXVM_VPORT_H
#define NXVM_VPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

typedef struct {
	t_faddrcc in[0x10000];
	t_faddrcc out[0x10000];
} t_port;

extern t_port vport;

void IO_Read_VOID();
void IO_Write_VOID();

void vportInit();
void vportRefresh();
void vportFinal();

#ifdef __cplusplus
}
#endif

#endif