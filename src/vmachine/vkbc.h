/* This file is a part of NXVM project. */

/* Keyboard Controller: not implemented yet */

#ifndef NXVM_VKBC_H
#define NXVM_VKBC_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

void IO_Read_0064();

void vkbcInit();
void vkbcReset();
void vkbcRefresh();
void vkbcFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
