/* This file is a part of NXVM project. */

#ifndef NXVM_QDBIOS_H
#define NXVM_QDBIOS_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

void INT_08();
void INT_09();
void INT_0E();
void INT_10();
void INT_11();
void INT_12();
void INT_13();
void INT_14();
void INT_15();
void INT_16();
void INT_17();
void INT_1A();

void qdbiosExecInt(t_nubit8 intid);
void qdbiosRefresh();
void qdbiosInit();
void qdbiosReset(); /* POST */
void qdbiosFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
