/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VKBC_H
#define NXVM_VKBC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_KBC "Intel 8042"

#define VKBC_STATUS_KE 0x10 /* keyboard enabled(1) or not(0) */

void vkbcInit();
void vkbcReset();
void vkbcRefresh();
void vkbcFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
