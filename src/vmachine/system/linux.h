/* This file is a part of NXVM project. */

#ifndef NXVM_LINUX_H
#define NXVM_LINUX_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "../vapi.h"

#if VGLOBAL_PLATFORM == VGLOBAL_VAR_LINUX

#include "unistd.h"

#define linuxSleep(milisec) usleep((milisec) * 1000)
void linuxKeyboardMakeStatus();
void linuxKeyboardMakeKey(int keyvalue);
void linuxDisplaySetScreen();
void linuxDisplayPaint();
void linuxStartMachine();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif

#endif
