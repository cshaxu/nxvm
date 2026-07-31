/* Copyright 2012-2014 Neko. */

#ifndef NXVM_LINUX_H
#define NXVM_LINUX_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "../../global.h"

void linuxSleep(uint32_t milisec);
void linuxDisplaySetScreen(int window);
void linuxDisplayPaint(int window);
void linuxStartMachine(int window);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
