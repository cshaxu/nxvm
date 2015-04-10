/* Copyright 2012-2014 Neko. */

#ifndef NKVDM_MSDOS_H
#define NKVDM_MSDOS_H

#ifdef __cplusplus
extern "C" {
#endif

void int20();
void int21();
void int2a();

void msdosInit();
void msdosFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
