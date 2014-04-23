/* This file is a part of NXVM project. */

#ifndef NXVM_LINUX_H
#define NXVM_LINUX_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

void linuxSleep(int milisec);
void linuxKeyboardMakeStatus();
void linuxKeyboardMakeKey(int keyvalue);
void linuxKeyboardProcess();
void linuxDisplayInit();
void linuxDisplayFinal();
void linuxDisplaySetScreen();
void linuxDisplayPaint(unsigned char force);
void linuxStartMachine();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
