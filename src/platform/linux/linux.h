/* Copyright 2012-2014 Neko. */

#ifndef NXVM_LINUX_H
#define NXVM_LINUX_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

void linuxSleep(unsigned int milisec);
void linuxDisplaySetScreen(unsigned char window);
void linuxDisplayPaint(unsigned char window);
void linuxStartMachine(unsigned char window);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
